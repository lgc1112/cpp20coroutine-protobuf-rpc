/*
 * @Author: ligengchao ligengchao@pku.edu.cn
 * @Date: 2023-07-09 14:40:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-07-17 21:48:37
 * @FilePath: /projects/newRpc/rpc-demo/src/server/server2.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
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

  ConnMgr *connMgr = s_ConnMgr;
  connMgr->Init();

  // 启动rpc服务
  if (connMgr->StartRpcService("127.0.0.1", 6699) != LLBC_OK) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace,
         "connMgr StartRpcService Fail");
    return -1;
  }

  RpcMgr serviceMgr(connMgr);
  MyEchoService echoService;
  serviceMgr.AddService(&echoService);

  // 死循环处理rpc请求
  while (!stop) {
    connMgr->Tick();
    s_RpcCoroMgr->Update();
    LLBC_Sleep(1);
  }

  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "server Stop");

  return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
