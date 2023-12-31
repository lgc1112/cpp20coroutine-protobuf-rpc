/*
 * @file:
 * @Author: ligengchao

 * @Date: 2023-06-21 15:46:03
 * @edit: ligengchao
 * @brief:
 */
#pragma once
#include "echo.pb.h"
#include "rpc_def.h"

class RpcController;
class MyEchoService : public echo::EchoService {
public:
  void Echo(::google::protobuf::RpcController *controller,
            const ::echo::EchoRequest *request, ::echo::EchoResponse *response,
            ::google::protobuf::Closure *done) override;
  void RelayEcho(::google::protobuf::RpcController *controller,
                 const ::echo::EchoRequest *request,
                 ::echo::EchoResponse *response,
                 ::google::protobuf::Closure *done) override;

  void GetData(::google::protobuf::RpcController *controller,
               const ::echo::GetDataReq *request, ::echo::GetDataRsp *response,
               ::google::protobuf::Closure *done) override;
};