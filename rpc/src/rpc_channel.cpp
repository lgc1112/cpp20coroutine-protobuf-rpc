/*
 * @file:
 * @Author: ligengchao

 * @Date: 2023-06-15 20:21:17
 * @edit: ligengchao
 * @brief:
 */
#include "rpc_channel.h"
#include "conn_mgr.h"
#include "llbc.h"
#include "rpc_coro_mgr.h"

using namespace llbc;

RpcChannel::~RpcChannel() {}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller,
                            const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *) {
  response->Clear();

  LOG_TRACE("CallMethod!");
  // 创建并填充发送包
  LLBC_Packet *sendPacket = LLBC_GetObjectFromSafetyPool<LLBC_Packet>();
  sendPacket->SetHeader(0, RpcOpCode::RpcReq, 0);
  sendPacket->SetSessionId(sessionId_);
  sendPacket->Write(method->service()->name());
  sendPacket->Write(method->name());

  // Controller信息和rsp并创建协程信息
  auto coroId = s_RpcCoroMgr->AddRpcCoroInfo(
      static_cast<RpcController *>(controller), response);

  // 填充协程Id和请求包
  sendPacket->Write(coroId);
  sendPacket->Write(*request);

  // 通过连接管理器发送包
  auto ret = connMgr_->PushPacket(sendPacket);
  if (ret != LLBC_OK) {
    LOG_ERROR("PushPacket failed, ret: %s", LLBC_FormatLastError());
    return;
  }
  LOG_TRACE("Waiting!");
}