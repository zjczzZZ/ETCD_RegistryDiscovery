#include "RpcServiceManager.h"

#include <thread>

static const std::string etcd_url = etcdv3::detail::resolve_etcd_endpoints("http://127.0.0.1:2379");

RpcServiceManager::~RpcServiceManager() {
    if (_watcher) {
        delete _watcher;
        _watcher = nullptr;
    }
    std::cout << "end" << std::endl;
}

void RpcServiceManager::Watch() {
    std::function<void(etcd::Response)> callback = std::bind(&RpcServiceManager::WatchHandler, this, std::placeholders::_1);
    _watcher = new etcd::Watcher(_etcdUrl, _dir, callback, true);
}

std::vector<std::string> RpcServiceManager::Responce(string service) {
    auto iter = _serviceAddress.find(service);
    if (iter != _serviceAddress.end()) {
        std::vector<string> res(iter->second.begin(), iter->second.end());
        return res;
    }
    std::vector<string> res;
    return res;
}

void RpcServiceManager::GetAllServiceAddress(string dir) {
    etcd::Client initNode(_etcdUrl);

    etcd::Response Resp = initNode.ls(dir).get();

    if (!Resp.is_ok()) {
        std::cout << " init list err " << Resp.error_code() << " " << Resp.error_message() << std::endl;
    }

    for (int i = 0; i < Resp.keys().size(); ++i) {
        string respkey = Resp.key(i);
        string respvalue = Resp.value(i).as_string();

        string serviceName = GetServiceFromKey(respkey);

        if (serviceName == "") {
            continue;
        }
        auto iter = _serviceAddress.find(serviceName);
        if (iter != _serviceAddress.end()) {
            if (iter->second.find(respvalue) == iter->second.end()) {
                iter->second.insert(respvalue);
            }
        } else {
            std::unordered_set<string> tmp;
            tmp.insert(respvalue);
            _serviceAddress.emplace(serviceName, tmp);
        }
    }
    PrintAllService();
}

void RpcServiceManager::WatchHandler(etcd::Response const &resp) {
    if (resp.error_code()) {
        std::cout << resp.error_code() << ": " << resp.error_message() << std::endl;
    } else {
        std::cout << resp.action() << " ... " << resp.value().as_string() << " Events size: " << resp.events().size() << std::endl;

        int evType = 0;

        string serviceStr = "";
        string addressStr = "";

        for (auto const &ev : resp.events()) {
            if (ev.prev_kv().key().find("/leader") == 0 || ev.kv().key().find("/leader") == 0) {
                return;
            }

            // 0创建 1删除
            evType = static_cast<int>(ev.event_type());

            if (evType == 0) {
                serviceStr = GetServiceFromKey(ev.kv().key());
                addressStr = ev.kv().as_string();
                ServiceAdd(serviceStr, addressStr);
            } else if (evType == 1) {
                serviceStr = GetServiceFromKey(ev.kv().key());
                addressStr = ev.prev_kv().as_string();
                ServiceDel(serviceStr, addressStr);
            }
            PrintAllService();
        }
    }
    // std::cout << "response called =================\n";
    std::cout << "\n";
}

void RpcServiceManager::ServiceAdd(string sername, string address) {
    auto iter = _serviceAddress.find(sername);
    if (iter != _serviceAddress.end()) {
        if (iter->second.find(address) == iter->second.end()) {
            iter->second.insert(address);
        }
    } else {
        std::unordered_set<string> tmp;
        tmp.insert(address);
        _serviceAddress.emplace(sername, tmp);
    }
}

void RpcServiceManager::ServiceDel(string sername, string address) {
    auto iter = _serviceAddress.find(sername);
    if (iter != _serviceAddress.end()) {
        if (iter->second.find(address) != iter->second.end()) {
            iter->second.erase(address);
        }
        if (iter->second.size() == 0) {
            _serviceAddress.erase(iter);
        }
    }
}

void RpcServiceManager::PrintAllService() {
    if (_serviceAddress.size() == 0) {
        std::cout << "key: null "
                  << " server num: 0 "
                  << " value -> null\n";
    } else {
        for (auto &val : _serviceAddress) {
            std::cout << "key: " << val.first << "  server num: " << val.second.size() << " value -> ";
            for (auto &addr : val.second) {
                std::cout << addr << "  ";
            }
            std::cout << "\n";
        }
    }
}

string RpcServiceManager::GetServiceFromKey(string str) {
    str = trim(str, '/');
    std::vector<string> strlist = splitStr(str, "/");
    if (strlist[0] == "service" && strlist.size() >= 2) {
        return strlist[1];
    }
    return "";
}

std::vector<string> RpcServiceManager::splitStr(string s, string del) {
    std::vector<string> res;
    int end = s.find(del);
    while (end != -1) {
        res.emplace_back(s.substr(0, end));
        s.erase(s.begin(), s.begin() + end + 1);
        end = s.find(del);
    }
    res.emplace_back(s.substr(0, end));
    return res;
}

string &RpcServiceManager::trim(string &s, char t) {
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(t));
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

int main() {
    RpcServiceManager Srpc(etcd_url, "/service");
    Srpc.Watch();

    std::this_thread::sleep_for(std::chrono::seconds(30));
}

// int main() {
//     etcd::Client initNode(etcd_url);

//     etcd::Response Resp = initNode.ls("/service").get();
//     std::cout << " init resq " << std::endl;

//     if (!Resp.is_ok()) {
//         std::cout << " init list err " << Resp.error_code() << " " << Resp.error_message() << std::endl;
//     }

//     std::cout << " init ok " << Resp.keys().size() << std::endl;

//     for (int i = 0; i < Resp.keys().size(); ++i) {
//         std::cout << " kv: " << Resp.key(i) << " -> " << Resp.value(i).as_string() << "\n";
//     }

//     etcd::Watcher watcher(etcd_url, "/service", printResponse, true);
//     while (true) {
//         std::this_thread::sleep_for(std::chrono::seconds(3));
//     }
// }
