#include <chrono>
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Response.hpp>
#include <etcd/SyncClient.hpp>
#include <etcd/Value.hpp>
#include <etcd/Watcher.hpp>
#include <thread>

static const std::string etcd_url = etcdv3::detail::resolve_etcd_endpoints("http://127.0.0.1:2379");

int main() {
    etcd::Client etcd(etcd_url);
    etcd.rmdir("/service", true);

    auto keep = etcd.leasekeepalive(10).get();
    int64_t leaseID = keep->Lease();
    etcd.set("/service/interface/8888", "http://127.0.0.1:8888", leaseID);
    std::cout << "ttl: " << etcd.leasetimetolive(leaseID).get().value().ttl() << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "ttl: " << etcd.leasetimetolive(leaseID).get().value().ttl() << "\n";

    {
        etcd::KeepAlive kal(etcd, 8, leaseID);
    }

    std::cout << "ttl: " << etcd.leasetimetolive(leaseID).get().value().ttl() << "\n";

    etcd.set("/service/interface/1111", "http://127.0.0.1:1111");
    std::this_thread::sleep_for(std::chrono::seconds(5));

    {
        etcd::KeepAlive kal(etcd, 8, leaseID);
    }
    std::cout << "ttl: " << etcd.leasetimetolive(leaseID).get().value().ttl() << "\n";

    etcd.set("/service/5555", "http://127.0.0.1:5555");
    std::this_thread::sleep_for(std::chrono::seconds(5));

    etcd.rmdir("/service", true);
}