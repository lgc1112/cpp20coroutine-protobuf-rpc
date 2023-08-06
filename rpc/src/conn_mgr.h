/*
 * @file:
 * @Author: ligengchao
 * @Date: 2023-06-19 20:39:04
 * @edit: ligengchao
 * @brief:
 */

#pragma once

#include "llbc.h"
#include "rpc_def.h"
using namespace llbc;

class RpcChannel;

enum RpcOpCode {
  RpcReq = 1,
  RpcRsp = 2,
};

enum ServerType {
  Server1 = 1,
  Server2 = 2,
};

#define Server1Ip "127.0.0.1"
#define Server1Port "6688"

#define Server2Ip "127.0.0.1"
#define Server2Port "6699"

#define SendQueueSize 1024 * 1024
#define RecvQueueSize 1024 * 1024
// 简单双线程无锁循环队列, 只提供两个方法, Push和Pop分别只能由一个线程调用
template <typename T, int QueueCapacity> class LockFreeQueue {
public:
  bool Push(T *);
  T *Pop();

  // bool IsEmpty() { return rdIdx_ == wtIdx_; }
  int GetSize();
  int GetCapacity() { return QueueCapacity; };

private:
  volatile int rdIdx_ = 0;
  volatile int wtIdx_ = 0;
  T *queue_[QueueCapacity] = {}; // 数据队列
};

template <typename T, int QueueCapacity>
bool LockFreeQueue<T, QueueCapacity>::Push(T *val) {
  int size = wtIdx_ - rdIdx_;
  if (size < 0)
    size += QueueCapacity;

  // 队列已满（还剩1个位置就算满了）
  if (QueueCapacity - size == 1)
    return false;

  queue_[wtIdx_] = val;
  // 写内存barrier, 防止乱序执行导致queue_[wtIdx_]
  // 值未完全写入，后台线程就开始处理了
  __asm__ __volatile__("sfence" ::: "memory");
  wtIdx_ = (wtIdx_ + 1) % QueueCapacity;
  return true;
}

template <typename T, int QueueCapacity>
T *LockFreeQueue<T, QueueCapacity>::Pop() {
  if (rdIdx_ != wtIdx_) {
    auto val = queue_[rdIdx_];
#ifdef NEXT_DEBUG
    queue_[rdIdx_] = nullptr;
    __asm__ __volatile__("sfence" ::: "memory");
#endif
    rdIdx_ = (rdIdx_ + 1) % QueueCapacity;
    return val;
  }

  return nullptr;
}

template <typename T, int QueueCapacity>
int LockFreeQueue<T, QueueCapacity>::GetSize() {
  int size = wtIdx_ - rdIdx_;
  if (size < 0)
    size += QueueCapacity;
  return size;
}

// 连接管理组件
class ConnComp : public LLBC_Component {
public:
  ConnComp();
  virtual ~ConnComp(){};

public:
  // 放入发送包
  int PushPacket(LLBC_Packet *sendPacket);
  // 取出接收包
  LLBC_Packet *PopPacket() { return recvQueue_.Pop(); }

  int GetSendQueueSize() { return sendQueue_.GetSize(); }

public:
  virtual bool OnInit(bool &initFinished);
  virtual void OnDestroy(bool &destroyFinished);
  virtual void OnUpdate();

public:
  // 网络连接事件监控
  virtual void OnSessionCreate(const LLBC_SessionInfo &sessionInfo);
  virtual void OnSessionDestroy(const LLBC_SessionDestroyInfo &destroyInfo);
  virtual void OnAsyncConnResult(const LLBC_AsyncConnResult &result);
  virtual void OnUnHandledPacket(const LLBC_Packet &packet);
  virtual void OnProtoReport(const LLBC_ProtoReport &report);

public:
  // 接收数据回调
  void OnRecvPacket(LLBC_Packet &packet);

private:
  LockFreeQueue<LLBC_Packet, SendQueueSize> sendQueue_;
  LockFreeQueue<LLBC_Packet, RecvQueueSize> recvQueue_;
};

// 连接管理器
class ConnMgr {
public:
  ConnMgr();
  virtual ~ConnMgr();

public:
  // 初始化
  int Init();
  // 启动rpc网络服务
  int StartRpcService(const char *ip, int port);
  // 获取/创建rpc客户端通信通道
  RpcChannel *GetRpcChannel(const char *ip, int port);

  // 销毁rpc客户端通信通道
  int CloseSession(int sessionId);
  // 放入发送包
  int PushPacket(LLBC_Packet *sendPacket) {
    return comp_->PushPacket(sendPacket);
  }
  // 获取发送队列大小
  int GetSendQueueSize() { return comp_->GetSendQueueSize(); }
  // 取出接收包
  LLBC_Packet *PopPacket() { return comp_->PopPacket(); }
  // 获取服务器SessionId
  int GetServerSessionId() { return serverSessionId_; }
  // 是否是服务器
  bool IsServer() { return isServer_; }
  // 更新处理收到的Rpc数据包，需要主线程定时Tick
  bool Tick();
  // 订阅指定cmdId的数据包处理回调
  int Subscribe(int cmdId, const LLBC_Delegate<void(LLBC_Packet &)> &deleg);
  // 取消订阅cmdId的数据包
  void Unsubscribe(int cmdId);

private:
  bool isServer_ = false;
  LLBC_Service *svc_ = nullptr;
  ConnComp *comp_ = nullptr;
  int serverSessionId_ = 0;
  std::unordered_map<int, LLBC_Delegate<void(LLBC_Packet &)>>
      packetDelegs_; // {cmdId : delegate}
  std::unordered_map<std::string, RpcChannel *> addr2Channel_;
  std::unordered_map<int, std::string> session2Addr_;
};

#define s_ConnMgr ::llbc::LLBC_Singleton<ConnMgr>::Instance()

