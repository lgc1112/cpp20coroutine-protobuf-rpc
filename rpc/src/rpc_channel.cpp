/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-15 20:21:17
 * @edit: regangcli
 * @brief:
 */
#include "rpc_channel.h"
#include "conn_mgr.h"
#include "llbc.h"
#include "rpc_coro_mgr.h"

using namespace llbc;

RpcChannel::~RpcChannel() { }

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller,
                            const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *) {

  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CallMethod!");
  LLBC_Packet *sendPacket = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
  sendPacket->SetHeader(0, RpcOpCode::RpcReq, 0);
  sendPacket->SetSessionId(sessionId_);
  sendPacket->Write(method->service()->name());
  sendPacket->Write(method->name());

  // 填充原始协程id
  auto mController = static_cast<MyController *>(controller);
  auto coroId = g_rpcCoroMgr->AddRpcCoro(mController, response);
  sendPacket->Write(coroId);
  sendPacket->Write(*request);

  connMgr_->PushPacket(sendPacket);
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Waiting!");
}