/*
 * @file: 
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-08-06 19:13:05
 * @edit: regangcli
 * @brief: 
 */
# include "echo_stub_impl.h"

std::suspend_always EchoService_MyStub::Echo(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                    const ::echo::EchoRequest* request,
                    ::echo::EchoResponse* response,
                    ::google::protobuf::Closure* done)
{
    stub.Echo(controller, request, response, done);
    return {};
}

std::suspend_always EchoService_MyStub::RelayEcho(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                    const ::echo::EchoRequest* request,
                    ::echo::EchoResponse* response,
                    ::google::protobuf::Closure* done)
{
    stub.RelayEcho(controller, request, response, done);
    return {};
}

std::suspend_always EchoService_MyStub::GetData(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                    const ::echo::GetDataReq* request,
                    ::echo::GetDataRsp* response,
                    ::google::protobuf::Closure* done)
{
    stub.GetData(controller, request, response, done);
    return {};
}