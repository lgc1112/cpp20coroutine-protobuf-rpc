# Introduction
cpp20coroutine-protobuf-rpc is a lightweight, simple, and easy-to-understand high-performance synchronous RPC implementation framework based on C++20 stackless coroutines and protobuf. With a small amount of code, it implements all the basic features of RPC, making it suitable for learning or as a basis for stackless coroutine-based RPC development. This framework creates only one coroutine for each RPC call, avoiding the creation of multiple layers and improving performance.

# Installation
The project depends on two third-party libraries, llbc and protobuf. Since using git submodule is inconvenient, the third-party library code has been directly committed to the repository source code. Users can also download it from the corresponding repository. Here are the installation steps for the project:

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
The detailed process and introduction of the code are shown above. In step 4, the rpc call is completed by calling the protobuf-generated stub.Echo method. During the internal rpc call process, no additional coroutines will be generated. The stub.Echo method only completes the data packet sending and records the context information and returns. After returning, it suspends and waits at co_await std::suspend_always{} until the rpc return packet is received or a timeout error occurs before being awakened. The entire Rpc call process only creates this one coroutine.

## Server service example:
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
  LLOG(nullptr, nullptr, LLBC_LogLevel::Trace, "RECEIVED MSG: %s",
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
In this framework, a single RPC call in this article only involves the creation and switching of one coroutine, and there is no problem of nested coroutines at various levels. In addition, the connection management module also uses dual-threading to reduce the workload of the main thread. Therefore, theoretically, the performance of this framework is very superior.

Specific test data and results will be supplemented later. 

// TODO: Specific data to be supplemented...


