/*
 * @file:
 * @Author: ligengchao
 * @Date: 2023-06-19 20:34:15
 * @edit: ligengchao
 * @brief:
 */
#include "conn_mgr.h"
#include "rpc_channel.h"

ConnComp::ConnComp() : LLBC_Component(LLBC_ComponentEvents::DefaultEvents) {}

bool ConnComp::OnInit(bool &initFinished) {
  LOG_TRACE("Service create!");
  return true;
}

void ConnComp::OnDestroy(bool &destroyFinished) {
  LOG_TRACE("Service destroy!");
}

void ConnComp::OnSessionCreate(const LLBC_SessionInfo &sessionInfo) {
  LOG_TRACE("Session Create: %s", sessionInfo.ToString().c_str());
}

void ConnComp::OnSessionDestroy(const LLBC_SessionDestroyInfo &destroyInfo) {
  LOG_TRACE("Session Destroy, info: %s", destroyInfo.ToString().c_str());
  s_ConnMgr->CloseSession(destroyInfo.GetSessionId());
}

void ConnComp::OnAsyncConnResult(const LLBC_AsyncConnResult &result) {
  LOG_TRACE("Async-Conn result: %s", result.ToString().c_str());
}

void ConnComp::OnUnHandledPacket(const LLBC_Packet &packet) {
  LOG_TRACE("Unhandled packet, sessionId: %d, opcode: %d, payloadLen: %ld",
            packet.GetSessionId(), packet.GetOpcode(),
            packet.GetPayloadLength());
}

void ConnComp::OnProtoReport(const LLBC_ProtoReport &report) {
  LOG_TRACE("Proto report: %s", report.ToString().c_str());
}

void ConnComp::OnUpdate() {}

int ConnComp::PushPacket(LLBC_Packet *sendPacket) {
  return GetService()->Send(sendPacket);
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
  svc_->SetDriveMode(LLBC_Service::ExternalDrive);
  svc_->SetFPS(500);
  comp_ = new ConnComp;
  svc_->AddComponent(comp_);
  svc_->SuppressCoderNotFoundWarning();
  return LLBC_OK;
}

int ConnMgr::Start() {
  auto ret = svc_->Start(5);
  LOG_TRACE("Service start, ret: %d", ret);
  return ret;
}

int ConnMgr::StartRpcService(const char *ip, int port) {
  LOG_TRACE("ConnMgr StartRpcService");
  LOG_TRACE("Server Will listening in %s:%d", ip, port);
  int serverSessionId_ = svc_->Listen(ip, port);
  if (serverSessionId_ == 0) {
    LOG_ERROR("Create session failed, reason: %s", LLBC_FormatLastError());
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
    LOG_ERROR("Create session failed, reason: %s", LLBC_FormatLastError());
    return nullptr;
  }

  LOG_TRACE("GetRpcChannel, sessionId:%d, addr:%s", sessionId, addr.c_str());
  session2Addr_.emplace(sessionId, addr);
  return addr2Channel_.emplace(addr, new RpcChannel(this, sessionId))
      .first->second;
}

int ConnMgr::CloseSession(int sessionId) {
  auto it = session2Addr_.find(sessionId);
  if (it == session2Addr_.end()) {
    LOG_ERROR("CloseSession, sessionId:%d not found", sessionId);
    return LLBC_FAILED;
  }

  addr2Channel_.erase(it->second);
  session2Addr_.erase(it);

  LOG_TRACE("CloseSession, %d", sessionId);
  return svc_->RemoveSession(sessionId);
}

bool ConnMgr::Tick() {
  
#ifdef EnableRpcPerfStat
  svc_->OnSvc(false);
#else
  svc_->OnSvc(true);
#endif

  return true;
}

int ConnMgr::Subscribe(int cmdId,
                       const LLBC_Delegate<void(LLBC_Packet &)> &deleg) {
  if (svc_->Subscribe(cmdId, deleg) != LLBC_OK) {
    LOG_ERROR("Subscribe failed, cmdId:%d, errstr:%s", cmdId,
              LLBC_FormatLastError());
    return LLBC_FAILED;
  }

  return LLBC_OK;
}

void ConnMgr::Unsubscribe(int cmdId) {}
