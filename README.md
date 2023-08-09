# Introduction
cpp20coroutine-protobuf-rpc is a lightweight, simple, and easy-to-understand high-performance RPC implementation framework based on C++20 stackless coroutines and protobuf. With a small amount of code, it implements all the basic features of RPC, making it suitable for learning or as a basis for stackless coroutine-based RPC development. This framework creates only one coroutine for each RPC call, avoiding the creation of multiple layers and improving performance.

# Installation
The project depends on two third-party libraries, [llbc](https://github.com/lailongwei/llbc) and [protobuf](https://github.com/protocolbuffers/protobuf). Since using git submodule is inconvenient, the third-party library code has been directly committed to the repository source code. Users can also download it from the corresponding repository. Here are the installation steps for the project:

- Clone the repository: git clone https://github.com/lgc1112/cppcoro-protobuf-rpc.git
- Enter the repository directory: cd cppcoro-protobuf-rpc
- Compile the llbc library: ./build.sh llbc
- Compile the protobuf-3.20.0 library: ./build.sh proto
- Compile the RPC code: ./build.sh

To run the tests:

- Run the server: ./rpc/bin/server
- Run the client: ./rpc/bin/client

If you need to recompile the RPC code, you can execute the './build.sh' or './build.sh rebuild' command.

Please make sure that you have installed the necessary dependencies and tools, such as a compiler, CMake, etc., before executing these commands.

# Example
## RPC method definition
According to the pb format, define req and rsp protocols, rpc service EchoService, Echo and RelayEcho two rpc methods as follows. Use Google's protoc to generate the corresponding stub for client invocation and service code for server invocation.
```
message EchoRequest {
    optional string msg = 1;
}
message EchoResponse {
    optional string msg = 2;
}

service EchoService {
    rpc Echo(EchoRequest) returns (EchoResponse);
    rpc RelayEcho(EchoRequest) returns (EchoResponse);
}
```
In this case, the Echo method is mainly used to send RPC requests to the server and receive direct responses; RelayEcho is mainly used to send requests to server 1, which then sends another nested RPC request to server 2, and finally, the response is sent back to the client. As shown in the diagram below.

![image](https://github.com/lgc1112/cpp20coroutine-protobuf-rpc/assets/40829436/829a3448-0335-4766-81c6-f461714175c5)

## Client RPC call example
```
// Coroutine method definition
RpcCoro CallMeathod() {
  // 1. Get or create rpc channel
  RpcChannel *channel = s_ConnMgr->GetRpcChannel("127.0.0.1", 6688);
  
  // 2. Create rpc req, rsp
  echo::EchoRequest req;
  req.set_msg("Hello, Echo.");
  echo::EchoResponse rsp;
  
  // 3. Get current coroutine handle and construct proto controller
  RpcController cntl(co_await GetHandleAwaiter{});
  
  // 4. Construct proto rpc stub and call its generated rpc method Echo, then suspend coroutine waiting for return
  echo::EchoService_Stub stub(channel);
  stub.Echo(cntl, req, rsp, nullptr);
  co_await std::suspend_always{};
  
  // 5. When rsp packet is received or timeout error occurs, coroutine is resumed. rpc call status can be read from proto controller, if call is successful, rsp can be read at this time. print coroutine return result
  LLOG("Recv Echo Rsp, status:%s, rsp:%s", cntl.Failed() ? cntl.ErrorText().c_str() : "success", 
       rsp.msg().c_str());
  
  // 6.perform other rpc calls and get returns...
  stub.RelayEcho(cntl, req, rsp, nullptr);
  co_await std::suspend_always{};
  LLOG("Recv RelayEcho Rsp, status:%s, rsp:%s", cntl.Failed() ? cntl.ErrorText().c_str() : "success", 
       rsp.msg().c_str());
  
  co_return;
}
```

![image](https://github.com/lgc1112/cpp20coroutine-protobuf-rpc/assets/40829436/72d9e1bc-2f65-46a9-bdaf-2822c2d6abc0)

The detailed code flow and client sequence diagram are shown above. In step 4, the protobuf-generated stub.Echo method is called to complete the RPC invocation. No additional coroutines will be created during the internal RPC invocation process. The stub.Echo method simply sends the data packet and records the context information before returning. After returning, it suspends at co_await std::suspend_always{} and waits until the RPC response packet is received or a timeout error occurs before being awakened. The entire RPC invocation process will only create this one RpcCoro coroutine, without any nested coroutine calls.



## Server service example:
Server-side RPC service code, implementing the Echo and RelayEcho methods:
```
// Inherit and implement the pb-generated echo::EchoService, override the rpc methods Echo and RelayEcho
class MyEchoService : public echo::EchoService {
public:
  void Echo(::google::protobuf::RpcController *controller,
            const ::echo::EchoRequest *request, ::echo::EchoResponse *response,
            ::google::protobuf::Closure *done) override;
  void RelayEcho(::google::protobuf::RpcController *controller,
             const ::echo::EchoRequest *request, ::echo::EchoResponse *response,
             ::google::protobuf::Closure *done) override;
};

// In the Echo method, fill in the response according to the rpc request rsp, and then return. The framework will complete the response logic internally.
void MyEchoService::Echo(::google::protobuf::RpcController *controller,
                         const ::echo::EchoRequest *request,
                         ::echo::EchoResponse *response,
                         ::google::protobuf::Closure *done) {
  // Fill in the coroutine return value
  response->set_msg(std::string(" Echo >>>>>>> ") + request->msg());
  done->Run();
}

// In the RelayEcho method, since it involves nested rpc calls, create a new coroutine InnerCallMeathod to complete it.
void MyEchoService::RelayEcho(::google::protobuf::RpcController *controller,
                          const ::echo::EchoRequest *req,
                          ::echo::EchoResponse *rsp,
                          ::google::protobuf::Closure *done) {
  LOG_TRACE("RECEIVED MSG: %s",
       req->msg().c_str());

  InnerCallMeathod(controller, req, rsp, done);
}

// InnerCallMeathod coroutine
RpcCoro InnerCallMeathod(::google::protobuf::RpcController *controller,
                         const ::echo::EchoRequest *req,
                         ::echo::EchoResponse *rsp,
                         ::google::protobuf::Closure *done) {
  // Initialize internal rpc innerReq innerRsp
  echo::EchoRequest innerReq;
  innerReq.set_msg("Relay Call >>>>>>" + req->msg());
  echo::EchoResponse innerRsp;
  
  // Get rpc channel
  RpcChannel *channel = s_ConnMgr->GetRpcChannel("127.0.0.1", 6699);

  // Suspend after internal rpc call
  MyController cntl(co_await GetHandleAwaiter{});
  cntl.SetPtrParam(handle);
  echo::EchoService_Stub stub(channel);
  stub.Echo(cntl, innerReq, innerRsp, nullptr);
  co_await std::suspend_always{};
  
  // When the rsp package is received or an error such as a timeout occurs, this coroutine is awakened and the coroutine return result is printed
  LLOG("Recv rsp, status:%s, rsp:%s",
       cntl.Failed() ? cntl.ErrorText().c_str() : "success",
       innerRsp.msg().c_str());
  if (cntl.Failed()) {
    controller->SetFailed(cntl.ErrorText());
  }

  // Fill in the coroutine return value
  rsp->set_msg(innerRsp.msg());
  done->Run();

  co_return;
}
```
Server-side RPC service process, including Echo and RelayEcho methods:

![image](https://github.com/lgc1112/cpp20coroutine-protobuf-rpc/assets/40829436/9c1dcea5-5d3c-42e7-85de-a08847dd856d)

The Echo method returns directly after being processed in the main coroutine, while the RelayEcho method, which requires a nested RPC call, creates a new thread internally to complete the call.

##  Client log print results
```
Recv Echo Rsp, status:success, rsp: Echo >>>>>>> hello, Echo.
Recv RelayEcho Rsp, status:success, rsp: Echo >>>>>>> Relay Call >>>>>> hello, RelayEcho.
```
As you can see, the first Echo method received the echo return, and the second RelayEcho method received the echo return after two server relay accesses, which can achieve the basic rpc call and nested rpc call of rpc.

## Rpc implementation framework

<img width="1479" alt="企业微信截图_80776009-59f2-4f9f-806a-00e838d83358" src="https://github.com/lgc1112/cpp20coroutine-protobuf-rpc/assets/40829436/2ce0f3e4-d8e9-4f50-a5e5-961afbc76c36">


This implementation framework is based on protobuf and C++20 coroutines.

protobuf: Define req and rsp protocols, service, and corresponding rpc methods according to the pb format. Using the protoc interpreter, you can generate rpc stub classes and interfaces for clients and rpc service classes and interfaces for servers. Using protobuf can greatly reduce the amount of code and the difficulty of understanding our rpc framework (although I personally think that the performance of the rpc code generated by protobuf is not particularly high...). Those who are not familiar with protobuf-generated rpc code can first get a general understanding. Since they are all classic rpc solutions, they will not be introduced too much here. In short, after defining the rpc methods and services through pb, rpc stub classes and rpc call methods will be generated for clients. After users call the corresponding rpc methods, the internal google::protobuf::RpcChannel class's CallMethod method will be called. Developers can rewrite CallMethod to do their own packet sending logic; At the same time, rpc service classes and rpc service methods will be generated for the server. The server needs to inherit and override these methods to implement its own rpc processing logic and fill in rsp (as in Section 2.3). So when an rpc request is received, the service class is found according to the service name and method name, and its CallMethod interface is called, which will eventually call the user-overridden method and obtain rsp.

C++20 coroutines: There are many explanations for C++20 coroutines at present. In short, C++20 coroutine is a stackless coroutine, which means that the suspension point of the coroutine can only be inside the coroutine method and cannot be suspended inside internal functions. Because C++20 coroutines do not need to maintain stack information and have compiler optimization support, their performance is relatively high. This article only encapsulates C++20 coroutines in the most primitive way, implementing the basic functionality required for Rpc, which is simple and easy to understand.

Tcp communication: For the Tcp communication part, in order to avoid reinventing the wheel, this article mainly uses a third-party library llbc to handle Tcp connection, disconnection, and exception handling. On this basis, a multi-threaded Tcp communication manager is implemented to reduce the overhead of the main thread and improve performance.

# Performance Testing
In this framework,  a single RPC call on the client side in this article only involves the creation and switching of one coroutine, without the issue of nested coroutines at various levels; the server side can either not create a coroutine or only create one. In addition, the connection management module uses a dual-thread approach to reduce the load on the main thread. Therefore, theoretically, this framework has excellent performance.

## Test Environment
|Name|Value|
| --- | --- | 
|CPU  | AMD EPYC 7K62 48-Core Processor |
|CPU Frequency|2595.124MHz|
|Image|Tencent Cloud tLinux 2.2-Integrated Edition|
|System|centOS7|
|CPU Cores|16|
|Memory|32G|
## Test Steps
In rpc_def.h, uncomment #define EnableRpcPerfStat, then run the generated ./server and ./client_stress_test for testing. Client output results are as follows:

Client output results are as follows:
```
23-08-08 16:11:14.752869 [INFO ][client_stress_test.cpp:57 CallMeathod]<> - Rpc Statistic fin, Count:112005, Fail Count:0, Total sum Time:381168879, Avg Time:3403.14, Max Time:6207, Min Time:369
23-08-08 16:11:15.752873 [INFO ][client_stress_test.cpp:57 CallMeathod]<> - Rpc Statistic fin, Count:109834, Fail Count:0, Total sum Time:427922472, Avg Time:3896.08, Max Time:25800, Min Time:386
23-08-08 16:11:16.752881 [INFO ][client_stress_test.cpp:57 CallMeathod]<> - Rpc Statistic fin, Count:110315, Fail Count:0, Total sum Time:424517227, Avg Time:3848.23, Max Time:25989, Min Time:273
23-08-08 16:11:17.752884 [INFO ][client_stress_test.cpp:57 CallMeathod]<> - Rpc Statistic fin, Count:117797, Fail Count:0, Total sum Time:378102349, Avg Time:3209.78, Max Time:24484, Min Time:159
23-08-08 16:11:18.752928 [INFO ][client_stress_test.cpp:57 CallMeathod]<> - Rpc Statistic fin, Count:116320, Fail Count:0, Total sum Time:401001834, Avg Time:3447.40, Max Time:30458, Min Time:162
23-08-08 16:11:19.752945 [INFO ][client_stress_test.cpp:57 CallMeathod]<> - Rpc Statistic fin, Count:114063, Fail Count:0, Total sum Time:418513161, Avg Time:3669.14, Max Time:32304, Min Time:242
```
Server output results are as follows:
```
23-08-08 22:57:20.765787 [INFO ][rpc_mgr.cpp:97 HandleRpcReq]<> - Rpc Statistic fin, Count:221057, Total sum Time:881181, Avg Time:3.99, Max Time:99, Min Time:2
23-08-08 22:57:21.765789 [INFO ][rpc_mgr.cpp:97 HandleRpcReq]<> - Rpc Statistic fin, Count:225007, Total sum Time:881178, Avg Time:3.92, Max Time:76, Min Time:2
23-08-08 22:57:22.765791 [INFO ][rpc_mgr.cpp:97 HandleRpcReq]<> - Rpc Statistic fin, Count:225047, Total sum Time:807463, Avg Time:3.59, Max Time:36, Min Time:2
23-08-08 22:57:23.765796 [INFO ][rpc_mgr.cpp:97 HandleRpcReq]<> - Rpc Statistic fin, Count:233627, Total sum Time:823445, Avg Time:3.52, Max Time:87, Min Time:2
```

## Test Reuslt
Since our RPC server supports processing RPC with and without coroutines, we measure the performance in both cases.

First, create 1 million coroutines and execute them, the average time for creating and executing each libco coroutine is: **61.6ns**.

Server-side measurement results:

|Test Function | No Coroutine | 	Coroutine |
| --- | --- | --- |
| Average processing time per RPC | 3.36us	|3.38 us|
| QPS|216,000 (2 clients, 99% CPU)|213,800 (2 clients, 99% CPU)|

According to the above test results, the switching execution time of C++20 coroutines is only 61.6ns. Therefore, whether or not coroutines are enabled to process RPC, the processing time of each Rpc remains almost unchanged, with an actual QPS of approximately 216,000, and the performance is excellent. Since from an implementation perspective, a single RPC call in this article only involves the creation and switching of one coroutine, there is no issue of nested coroutines at various levels. In addition, the connection management module also uses a dual-thread approach to reduce the load on the main thread. As a result, the performance of this framework is outstanding, with a measured QPS of up to 216,000 and a theoretical maximum QPS of 1 / 3.36us = 297,000. When the sleep time of the client and server is changed to 0, the average latency of the same machine Rpc call is about 1ms, and the minimum latency is 50us.

In addition, I have also implemented an RPC framework based on stackful coroutines using libco, which can be seen at: [Lightweight, High-Performance RPC Framework Based on libco and protobuf](https://github.com/lgc1112/libco-protobuf-rpc). Its measurement results are as follows:

|Test Function | No Coroutine | 	Coroutine |
| --- | --- | --- |
| Average processing time per RPC| 3.33us	|5.68 us|
| QPS|218,800 (3 clients, 99% CPU)|125,000 (2 clients, 99% CPU)|

From the above results, it can be seen that when not using coroutines to process RPC, there is not much difference in the RPC processing time and QPS between the two coroutine frameworks. However, since the switching time of stackful coroutines is about 700 times that of stackless coroutines, when using coroutines to process RPC, the QPS of the stackful coroutine solution is 40% lower than that of the stackless coroutine solution. But the advantage of the stackful coroutine solution is its higher usability.

# Conclusion
Compared to stackful coroutine RPC frameworks, the advantage of the framework in this article lies in its higher performance. The switching efficiency of C++20 coroutines is far superior to that of stackful coroutines, as they do not need to preserve stack information, and as a language-level design, they can fully utilize the optimization capabilities of the compiler, thus providing excellent performance. However, the drawback is also quite obvious, that is, RPC methods must be called within coroutine methods and cannot be freely called within nested functions. Essentially, it can be regarded as a form of asynchronous call. From an implementation perspective, a single RPC call on the client side in this article only involves the creation and switching of one coroutine, without the issue of nested coroutines at various levels; the server side can either not create a coroutine or only create one. In addition, the connection management module uses a dual-thread approach to reduce the load on the main thread. Therefore, theoretically, this framework has excellent performance.

To achieve synchronous RPC calls within nested functions of coroutine methods, some stackless coroutine RPC frameworks do the following: if an RPC call is needed within an internal function, then all functions in the call stack must be coroutines, i.e., a single RPC call is completed by multiple nested coroutines. Another approach is to manually save stack information, but this is essentially a stackful coroutine solution. Moreover, the nested stackless coroutine call method significantly increases the complexity of use, and its performance is not necessarily more efficient than a single stackful coroutine.

This article's framework can also support nested coroutine calls by simply adding a nested call coroutine library and initiating an RPC in the innermost coroutine. Since this article is more focused on implementing a lightweight, high-performance RPC framework, it only provides a simple encapsulation of C++20 coroutines and does not integrate some complex coroutine scheduling libraries. However, in theory, integrating these coroutine libraries is not a problem. To compare and study with stackful coroutine RPC frameworks, I have also implemented a similar stackful coroutine RPC, which can be seen at: Lightweight, High-Performance RPC Framework Based on libco and protobuf. This framework has much better usability because stackless coroutine RPC often sacrifices some usability for performance.


