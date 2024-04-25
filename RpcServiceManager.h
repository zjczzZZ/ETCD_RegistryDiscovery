#include <chrono>
#include <etcd/Client.hpp>
#include <etcd/Response.hpp>
#include <etcd/SyncClient.hpp>
#include <etcd/Value.hpp>
#include <etcd/Watcher.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using std::string;

class RpcServiceManager {
    // using GetAllCB = std::function<void(etcd::Response const&)>;

private:
    string _etcdUrl;
    std::mutex _mtx;
    etcd::Watcher* _watcher;
    string _dir;
    std::unordered_map<string, std::unordered_set<string>> _serviceAddress;

public:
    RpcServiceManager(string url, string dir)
        : _etcdUrl(url),
          _dir(dir) {
        _serviceAddress.clear();
        GetAllServiceAddress(_dir);
    }
    ~RpcServiceManager();

    void Watch();

    std::vector<string> Responce(string);

private:
    RpcServiceManager() = delete;

    // 初始化 获取dir下所有的服务与地址
    void GetAllServiceAddress(string dir);

    // watch 某路径下 resp的处理函数
    void WatchHandler(etcd::Response const& resp);

    // 向数据结构中添加  服务->地址的kv
    void ServiceAdd(string sername, string address);

    // 在数据结构中删除 服务->地址的kv
    void ServiceDel(string sername, string address);

    // 打印所有服务及地址
    void PrintAllService();

    // 从resp的key中获取service名
    static string GetServiceFromKey(string str);

    // 工具 用del拆分s
    static std::vector<string> splitStr(string s, string del);

    // 工具 s字符串去掉首尾的t
    static string& trim(string& s, char t);
};