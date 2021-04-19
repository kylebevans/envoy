#pragma once

#include "envoy/config/cluster/v3/cluster.pb.h"
#include "envoy/config/cluster/srv_dns/cluster.pb.h"
#include "envoy/config/cluster/srv_dns/cluster.pb.validate.h"
#include "envoy/config/cluster/v3/cluster.pb.h"

#include "common/upstream/cluster_factory_impl.h"
#include "common/upstream/upstream_impl.h"

namespace Envoy {
namespace Extensions {
namespace Clusters {
namespace SrvDns {

class SrvDnsCluster : public Upstream::BaseStrictDnsClusterImpl {
public:
  SrvDnsCluster(const envoy::config::cluster::v3::Cluster& cluster, Runtime::Loader& runtime,
                Network::DnsResolverSharedPtr dns_resolver,
                Server::Configuration::TransportSocketFactoryContextImpl& factory_context,
                Stats::ScopePtr&& stats_scope, bool added_via_api);

  void createLoadAssignmentFromSrv();
};

/**
 * Factory for SrvDnsCluster
 */
class SrvDnsClusterFactory : public ClusterFactoryImplBase {
  std::pair<ClusterImplBaseSharedPtr, ThreadAwareLoadBalancerPtr> createClusterImpl(
      const envoy::config::cluster::v3::Cluster& cluster, ClusterFactoryContext& context,
      Server::Configuration::TransportSocketFactoryContextImpl& socket_factory_context,
      Stats::ScopePtr&& stats_scope) override;
};

} // namespace SrvDns
} // namespace Clusters
} // namespace Extensions
} // namespace Envoy