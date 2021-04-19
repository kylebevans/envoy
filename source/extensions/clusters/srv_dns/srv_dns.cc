#include "srv_dns.h"

#include "envoy/common/exception.h"
#include "envoy/config/cluster/v3/cluster.pb.h"
#include "envoy/config/endpoint/v3/endpoint.pb.h"
#include "envoy/config/endpoint/v3/endpoint_components.pb.h"

namespace Envoy {
namespace Extensions {
namespace Clusters {
namespace SrvDns {

SrvDnsCluster::SrvDnsCluster(const envoy::config::cluster::v3::Cluster& cluster, Runtime::Loader& runtime,
                Network::DnsResolverSharedPtr dns_resolver,
                Server::Configuration::TransportSocketFactoryContextImpl& factory_context,
                Stats::ScopePtr&& stats_scope, bool added_via_api)
                : Upstream::BaseStrictDnsClusterImpl(cluster, runtime, dns_resolver, factory_context,
                                                     std::move(stats_scope), added_via_api) {
  createLoadAssignmentFromSrv();
  resolve_timer_(dispatcher.createTimer([this]() -> void { startResolve(); }))
}

void SrvDnsCluster::createLoadAssignmentFromSrv() {

}

} // namespace SrvDns
} // namespace Clusters
} // namespace Extensions
} // namespace Envoy