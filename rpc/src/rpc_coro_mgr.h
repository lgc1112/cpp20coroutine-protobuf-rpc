/*
 * @file:
 * @Author: regangcli
 * @copyright: Tencent Technology (Shenzhen) Company Limited
 * @Date: 2023-06-20 20:13:23
 * @edit: regangcli
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
class MyController;

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

class CoroInfo {
public:
  CoroInfo(int coroId, RpcCoroMgr *coroMgr, MyController *controller,
           google::protobuf::Message *rsp, int timeoutTime = CoroTimeoutTime)
      : id_(coroId), coroMgr_(coroMgr), controller_(controller), rsp_(rsp),
        timeoutTime_(llbc::LLBC_GetMilliSeconds() + timeoutTime) {}

  MyController *GetController() const { return controller_; }
  void *GetAddr() const;
  int GetId() const { return id_; }
  int DecodeRsp(llbc::LLBC_Packet *packet);
  llbc::sint64 GetTimeoutTime() const { return timeoutTime_; }

  void Resume();

  void OnCoroTimeout();
  void OnCoroCancel();

private:
  int id_;
  RpcCoroMgr *coroMgr_;
  MyController *controller_;  
  google::protobuf::Message *rsp_;
  llbc::sint64 timeoutTime_;
};

struct CoroInfoWeatherCmp {
  bool operator()(CoroInfo *a, CoroInfo *b) {
    return a->GetTimeoutTime() < b->GetTimeoutTime();
  }
};

//
class RpcCoroMgr {
public:
  RpcCoroMgr() {}
  ~RpcCoroMgr();

  int AddRpcCoro(MyController *controller, google::protobuf::Message *rsp);
  CoroInfo *GetRpcCoroInfo(int coroId);
  void ResumeRpcCoro(int coroId);
  void Update(); // 处理超时协程

  llbc::LLBC_Packet *GetRecvPacket() { return recvPacket_; }
  void SetRecvPacket(llbc::LLBC_Packet *packet) { recvPacket_ = packet; }

private:
  std::unordered_map<int, CoroInfo *> coroInfos_;
  int maxCoroId = 1;
  llbc::LLBC_Packet *recvPacket_ = nullptr;

  llbc::LLBC_BinaryHeap<CoroInfo *, CoroInfoWeatherCmp>
      coroTimeHeap_; // 超时时间小根堆堆
};

#define s_rpcCoroMgr ::llbc::LLBC_Singleton<RpcCoroMgr>::Instance()
