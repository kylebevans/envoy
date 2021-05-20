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
  SrvDnsCluster(const envoy::config::cluster::v3::Cluster& cluster,
                const envoy::extensions::cluster::srv_dns::ClusterConfig& srv_dns_cluster,
                Runtime::Loader& runtime,
                Network::DnsResolverSharedPtr dns_resolver,
                Server::Configuration::TransportSocketFactoryContextImpl& factory_context,
                Stats::ScopePtr&& stats_scope, bool added_via_api);

protected:
  void startPreInit() override;

  Upstream::SrvLoadAssignmentManager srv_load_assignment_manager_;
};

/**
 * Factory for SrvDnsCluster
 */
class SrvDnsClusterFactory : public Upstream::ConfigurableClusterFactoryBase<
                                envoy::extensions::cluster::srv_dns::ClusterConfig> {
public:
  SrvDnsClusterFactory()
      : ConfigurableClusterFactoryBase(Extensions::Clusters::ClusterTypes::get().SrvDns) {}

private:
  friend class SrvDnsClusterTest;

  std::pair<Upstream::ClusterImplBaseSharedPtr, Upstream::ThreadAwareLoadBalancerPtr>
  createClusterWithConfig(
      const envoy::config::cluster::v3::Cluster& cluster,
      const envoy::extensions::cluster::srv_dns::ClusterConfig& proto_config,
      Upstream::ClusterFactoryContext& context,
      Server::Configuration::TransportSocketFactoryContextImpl& socket_factory_context,
      Stats::ScopePtr&& stats_scope) override;
};

} // namespace SrvDns
} // namespace Clusters
} // namespace Extensions
} // namespace Envoy