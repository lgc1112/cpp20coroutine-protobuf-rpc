###
 # @file: 
 # @Author: regangcli
 # @copyright: Tencent Technology (Shenzhen) Company Limited
 # @Date: 2023-06-15 16:49:43
 # @edit: regangcli
 # @brief: 
### 
# set -x

PROJ_ROOT="$(cd "`dirname "$0"`" && pwd)"
PROTO_PATH="${PROJ_ROOT}/protobuf-3.20.0"
LLBC_PATH="${PROJ_ROOT}/llbc"
PROTO_SRC_PATH="${PROTO_PATH}/src"
PROTO_LIB_PATH="${PROTO_SRC_PATH}/lib"
PROTOC_PATH="${PROTO_SRC_PATH}/protoc"
LLBC_PATH="${PROJ_ROOT}/llbc"
RPC_PATH="${PROJ_ROOT}/rpc"
BUILD_PATH="./build/"


# build protobuf lib function
build_protobuf() {
    echo "Building protobuf"
    rm -fr ${BUILD_PATH}
    cd $PROTO_PATH
    autoreconf -f -i
    ./configure
    make -j15
    echo "Building protobuf done"
    cd -
}

# build llbc lib function
build_llbc() {
    # sudo yum install libuuid libuuid-devel
    echo "Building llbc"
    cd $LLBC_PATH
    make core_lib -j15
    echo "Building llbc done"
    cd -
}

# build rpc function
build_rpc() {
    echo "Building rpc"
    # gen pb
    (cd $RPC_PATH/pb && $PROTOC_PATH  *.proto --cpp_out=. )

    mkdir -p ${BUILD_PATH} && cd ${BUILD_PATH}
    cmake -DCMAKE_BUILD_TYPE=Debug .. && make VERBOSE=1 -j15
    echo "Building rpc done"
}

re_build_rpc() {
    echo "Building all rpc"
    rm -rf ${BUILD_PATH}
    rm -rf $RPC_PATH/pb/*.pb.*
    rm -rf $RPC_PATH/bin
    # build_protobuf
    # build_llbc
    build_rpc
}

# 根据参数调用相应的函数
if [[ ${1} == "proto" ]]; then 
    build_protobuf
    exit 0
elif [[ ${1} == "llbc" ]]; then 
    build_llbc
    exit 0
elif [[ ${1} == "rebuild" ]]; then 
    re_build_rpc
    exit 0
elif [[ ${1} == "all" ]]; then 
    build_protobuf
    build_llbc
    re_build_rpc
    exit 0
elif [[ ! -n "$1" ]]; then 
    build_rpc
    exit 0
else 
    echo "Usage: $0 |[proto|llbc|rebuild|all]"
    exit 1
fi

