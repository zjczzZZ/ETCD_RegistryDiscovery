all:service RpcServiceManager

service:service.cpp
	g++ service.cpp -o service -lpthread -letcd-cpp-api -lprotobuf  -lgrpc++ -lgrpc -lz -lcpprest -lssl -lcrypto -lboost_system

RpcServiceManager:RpcServiceManager.cpp
	g++ RpcServiceManager.cpp -o RpcServiceManager -lpthread -letcd-cpp-api -lprotobuf  -lgrpc++ -lgrpc -lz -lcpprest -lssl -lcrypto -lboost_system

clean:
	rm service RpcServiceManager