/*
 * @file:
 * @Author: ligengchao

 * @Date: 2023-06-19 20:13:51
 * @edit: ligengchao
 * @brief:
 */
#include "rpc_mgr.h"
#include "conn_mgr.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
using namespace llbc;

RpcMgr::RpcMgr(ConnMgr *connMgr) : connMgr_(connMgr) {
  connMgr_->Subscribe(
      RpcOpCode::RpcReq,
      LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcMgr::HandleRpcReq));
  connMgr_->Subscribe(
      RpcOpCode::RpcRsp,
      LLBC_Delegate<void(LLBC_Packet &)>(this, &RpcMgr::HandleRpcRsp));
}

RpcMgr::~RpcMgr() {
  connMgr_->Unsubscribe(RpcOpCode::RpcReq);
  connMgr_->Unsubscribe(RpcOpCode::RpcRsp);
}

void RpcMgr::AddService(::google::protobuf::Service *service) {
  ServiceInfo service_info;
  service_info.service = service;
  service_info.sd = service->GetDescriptor();
  for (int i = 0; i < service_info.sd->method_count(); ++i) {
    service_info.mds[service_info.sd->method(i)->name()] =
        service_info.sd->method(i);
  }

  _services[service_info.sd->name()] = service_info;
}

void RpcMgr::HandleRpcReq(LLBC_Packet &packet) {
  // 读取serviceName&methodName
  int srcCoroId;
  std::string serviceName, methodName;
  if (packet.Read(serviceName) != LLBC_OK ||
      packet.Read(methodName) != LLBC_OK || packet.Read(srcCoroId) != LLBC_OK) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "read packet failed");
    return;
  }

  auto service = _services[serviceName].service;
  auto md = _services[serviceName].mds[methodName];
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv service_name:%s",
       serviceName.c_str());
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "recv method_name:%s",
       methodName.c_str());
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "srcCoroId:%d", srcCoroId);

  // 解析req&创建rsp
  auto req = service->GetRequestPrototype(md).New();
  if (packet.Read(*req) != LLBC_OK) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "read packet failed");
    delete req;
    return;
  }
  auto rsp = service->GetResponsePrototype(md).New();

  auto controller = new RpcController();
  controller->SetParam1(packet.GetSessionId());
  controller->SetParam2(srcCoroId);

  // 创建rpc完成回调函数
  auto done = ::google::protobuf::NewCallback(
      this, &RpcMgr::OnRpcDone, controller, rsp);

  service->CallMethod(md, controller, req, rsp, done);
}

void RpcMgr::HandleRpcRsp(LLBC_Packet &packet) {
  // 读取源协程Id
  int dstCoroId = 0;
  if (packet.Read(dstCoroId)  != LLBC_OK)
  {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "read packet failed");
    return;
  }

  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "resume coro:%d, packet:%s",
       dstCoroId, packet.ToString().c_str());

  // 获取协程信息
  auto coroInfo = s_RpcCoroMgr->GetRpcCoroInfo(int(dstCoroId));
  if (!coroInfo) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "coro not found, coroId:%d",
         dstCoroId);
    return;
  }

  // 读取错误描述
  std::string errText;
  if (packet.Read(errText) != LLBC_OK)
  {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "read packet failed");
    coroInfo->GetController()->SetFailed("read packet  failed");
  }

  // 解码packet并填充rsp
  if (coroInfo->DecodeRsp(&packet)  != LLBC_OK)
  {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Error, "decode rsp failed, coroId:%d",
         dstCoroId);
    coroInfo->GetController()->SetFailed("decode rsp failed");
  }
  
  // 填充错误状态
  if (!errText.empty()) {
    coroInfo->GetController()->SetFailed(errText);
  }

  // 恢复对应协程处理
  s_RpcCoroMgr->ResumeRpcCoro(int(dstCoroId));
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "resume coro:%d finished, packet:%s, errText:%s",
       dstCoroId, packet.ToString().c_str(), errText.c_str());
}

void RpcMgr::OnRpcDone(RpcController *controller, google::protobuf::Message *rsp) {
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "OnRpcDone, rsp:%s",
       rsp->DebugString().c_str());

  auto sessionId = controller->GetParam1();
  auto srcCoroId = controller->GetParam2();
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "coroSessionId:%d, srcCoroId:%d, errText:%s",
       sessionId, srcCoroId, controller->ErrorText().c_str());
  auto packet = LLBC_GetObjectFromUnsafetyPool<LLBC_Packet>();
  packet->SetSessionId(sessionId);
  packet->SetOpcode(RpcOpCode::RpcRsp);
  packet->Write(srcCoroId);
  packet->Write(controller->ErrorText());
  packet->Write(*rsp);
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "rsp packet:%s",
       packet->ToString().c_str());
  // 回包
  connMgr_->PushPacket(packet);
  delete controller;
}
