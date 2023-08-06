
#include "rpc_coro_mgr.h"
#include <csignal>
#include "conn_mgr.h"
#include "echo_service_impl.h"
#include "llbc.h"
#include "rpc_channel.h"
#include "rpc_mgr.h"

RpcCoroMgr coro_mgr;
int main() {
    auto func = [](int id) -> std::suspend_always { 
        std::cout << "Coroutine entry function called for coroutine with id " << id << std::endl; 
        return coro_mgr.get_coro(id)->yield();
        // co_await Awaiter{id};
        std::cout << "Coroutine entry function called after yield " << id << std::endl; 
        // co_return;
    };
    
    int coro_id = coro_mgr.create_coro(func, nullptr);
    
    RpcCoro *coro = coro_mgr.get_coro(coro_id);
    if (coro) {
        std::cout << "resume1" << std::endl;
        coro->resume();
        std::cout << "resume2" << std::endl;
        coro->resume();
        std::cout << "resume3" << std::endl;
        coro->resume();
        std::cout << "yield" << std::endl;
        coro->yield();
        std::cout << "444" << std::endl;
        coro->cancel();
        std::cout << "Coroutine with id " << coro_id << " canceled" << std::endl;
    }
}



void Call(void* parentCoro) {
    
    echo::EchoRequest req;
    echo::EchoResponse rsp;
    req.set_msg("hello, myrpc.");

    std::cout << "CallMeathod handle:" << parentCoro << std::endl;
    
    s_RpcCoroMgr->AddRpcCoroInfo(parentCoro, &rsp);
    // s_RpcCoroMgr->CreateRpcCoro(parentCoro, &rsp);
}


RpcCoro CallMeathod() {
    void *handle = co_await GetHandleAwaiter{}; // Specify the type explicitly
    Call(handle);
    LOG_TRACE("call ing");
    co_await std::suspend_always{};
    std::cout << "CallMeathod return" << std::endl;
    co_return;
}

int main2() {
    CallMeathod();
    {
        std::cout << "resume1" << std::endl;
        s_RpcCoroMgr->ResumeRpcCoro(1);
        std::cout << "Coroutine with id " << 1 << " canceled" << std::endl;
    }
    return 0;
}