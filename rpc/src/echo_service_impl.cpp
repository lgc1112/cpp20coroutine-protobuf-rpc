/*
 * @file:
 * @Author: ligengchao

 * @Date: 2023-06-21 15:47:04
 * @edit: ligengchao
 * @brief:
 */
#include "echo_service_impl.h"
#include "conn_mgr.h"
#include "echo_stub_impl.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
using namespace llbc;

RpcCoro TestCallMeathod(::google::protobuf::Closure *done) {
  done->Run();
  co_return;
}
void MyEchoService::Echo(::google::protobuf::RpcController *controller,
                         const ::echo::EchoRequest *request,
                         ::echo::EchoResponse *response,
                         ::google::protobuf::Closure *done) {

#ifndef EnableRpcPerfStat
  LOG_INFO("received, msg:%s", request->msg().c_str());
  // LLBC_Sleep(5000); timeout test
  response->set_msg(std::string(" Echo >>>>>>> ") + request->msg());
  done->Run();
#else
  // TestCallMeathod(done);
  done->Run();
#endif
}

RpcCoro InnerCallMeathod(::google::protobuf::RpcController *controller,
                         const ::echo::EchoRequest *req,
                         ::echo::EchoResponse *rsp,
                         ::google::protobuf::Closure *done) {
  // 初始化内部 rpc req & rsp
  echo::EchoRequest innerReq;
  innerReq.set_msg("Relay Call >>>>>>" + req->msg());
  echo::EchoResponse innerRsp;
  // 创建 rpc channel
  RpcChannel *channel = s_ConnMgr->GetRpcChannel("127.0.0.1", 6688);
  if (!channel) {
    LOG_INFO("GetRpcChannel Fail");
    rsp->set_msg(req->msg() + " ---- inner rpc call server not exist");
    controller->SetFailed("GetRpcChannel Fail");
    done->Run();
    co_return;
  }

  LOG_INFO("call, msg:%s", innerReq.msg().c_str());

  RpcController cntl(co_await GetHandleAwaiter{});

  EchoService_MyStub stub(channel);
  co_await stub.Echo(&cntl, &innerReq, &innerRsp, nullptr);
  LOG_INFO("Recv rsp, status:%s, rsp:%s",
           cntl.Failed() ? cntl.ErrorText().c_str() : "success",
           innerRsp.msg().c_str());
  if (cntl.Failed()) {
    controller->SetFailed(cntl.ErrorText());
  }

  rsp->set_msg(innerRsp.msg());
  done->Run();

  co_return;
}

void MyEchoService::RelayEcho(::google::protobuf::RpcController *controller,
                              const ::echo::EchoRequest *req,
                              ::echo::EchoResponse *rsp,
                              ::google::protobuf::Closure *done) {
  LOG_TRACE("RECEIVED MSG: %s", req->msg().c_str());

  InnerCallMeathod(controller, req, rsp, done);
}

void MyEchoService::GetData(::google::protobuf::RpcController *controller,
                            const ::echo::GetDataReq *request,
                            ::echo::GetDataRsp *response,
                            ::google::protobuf::Closure *done) {
  LOG_TRACE("RECEIVED get MSG");
  response->set_msg(" ---- data from inner rpc call");
  done->Run();
}