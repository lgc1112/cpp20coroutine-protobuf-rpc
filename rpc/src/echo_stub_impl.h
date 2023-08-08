/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-08-06 19:13:16
 * @edit: regangcli
 * @brief:
 */

#pragma once

#include "echo.pb.h"
#include <coroutine>
#define PROTOBUF_NAMESPACE_ID google::protobuf
class EchoService_MyStub {
public:
  EchoService_MyStub(::PROTOBUF_NAMESPACE_ID::RpcChannel *channel)
      : stub(channel){};
  ~EchoService_MyStub(){};

  std::suspend_always Echo(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
                           const ::echo::EchoRequest *request,
                           ::echo::EchoResponse *response,
                           ::google::protobuf::Closure *done);
  std::suspend_always
  RelayEcho(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
            const ::echo::EchoRequest *request, ::echo::EchoResponse *response,
            ::google::protobuf::Closure *done);
  std::suspend_always
  GetData(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
          const ::echo::GetDataReq *request, ::echo::GetDataRsp *response,
          ::google::protobuf::Closure *done);

private:
  ::echo::EchoService_Stub stub;
};
