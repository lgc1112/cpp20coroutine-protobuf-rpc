/*
 * @Author: ligengchao ligengchao@pku.edu.cn
 * @Date: 2023-07-09 17:19:49
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-17 21:48:16
 * @FilePath: /projects/newRpc/rpc-demo/src/server/server.cpp
 */
#include "conn_mgr.h"
#include "echo_service_impl.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_mgr.h"
#include <csignal>

using namespace llbc;

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
  LLBC_HookProcessCrash();

  // 初始化日志
  const std::string path = __FILE__;
  const std::string logPath = path.substr(0, path.find_last_of("/\\"))+ "/../../log/cfg/server_log.cfg";
  auto ret = LLBC_LoggerMgrSingleton->Initialize(logPath);
  if (ret == LLBC_FAILED) {
    std::cout << "Initialize logger failed, error: " << LLBC_FormatLastError()
              << "path:" << logPath
              << std::endl;
    return -1;
  }
  
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "Hello Server!");

  // 初始化rpc协程管理器
  s_ConnMgr->Init();

  // 启动rpc监听服务
  if (s_ConnMgr->StartRpcService("127.0.0.1", 6688) != LLBC_OK) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace,
         "connMgr StartRpcService Fail");
    return -1;
  }

  // 添加rpc服务
  RpcMgr serviceMgr(s_ConnMgr);
  MyEchoService echoService;
  serviceMgr.AddService(&echoService);

  // 主循环
  while (!stop) {
    // 更新协程管理器，处理超时协程
    s_RpcCoroMgr->Update(); 
    // 更新连接管理器，处理接收到的rpc req和rsp
    auto isBusy = s_ConnMgr->Tick();
    if (!isBusy) 
      LLBC_Sleep(1);
  }

  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "server Stop");

  return 0;
}

