/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:31
 * @edit: regangcli
 * @brief:
 */

#include "rpc_coro_mgr.h"
#include "rpc_channel.h"

using namespace llbc;

void *CoroInfo::GetAddr() const { return controller_->GetPtrParam(); }

int CoroInfo::DecodeRsp(llbc::LLBC_Packet *packet) {
  if (!packet || packet->Read(*rsp_) != LLBC_OK) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "Read recvPacket fail");
    return LLBC_FAILED;
  }
  return LLBC_OK;
}

void CoroInfo::Resume() { coroMgr_->ResumeRpcCoro(id_); }

void CoroInfo::OnCoroTimeout() {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Warn, "Coro %d timeout, resume", id_);
  controller_->SetFailed("timeout");
  Resume();
}

void CoroInfo::OnCoroCancel() {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Warn, "Coro %d cancelled, resume", id_);
  controller_->SetFailed("cancelled");
  Resume();
}

RpcCoroMgr::~RpcCoroMgr() {
  for (auto it = coroInfos_.begin(); it != coroInfos_.end(); ++it) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Warn,
         "Coro %d not resume, resume failed", it->first);
    it->second->OnCoroCancel();
  }
}

int RpcCoroMgr::AddRpcCoro(MyController *controller,
                           google::protobuf::Message *rsp) {
  int id = maxCoroId++;
  auto coroInfo = new CoroInfo(id, this, controller, rsp);
  coroInfos_[id] = coroInfo;
  coroTimeHeap_.Insert(coroInfo);
  return id;
}

CoroInfo *RpcCoroMgr::GetRpcCoroInfo(int coroId) {
  auto it = coroInfos_.find(coroId);
  if (it == coroInfos_.end()) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error,
         "Coro %d not found, resume failed", coroId);
    return nullptr;
  }
  return it->second;
}

void RpcCoroMgr::ResumeRpcCoro(int coroId) {
  auto it = coroInfos_.find(coroId);
  if (it == coroInfos_.end()) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error,
         "Coro %d not found, resume failed", coroId);
    return;
  }
  auto coroInfo = it->second;
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "parentCoro resume");
  std::coroutine_handle<RpcCoro::promise_type>::from_address(
      coroInfo->GetAddr())
      .resume();
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "parentCoro resumed");

  coroTimeHeap_.DeleteElem(coroInfo);
  delete coroInfo;
  coroInfos_.erase(it);

  return;
}

void RpcCoroMgr::Update() {
  auto now = LLBC_GetMilliSeconds();
  // 有超时的协程，调用超时处理函数
  while (!coroTimeHeap_.IsEmpty() &&
         coroTimeHeap_.Top()->GetTimeoutTime() <= now) {
    auto coroInfo = coroTimeHeap_.Top();
    coroInfo->OnCoroTimeout();
    // coroTimeHeap_.DeleteTop();
  }
}
