protoc -I. --cpp_out=./ service.proto
protoc -I. --grpc_out=./ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` service.proto
