#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "envoy/common/pure.h"
#include "envoy/network/address.h"

namespace Envoy {
namespace Network {

/**
 * An active async DNS query.
 */
class ActiveDnsQuery {
public:
  virtual ~ActiveDnsQuery() = default;

  /**
   * Cancel an outstanding DNS request.
   */
  virtual void cancel() PURE;
};

/**
 * DNS response.
 */
struct DnsResponse {
  DnsResponse(const Address::InstanceConstSharedPtr& address, const std::chrono::seconds ttl)
      : address_(address), ttl_(ttl) {}

  const Address::InstanceConstSharedPtr address_;
  const std::chrono::seconds ttl_;
};

/**
 * SRV DNS response.
 */
struct SrvDnsResponse {
  SrvDnsResponse(const std::string target&, const uint32_t port, const std::chrono::seconds ttl, const uint32_t weight)
      : address_(address), port_(port), ttl_(ttl), weight_(weight) {}

  const std::string target_;
  const uint32_t port_;
  const std::chrono::seconds ttl_;
  const uint32_t weight_;
  // If there is interest in the future, SRV record priority will trigger cluster locality awareness and the priority
  // will map to locality priority.  Each SRV target would map to a different locality.
  // const uint32_t priority_
};

enum class DnsLookupFamily { V4Only, V6Only, Auto };

/**
 * An asynchronous DNS resolver.
 */
class DnsResolver {
public:
  virtual ~DnsResolver() = default;

  /**
   * Final status for a DNS resolution.
   */
  enum class ResolutionStatus { Success, Failure };

  /**
   * Called when a resolution attempt is complete.
   * @param status supplies the final status of the resolution.
   * @param response supplies the list of resolved IP addresses and TTLs.
   */
  using ResolveCb = std::function<void(ResolutionStatus status, std::list<DnsResponse>&& response)>;

  /**
   * Initiate an async DNS resolution.
   * @param dns_name supplies the DNS name to lookup.
   * @param dns_lookup_family the DNS IP version lookup policy.
   * @param callback supplies the callback to invoke when the resolution is complete.
   * @return if non-null, a handle that can be used to cancel the resolution.
   *         This is only valid until the invocation of callback or ~DnsResolver().
   */
  virtual ActiveDnsQuery* resolve(const std::string& dns_name, DnsLookupFamily dns_lookup_family,
                                  ResolveCb callback) PURE;

  /**
   * Called when a resolution attempt for an SRV record is complete.
   * @param srv_records supplies the list of resolved SRV records. The list will be empty if the
   *        resolution failed.
   */
  typedef std::function<void(const std::list<Address::SrvInstanceConstSharedPtr>&& srv_records)>
      ResolveSrvCb;

  /**
   * Initiate an async DNS resolution for an SRV record.
   * @param dns_name supplies the DNS name to lookup.
   * @param dns_lookup_family the DNS IP version lookup policy.
   * @param callback supplies the callback to invoke when the resolution is complete.
   * @return if non-null, a handle that can be used to cancel the resolution.
   *         This is only valid until the invocation of callback or ~DnsResolver().
   */
  virtual ActiveDnsQuery* resolveSrv(const std::string& dns_name, DnsLookupFamily dns_lookup_family,
                                     ResolveSrvCb callback) PURE;
};

using DnsResolverSharedPtr = std::shared_ptr<DnsResolver>;

} // namespace Network
} // namespace Envoy
