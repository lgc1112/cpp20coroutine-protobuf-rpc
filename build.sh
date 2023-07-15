###
 # @file: 
 # @Author: regangcli
 # @copyright: Tencent Technology (Shenzhen) Company Limited
 # @Date: 2023-06-15 16:49:43
 # @edit: regangcli
 # @brief: 
### 
set -x

PROJ_ROOT="$(cd "`dirname "$0"`" && pwd)"
PROTO_PATH="${PROJ_ROOT}/protobuf-3.20.0"
LLBC_PATH="${PROJ_ROOT}/llbc"
PROTO_SRC_PATH="${PROTO_PATH}/src"
PROTO_LIB_PATH="${PROTO_SRC_PATH}/lib"
PROTOC_PATH="${PROTO_SRC_PATH}/protoc"
LLBC_PATH="${PROJ_ROOT}/llbc"
RPC_PATH="${PROJ_ROOT}/rpc"
BUILD_PATH="./build/"

# build protobuf lib
cd $PROTO_PATH
autoreconf -f -i
./configure
make -j12
cd -



# # build llbc lib
# # cd $LLBC_PATH
# # make coro_lib
# # cd -

# # build rpc
# target=${1-"use_cache"}
# if [[ "$target" == "all" ]]; then 
#     echo "build all"
#     rm -fr ${BUILD_PATH}
# else
#     echo "build"
# fi
# export LD_LIBRARY_PATH="${PROTO_LIB_PATH}:${LD_LIBRARY_PATH}"
# (cd $RPC_PATH/pb && $PROTOC_PATH  *.proto --cpp_out=. )
# # rm -fr ${build_dir}
# mkdir -p ${BUILD_PATH} && cd ${BUILD_PATH}

# cmake -DCMAKE_BUILD_TYPE=Debug .. && make VERBOSE=1 -j12

# # pkill server
# # ./server
