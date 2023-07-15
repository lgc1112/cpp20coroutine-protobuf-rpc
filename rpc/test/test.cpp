#include <iostream>
#include <coroutine>

struct MyCoroutine {
    struct promise_type {
        MyCoroutine get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    std::coroutine_handle<promise_type> handle;

    MyCoroutine(std::coroutine_handle<promise_type> h) : handle(h) {} // Remove 'explicit' keyword
    ~MyCoroutine() { }
};

struct GetHandleAwaiter {
    bool await_ready() const noexcept { return false; }
    
    template <typename PromiseType>
    bool await_suspend(std::coroutine_handle<PromiseType> handle) {
        handle_ = handle;
        
        std::cout << "await_suspend" << std::endl;
        return false;
    }
    
    std::coroutine_handle<MyCoroutine::promise_type> await_resume() noexcept { 
        std::cout << "await_resume" << std::endl; return handle_; }
    
    std::coroutine_handle<MyCoroutine::promise_type> handle_;
};

MyCoroutine my_coroutine(std::coroutine_handle<MyCoroutine::promise_type> &handle) {
    std::cout << "my_coroutine start " << std::endl;
    std::coroutine_handle<MyCoroutine::promise_type> handle2 = co_await GetHandleAwaiter{}; // Specify the type explicitly
    std::cout << "Coroutine handle: " << handle2.address() << std::endl;
    handle = handle2;
    std::cout << "Coroutine handle2: " << handle.address() << std::endl;
    co_await std::suspend_always{};
    std::cout << "Coroutine resume: " << handle.address() << std::endl;

    co_return;
}

int main() {
    std::cout << "main start" << std::endl;
    std::coroutine_handle<MyCoroutine::promise_type> handle;
    my_coroutine(handle);
    std::cout << "get Coroutine handle " << handle.address() << std::endl;
    std::cout << "handle resume" << std::endl;
    handle.resume();
    std::cout << "main stop" << std::endl;
    return 0;
}