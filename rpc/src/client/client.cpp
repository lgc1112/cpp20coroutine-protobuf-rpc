#include "conn_mgr.h"
#include "echo.pb.h"
#include "echo_service_impl.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_coro_mgr.h"
#include "rpc_service_mgr.h"
#include <csignal>
#include <iostream>

using namespace llbc;

RpcCoro CallMeathod(RpcChannel *channel) {
  // 创建rpc req & resp
  echo::EchoRequest req;
  echo::EchoResponse rsp;
  req.set_msg("hello, myrpc.");
  void *handle = co_await GetHandleAwaiter{};

  LLOG(nullptr, nullptr, LLBC_LogLevel::Info, "Rpc call, msg:%s",
       req.msg().c_str());
  MyController cntl;
  cntl.SetPtrParam(handle);
  echo::EchoService_Stub stub(channel);
  stub.Echo(&cntl, &req, &rsp, nullptr);
  co_await std::suspend_always{};
  LLOG(nullptr, nullptr, LLBC_LogLevel::Info, "Recv rsp, status:%s, rsp:%s", cntl.Failed() ? cntl.ErrorText().c_str() : "success", 
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
  const std::string logPath = path.substr(0, path.find_last_of("/\\"))+ "/../../log/cfg/server_log.cfg";
  auto ret = LLBC_LoggerMgrSingleton->Initialize(logPath);
  if (ret == LLBC_FAILED) {
    std::cout << "Initialize logger failed, error: " << LLBC_FormatLastError()
              << "path:" << logPath
              << std::endl;
    return -1;
  }

  // 初始化连接管理器
  ConnMgr *connMgr = s_ConnMgr;
  ret = connMgr->Init();
  if (ret != LLBC_OK) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace,
         "Initialize connMgr failed, error:%s", LLBC_FormatLastError());
    return -1;
  }

  // 创建rpc channel
  RpcChannel *channel = connMgr->CreateRpcChannel("127.0.0.1", 6688);
  if (!channel) {
    LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CreateRpcChannel Fail");
    return -1;
  }

  LLOG(nullptr, nullptr, LLBC_LogLevel::Info, "Client Start!");

  // 创建rpc req & resp
  echo::EchoRequest req;
  echo::EchoResponse rsp;
  req.set_msg("hello, myrpc.");

  // 创建rpc controller & stub
  MyController cntl;
  echo::EchoService_Stub stub(channel);
  RpcServiceMgr serviceMgr(connMgr);

  // 死循环处理 rpc 请求
  int count = 0;
  while (!stop) {
    // tick 处理接收到的 rpc req和rsp
    // 若有rsp，Tick内部会调用Rsp处理函数，从而唤醒对应休眠的协程
    g_rpcCoroMgr->Update();
    auto isBusy = connMgr->Tick();
    if (!isBusy) {
      LLBC_Sleep(1);
      ++count;
      // 满1s就创建一个新协协程发一个包
      if (count == 1000) {
        count = 0;
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CallMeathod Start");
        CallMeathod(channel);
        LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "CallMeathod return");
      }
    }
  }

  LLOG(nullptr, nullptr, LLBC_LogLevel::Info, "client Stop");

  return 0;
}
