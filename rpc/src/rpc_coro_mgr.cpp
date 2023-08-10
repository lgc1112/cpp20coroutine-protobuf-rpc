/*
 * @file:
 * @Author: ligengchao

 * @Date: 2023-06-20 20:13:31
 * @edit: ligengchao
 * @brief:
 */

#include "rpc_coro_mgr.h"
#include "rpc_channel.h"

using namespace llbc;

void *RpcCoroInfo::GetHandle() const { return controller_->GetPtrParam(); }

int RpcCoroInfo::DecodeRsp(llbc::LLBC_Packet *packet) {
  if (!packet || packet->Read(*rsp_) != LLBC_OK) {
    LOG_ERROR("Read recvPacket fail");
    return LLBC_FAILED;
  }
  return LLBC_OK;
}

void RpcCoroInfo::Resume() { coroMgr_->ResumeRpcCoro(id_); }

void RpcCoroInfo::OnCoroTimeout() {
  LOG_WARN("Coro %d timeout, resume", id_);
  controller_->SetFailed("timeout");
  Resume();
}

void RpcCoroInfo::OnCoroCancel() {
  LOG_WARN("Coro %d cancelled, resume", id_);
  controller_->SetFailed("cancelled");
  Resume();
}

RpcCoroMgr::~RpcCoroMgr() {
  for (auto it = coroInfos_.begin(); it != coroInfos_.end(); ++it) {
    LOG_WARN("Coro %d not resume, resume failed", it->first);
    it->second->OnCoroCancel();
  }
}

int RpcCoroMgr::AddRpcCoroInfo(RpcController *controller,
                               google::protobuf::Message *rsp) {
  int id = maxCoroId++;
  // auto coroInfo = LLBC_GetObjectFromSafetyPool<RpcCoroInfo>();
  // coroInfo->SetCoroId(id);
  // coroInfo->SetRpcCoroMgr(this);
  // coroInfo->SetController(controller);
  // coroInfo->SetRsp(rsp);
  // coroInfo->SetTimeoutTime(llbc::LLBC_GetMilliSeconds() + CoroTimeoutTime);

  auto coroInfo = new RpcCoroInfo(id, this, controller, rsp);
  coroInfos_[id] = coroInfo;
  coroTimeHeap_.Insert(coroInfo);
  return id;
}

RpcCoroInfo *RpcCoroMgr::GetRpcCoroInfo(int coroId) {
  auto it = coroInfos_.find(coroId);
  if (it == coroInfos_.end()) {
    LOG_ERROR("Coro %d not found, resume failed", coroId);
    return nullptr;
  }
  return it->second;
}

void RpcCoroMgr::ResumeRpcCoro(int coroId) {
  auto it = coroInfos_.find(coroId);
  if (it == coroInfos_.end()) {
    LOG_ERROR("Coro %d not found, resume failed", coroId);
    return;
  }
  auto coroInfo = it->second;
  LOG_TRACE("parentCoro resume");
  std::coroutine_handle<RpcCoro::promise_type>::from_address(
      coroInfo->GetHandle())
      .resume();
  LOG_TRACE("parentCoro resumed");
  coroTimeHeap_.DeleteElem(coroInfo);
  // LLBC_Recycle(coroInfo);
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
  }
}
