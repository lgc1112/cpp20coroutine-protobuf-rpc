/*
 * @file:
 * @Author: ligengchao

 * @Date: 2023-06-20 20:13:23
 * @edit: ligengchao
 * @brief:
 */

#pragma once

#include "llbc.h"

#include "echo.pb.h"
#include <coroutine>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace llbc {
class LLBC_Packet;
}

class RpcCoroMgr;
class RpcController;

#define CoroTimeoutTime 5000 // milli seconds
class RpcCoro {
public:
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {
    int unique_id;

    auto get_return_object() {
      return RpcCoro{handle_type::from_promise(*this)};
    }

    auto initial_suspend() { return std::suspend_never{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void unhandled_exception() { std::terminate(); }
    void return_void() {}
    auto yield_value() { return std::suspend_always{}; }
  };

  RpcCoro(RpcCoro const &) = delete;
  RpcCoro(RpcCoro &&rhs) : coro_handle(rhs.coro_handle) {
    rhs.coro_handle = nullptr;
  }

  ~RpcCoro() {}

  int get_id() const { return coro_handle.promise().unique_id; }
  void resume() { coro_handle.resume(); }
  auto yield() { return coro_handle.promise().yield_value(); }
  void cancel() {
    coro_handle.destroy();
    coro_handle = nullptr;
  }

private:
  RpcCoro(handle_type h) : coro_handle(h) {}

  handle_type coro_handle;
};

struct GetHandleAwaiter {
  bool await_ready() const noexcept { return false; }
  bool await_suspend(std::coroutine_handle<RpcCoro::promise_type> handle) {
    handle_ = handle.address();
    return false;
  }

  void *await_resume() noexcept { return handle_; }

  void *handle_;
};

class RpcCoroInfo {
public:
  RpcCoroInfo(int coroId, RpcCoroMgr *coroMgr, RpcController *controller,
           google::protobuf::Message *rsp, int timeoutTime = CoroTimeoutTime)
      : id_(coroId), coroMgr_(coroMgr), controller_(controller), rsp_(rsp),
        timeoutTime_(llbc::LLBC_GetMilliSeconds() + timeoutTime) {}

  // 获取rpc协程的Controller
  RpcController *GetController() const { return controller_; }
  // 获取rpc协程的handle
  void *GetHandle() const;
  // 获取rpc协程的id
  int GetId() const { return id_; }
  // 根据接收到的packet解析出rsp
  int DecodeRsp(llbc::LLBC_Packet *packet);
  // 获取超时时间
  llbc::sint64 GetTimeoutTime() const { return timeoutTime_; }
  // 恢复协程
  void Resume();

  void OnCoroTimeout();
  void OnCoroCancel();

private:
  int id_;
  RpcCoroMgr *coroMgr_;
  RpcController *controller_;  
  google::protobuf::Message *rsp_;
  llbc::sint64 timeoutTime_;
};

struct CoroInfoWeatherCmp {
  bool operator()(RpcCoroInfo *a, RpcCoroInfo *b) {
    return a->GetTimeoutTime() < b->GetTimeoutTime();
  }
};

// 协程管理器
class RpcCoroMgr {
public:
  RpcCoroMgr() {}
  ~RpcCoroMgr();

  // 添加协程信息
  int AddRpcCoroInfo(RpcController *controller, google::protobuf::Message *rsp);
  // 获取协程信息
  RpcCoroInfo *GetRpcCoroInfo(int coroId);
  // 恢复对应的协程
  void ResumeRpcCoro(int coroId);
  // 主协程定时调用，处理超时协程
  void Update();

  llbc::LLBC_Packet *GetRecvPacket() { return recvPacket_; }
  void SetRecvPacket(llbc::LLBC_Packet *packet) { recvPacket_ = packet; }

private:
  std::unordered_map<int, RpcCoroInfo *> coroInfos_;
  int maxCoroId = 1;
  llbc::LLBC_Packet *recvPacket_ = nullptr;

  llbc::LLBC_BinaryHeap<RpcCoroInfo *, CoroInfoWeatherCmp>
      coroTimeHeap_; // 超时时间小根堆堆
};

#define s_RpcCoroMgr ::llbc::LLBC_Singleton<RpcCoroMgr>::Instance()
