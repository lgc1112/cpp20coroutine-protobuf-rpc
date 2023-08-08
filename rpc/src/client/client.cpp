/*
 * @Author: ligengchao ligengchao@pku.edu.cn
 * @Date: 2023-07-09 14:40:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-08-08 23:10:11
 * @FilePath: /projects/newRpc/rpc-demo/src/client/client.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "conn_mgr.h"
#include "echo.pb.h"
#include "echo_service_impl.h"
#include "echo_stub_impl.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_mgr.h"
#include <csignal>
#include <iostream>

using namespace llbc;

RpcCoro CallMeathod(RpcChannel *channel) {
  // 创建rpc req & resp
  echo::EchoRequest req;
  echo::EchoResponse rsp;
  // void *handle = co_await GetHandleAwaiter{};

  // 获取当前协程handle并构造proto controller并构造proto rpc stub并
  RpcController cntl(co_await GetHandleAwaiter{});
  EchoService_MyStub stub(channel);

  req.set_msg("Hello, Echo.");
  LOG_INFO("Rpc Echo Call, msg:%s", req.msg().c_str());
  // 调用生成的rpc方法Echo,然后挂起协程等待返回
  co_await stub.Echo(&cntl, &req, &rsp, nullptr);
  LOG_INFO("Recv Echo Rsp, status:%s, rsp:%s",
           cntl.Failed() ? cntl.ErrorText().c_str() : "success",
           rsp.msg().c_str());

  req.set_msg("Hello, RelayEcho.");
  LOG_INFO("Rpc RelayEcho Call, msg:%s", req.msg().c_str());
  // 调用生成的rpc方法RelayEcho,然后挂起协程等待返回
  co_await stub.RelayEcho(&cntl, &req, &rsp, nullptr);
  LOG_INFO("Recv RelayEcho Rsp, status:%s, rsp:%s",
           cntl.Failed() ? cntl.ErrorText().c_str() : "success",
           rsp.msg().c_str());
  co_return;
}

bool stop = false;

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  stop = true;
}

int main() {
  // 注册信号 SIGINT 和信号处理程序
  signal(SIGINT, signalHandler);

  // 初始化llbc库
  LLBC_Startup();
  LLBC_Defer(LLBC_Cleanup());

  // 初始化日志
  const std::string path = __FILE__;
  const std::string logPath = path.substr(0, path.find_last_of("/\\")) +
                              "/../../log/cfg/server_log.cfg";
  auto ret = LLBC_LoggerMgrSingleton->Initialize(logPath);
  if (ret == LLBC_FAILED) {
    std::cout << "Initialize logger failed, error: " << LLBC_FormatLastError()
              << "path:" << logPath << std::endl;
    return -1;
  }

  // 初始化连接管理器
  s_ConnMgr->Init();
  RpcMgr serviceMgr(s_ConnMgr);
  ret = s_ConnMgr->Start();
  if (ret != LLBC_OK) {
    LOG_TRACE("Initialize connMgr failed, error:%s", LLBC_FormatLastError());
    return -1;
  }

  // 创建rpc channel
  RpcChannel *channel = s_ConnMgr->GetRpcChannel("127.0.0.1", 6688);
  if (!channel) {
    LOG_TRACE("GetRpcChannel Fail");
    return -1;
  }

  LOG_INFO("Client Start!");

  // 创建rpc req & resp
  echo::EchoRequest req;
  echo::EchoResponse rsp;
  req.set_msg("hello, myrpc.");

  // 创建rpc controller & stub
  RpcController cntl;
  EchoService_MyStub stub(channel);

  // 主循环处理 rpc 请求
  int count = 0;
  while (!stop) {
    // 更新协程管理器，处理超时协程
    s_RpcCoroMgr->Update();
    // 更新连接管理器，处理接收到的rpc req和rsp
    s_ConnMgr->Tick();
    LLBC_Sleep(1);
    ++count;
    // 满1s就创建一个新协程发请求包
    if (count == 1000) {
      count = 0;
      LOG_TRACE("CallMeathod Start");
      CallMeathod(channel);
      LOG_TRACE("CallMeathod return");
    }
  }

  LOG_INFO("client Stop");

  return 0;
}
