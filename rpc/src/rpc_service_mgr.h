/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-19 20:14:54
 * @edit: regangcli
 * @brief:
 */
#pragma once

#include "google/protobuf/message.h"
#include "google/protobuf/service.h"
#include "google/protobuf/stubs/common.h"
#include <map>

namespace llbc {
class LLBC_Packet;
}

class ConnMgr;
class MyController;
class RpcServiceMgr {
public:
  RpcServiceMgr(ConnMgr *connMgr);
  virtual ~RpcServiceMgr();

  // 添加服务
  void AddService(::google::protobuf::Service *service);

private:
  // 处理RPC请求和返回包
  void HandleRpcReq(llbc::LLBC_Packet &packet);
  void HandleRpcRsp(llbc::LLBC_Packet &packet);
  // 处理RPC处理结束
  void OnRpcDone(MyController *controller, google::protobuf::Message *rsp);

private:
  struct ServiceInfo {
    ::google::protobuf::Service *service;
    const ::google::protobuf::ServiceDescriptor *sd;
    std::map<std::string, const ::google::protobuf::MethodDescriptor *> mds;
  };

  std::map<std::string, ServiceInfo> _services;
  ConnMgr *connMgr_ = nullptr;
  bool stop_ = false;
};