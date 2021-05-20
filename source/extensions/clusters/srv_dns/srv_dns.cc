#include "srv_dns.h"

#include <cstdint>

#include "envoy/common/exception.h"
#include "envoy/config/cluster/v3/cluster.pb.h"
#include "envoy/config/endpoint/v3/endpoint.pb.h"
#include "envoy/config/endpoint/v3/endpoint_components.pb.h"

#include "absl/container/node_hash_set.h"

namespace Envoy {
namespace Extensions {
namespace Clusters {
namespace SrvDns {

  // The cluster won't be fully initialized cluster manager calls startPreInit,
  // startPreInit calls startSrvResolve on the srv_resolve_targets_, at least
  // one finishes a SRV resolution, the callback creates regular resolve_targets_,
  // startResolve gets called on those, and at least one of them finishes and calls
  // onPreInitComplete to finish initializing the cluster.

  SrvDnsCluster::SrvDnsCluster(const envoy::config::cluster::v3::Cluster& cluster,
                const envoy::extensions::cluster::srv_dns::ClusterConfig& srv_dns_cluster,
                Runtime::Loader& runtime,
                Network::DnsResolverSharedPtr dns_resolver,
                Server::Configuration::TransportSocketFactoryContextImpl& factory_context,
                Stats::ScopePtr&& stats_scope, bool added_via_api)
                : Upstream::BaseStrictDnsClusterImpl(cluster, runtime, dns_resolver,
                    factory_context, std::move(stats_scope), added_via_api) {
  // Initialize srv_load_assignment_manager_.
  srv_load_assignment_manager_ = Upstream::SrvLoadAssignmentManager(
      srv_dns_cluster.srv_names, dns_resolver, factory_context.dispatcher(),
      [this](const absl::flat_hash_map<uint32_t,
        std::pair<envoy::config::endpoint::v3::LocalityLbEndpoints,
          std::vector<envoy::config::endpoint::v3::LbEndpoints>>>& add_lb_endpoints,
        const absl::flat_hash_map<uint32_t,
          std::pair<envoy::config::endpoint::v3::LocalityLbEndpoints,
          std::vector<envoy::config::endpoint::v3::LbEndpoints>>>& remove_lb_endpoints) -> void {
        // map of locality with vector to add  and map of locality with vector to remove


        // compare load assignments and only process differences? or just reload the load assignment?
        // how does a cds update work? that might help inform this

        // try this: create new flat hash set of resolve targets, iterate through the old resolve
        // targets and check if they are in the set, if not then add them to a vector to remove
        // also create a flat hash set of old resolve targets, iterate through new ones and check
        // if they are in the set, if not then add them to a vector to add and call startResolve
        // Finally remove the ones in the remove vector and add the ones from the add vector to
        // resolve_targets_


        // Should just be one pair of [priority, lb_endpoints] unless/until we implement localities
        // based on priority for srv.
        // Overall the complexity should be O(K x N) with number of endpoints to remove and
        // number of resolve_targets_.  Could possibly reduce it to O(K + N) if we used an
        // unordered_set of lb_endpoints or strings of hostnames.  An unordered set of
        // lb_endpoints would require specializing std::hash for it and writing a comparator
        // function. Endpoints and resolve targets are probably smallish in most cases.
        for (const auto& [pri, lb_endpoints] : remove_locality_lb_endpoints) {
          this->resolve_targets_.remove_if([&lb_endpoint](auto& curr) {
            for (const auto& lb_endpoint : lb_endpoints.second) {
              if (curr.dns_address_ ==
                    lb_endpoint.endpoint().address().socket_address().address()) {
                return true;
              }
            }
            return false;
          });
        }

        // Create new resolve_targets for the new lb_endpoints, startResolve, and add them to
        // resolve_targets_.
        for (const auto& [pri, lb_endpoints] : add_locality_lb_endpoints) {
          for (const auto& lb_endpoint : lb_endpoints.second) {
            const auto& socket_address = lb_endpoint.endpoint().address().socket_address();

            const std::string& url =
                fmt::format("tcp://{}:{}", socket_address.address(), socket_address.port_value());
            auto resolve_target = std::make_unique<ResolveTargetPtr>(*this, dispatcher,
                                                            url, lb_endpoints.first, lb_endpoint);
            resolve_target->startResolve();
            this->resolve_targets_.push_back(std::move(resolve_target));
          }
        }
      });

  if (cluster.has_load_assignment()) {
    throw EnvoyException("SRV_DNS clusters must have no load assignment.");
  }
}

void SrvDnsCluster::startPreInit() {
  for (const SrvResolveTargetPtr& target : srv_load_assignment_manager.srv_resolve_targets_) {
    target->startSrvResolve();
  }
  // If the config provides no endpoints, the cluster is initialized immediately as if all hosts are
  // resolved in failure.
  if (srv_resolve_targets_.empty()) {
    onPreInitComplete();
  }
}


} // namespace SrvDns
} // namespace Clusters
} // namespace Extensions
} // namespace Envoy