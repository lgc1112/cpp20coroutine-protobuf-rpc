/*
 * @file:
 * @Author: ligengchao
 * @Date: 2023-06-19 20:34:15
 * @edit: ligengchao
 * @brief:
 */
#include "conn_mgr.h"
#include "rpc_channel.h"

ConnComp::ConnComp()
    : LLBC_Component(LLBC_ComponentEvents::DefaultEvents |
                     LLBC_ComponentEvents::OnUpdate) {}

bool ConnComp::OnInit(bool &initFinished) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Service create!");
  return true;
}

void ConnComp::OnDestroy(bool &destroyFinished) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Service destroy!");
}

void ConnComp::OnSessionCreate(const LLBC_SessionInfo &sessionInfo) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Session Create: %s",
       sessionInfo.ToString().c_str());
}

void ConnComp::OnSessionDestroy(const LLBC_SessionDestroyInfo &destroyInfo) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Session Destroy, info: %s",
       destroyInfo.ToString().c_str());
  // Todo：此处为网络中断时，通信线程调用，需要将消息放入recvQueue_，由主线程处理
  s_ConnMgr->CloseSession(destroyInfo.GetSessionId());
}

void ConnComp::OnAsyncConnResult(const LLBC_AsyncConnResult &result) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Async-Conn result: %s",
       result.ToString().c_str());
}

void ConnComp::OnUnHandledPacket(const LLBC_Packet &packet) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace,
       "Unhandled packet, sessionId: %d, opcode: %d, payloadLen: %ld",
       packet.GetSessionId(), packet.GetOpcode(), packet.GetPayloadLength());
}

void ConnComp::OnProtoReport(const LLBC_ProtoReport &report) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Proto report: %s",
       report.ToString().c_str());
}

void ConnComp::OnUpdate() {
  auto *sendPacket = sendQueue_.Pop();
  while (sendPacket) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "sendPacket:%s",
         sendPacket->ToString().c_str());
    auto ret = GetService()->Send(sendPacket);
    if (ret != LLBC_OK) {
      LLOG(nullptr, nullptr, LLBC_LogLevel::Error,
           "Send packet failed, err: %s", LLBC_FormatLastError());
    }

    // LLBC_Recycle(sendPacket);
    sendPacket = sendQueue_.Pop();
  }
}

void ConnComp::OnRecvPacket(LLBC_Packet &packet) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "OnRecvPacket:%s",
       packet.ToString().c_str());
  LLBC_Packet *recvPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
  recvPacket->SetHeader(packet, packet.GetOpcode(), 0);
  recvPacket->SetPayload(packet.DetachPayload());
  recvQueue_.Push(recvPacket);
}

int ConnComp::PushPacket(LLBC_Packet *sendPacket) {
  if (sendQueue_.Push(sendPacket))
    return LLBC_OK;

  return LLBC_FAILED;
}

ConnMgr::ConnMgr() {
  // Init();
}

ConnMgr::~ConnMgr() {
  for (auto &pair : session2Addr_) {
    CloseSession(pair.first);
  }
}

int ConnMgr::Init() {
  // Create service
  svc_ = LLBC_Service::Create("SvcTest");
  comp_ = new ConnComp;
  svc_->AddComponent(comp_);
  svc_->Subscribe(RpcOpCode::RpcReq, comp_, &ConnComp::OnRecvPacket);
  svc_->Subscribe(RpcOpCode::RpcRsp, comp_, &ConnComp::OnRecvPacket);
  svc_->SuppressCoderNotFoundWarning();
  auto ret = svc_->Start(1);
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Service start, ret: %d", ret);
  return ret;
}

int ConnMgr::StartRpcService(const char *ip, int port) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "ConnMgr StartRpcService");
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Server Will listening in %s:%d",
       ip, port);
  int serverSessionId_ = svc_->Listen(ip, port);
  if (serverSessionId_ == 0) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error,
         "Create session failed, reason: %s", LLBC_FormatLastError());
    return LLBC_FAILED;
  }

  isServer_ = true;
  return LLBC_OK;
}

// 创建rpc客户端通信通道
RpcChannel *ConnMgr::GetRpcChannel(const char *ip, int port) {
  std::string addr = std::string(ip) + ":" + std::to_string(port);
  auto it = addr2Channel_.find(addr);
  if (it != addr2Channel_.end()) {
    return it->second;
  }

  auto sessionId = svc_->Connect(ip, port);
  if (sessionId == 0) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error,
         "Create session failed, reason: %s", LLBC_FormatLastError());
    return nullptr;
  }

  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace,
       "GetRpcChannel, sessionId:%d, addr:%s", sessionId, addr.c_str());
  session2Addr_.emplace(sessionId, addr);
  return addr2Channel_.emplace(addr, new RpcChannel(this, sessionId))
      .first->second;
}

int ConnMgr::CloseSession(int sessionId) {
  auto it = session2Addr_.find(sessionId);
  if (it == session2Addr_.end()) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error,
         "CloseSession, sessionId:%d not found", sessionId);
    return LLBC_FAILED;
  }

  addr2Channel_.erase(it->second);
  session2Addr_.erase(it);

  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CloseSession, %d", sessionId);
  return svc_->RemoveSession(sessionId);
}

bool ConnMgr::Tick() {
  auto ret = false;
  // 读取接收到的数据包并给对应的订阅者处理
  auto packet = PopPacket();
  while (packet) {
    ret = true;
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Tick");
    auto it = packetDelegs_.find(packet->GetOpcode());
    if (it == packetDelegs_.end())
      LLOG(nullptr, nullptr, LLBC_LogLevel::Warn, "Recv Untapped opcode:%d",
           packet->GetOpcode());
    else
      (it->second)(*packet);

    // 取下一个包
    LLBC_Recycle(packet);
    packet = PopPacket();
  }

  return ret;
}

int ConnMgr::Subscribe(int cmdId,
                       const LLBC_Delegate<void(LLBC_Packet &)> &deleg) {
  auto pair = packetDelegs_.emplace(cmdId, deleg);
  if (!pair.second) {
    return LLBC_FAILED;
  }

  return LLBC_OK;
}

void ConnMgr::Unsubscribe(int cmdId) { packetDelegs_.erase(cmdId); }
