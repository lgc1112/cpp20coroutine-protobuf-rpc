/*
 * @Author: ligengchao ligengchao@pku.edu.cn
 * @Date: 2023-07-09 14:40:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-08-09 14:38:18
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

long long printTime = 0;
long long rpcCallCount = 0, failCount = 0, finishCount = 0;
long long rpcCallTimeSum = 0;
long long maxRpcCallTime = 0;
long long minRpcCallTime = LLONG_MAX;
echo::EchoRequest req;
echo::EchoResponse rsp;

RpcCoro CallMeathod(EchoService_MyStub &stub) {
  // 获取当前协程handle并构造proto controller并构造proto rpc stub并
  RpcController cntl(co_await GetHandleAwaiter{});

  long long beginRpcReqTime = llbc::LLBC_GetMicroSeconds();

  // 调用生成的rpc方法Echo,然后挂起协程等待返回
  co_await stub.Echo(&cntl, &req, &rsp, nullptr);

  long long endTime = llbc::LLBC_GetMicroSeconds();
  long long callTime = endTime - beginRpcReqTime;
  rpcCallTimeSum += callTime;
  rpcCallCount++;
  if (cntl.Failed()) {
    ++failCount;
  }

  maxRpcCallTime = std::max(maxRpcCallTime, callTime);
  minRpcCallTime = std::min(minRpcCallTime, callTime);
  //1s打印一次统计信息
  if (endTime - printTime >= 1000000) {
    LOG_INFO("Rpc Statistic fin, Rpc Count:%lld, Fail Count:%lld, Total sum "
             "Time:%lld, Avg Rpc Time:%.2f, Max Time:%lld, Min Time:%lld",
             rpcCallCount, failCount, rpcCallTimeSum,
             (double)rpcCallTimeSum / rpcCallCount, maxRpcCallTime,
             minRpcCallTime);
    printTime = endTime;
    rpcCallTimeSum = 0;
    rpcCallCount = 0;
    failCount = 0;
    maxRpcCallTime = 0;
    minRpcCallTime = LLONG_MAX;
  }
}

bool stop = false;

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  stop = true;
}

RpcCoro CallMeathod() {
  rpcCallCount++;
  co_return;
}

void testCoroTime() {
  long long rpcCallTimeSum = 0;

  long long beginTime = llbc::LLBC_GetMicroSeconds();
  for (int i = 0; i < 1000000; i++) {
    auto func = []() -> RpcCoro {
      rpcCallCount++;
      co_return;
    };
    func();
  }

  long long endTime = llbc::LLBC_GetMicroSeconds();
  rpcCallTimeSum = endTime - beginTime;
  LOG_INFO("Test fin, Count:%lld, Total sum Time:%lld, Avg Time:%.2f",
           rpcCallCount, rpcCallTimeSum, (double)rpcCallTimeSum / rpcCallCount);
}

int main() {
  // testCoroTime();
  // exit(0);

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
    LOG_WARN("Initialize connMgr failed, error:%s", LLBC_FormatLastError());
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
  req.set_msg("Hello, Echo.");

  // 主循环处理 rpc 请求
  while (!stop) {
    // 更新协程管理器，处理超时协程
    s_RpcCoroMgr->Update();
    // 更新连接管理器，处理网络数据包
    s_ConnMgr->Tick();
    if (s_RpcCoroMgr->GetRpcCoroCount() <= 200)
      CallMeathod(stub);
    else
      LLBC_Sleep(1);
  }

  LOG_INFO("client Stop");

  return 0;
}
