#ifndef _ROUTING_TABLE_H
#define _ROUTING_TABLE_H

#include "ospf_packets.h"
#include "shared.h"
#include <netinet/in.h>

/* 11. The Routing Table Structure */
/* The routing table data structure contains all the information
   necessary to forward an IP data packet toward its destination. Each
   routing table entry describes the collection of best paths to a
   particular destination. When forwarding an IP data packet, the
   routing table entry providing the best match for the packet’s IP
   destination is located. The matching routing table entry then
   provides the next hop towards the packet’s destination. OSPF also
   provides for the existence of a default route (Destination ID =
   DefaultDestination, Address Mask = 0x00000000). When the default
   route exists, it matches all IP destinations (although any other
   matching entry is a better match). Finding the routing table entry
   that best matches an IP destination is further described in Section
   11.1.

   There is a single routing table in each router. Two sample routing
   tables are described in Sections 11.2 and 11.3. The building of the
   routing table is discussed in Section 16.

   The rest of this section defines the fields found in a routing table
   entry. The first set of fields describes the routing table entry’s
   destination. */

typedef struct route{
   /* Destination Type - 
      Destination type is either "network" or "router". Only network
      entries are actually used when forwarding IP data traffic.
      Router routing table entries are used solely as intermediate
      steps in the routing table build process.
      A network is a range of IP addresses, to which IP data traffic
      may be forwarded. This includes IP networks (class A, B, or C),
      IP subnets, IP supernets and single IP hosts. The default route
      also falls into this category.
      Router entries are kept for area border routers and AS boundary
      routers. Routing table entries for area border routers are used
      when calculating the inter-area routes (see Section 16.2), and
      when maintaining configured virtual links (see Section 15).
      Routing table entries for AS boundary routers are used when
      calculating the AS external routes (see Section 16.4). */
   int dest_type;

   /* Destination ID - 
      The destination’s identifier or name. This depends on the
      Destination Type. For networks, the identifier is their
      associated IP address. For routers, the identifier is the OSPF
      Router ID.[9] */
	in_addr_t dest_id;

   /* Address Mask - 
      Only defined for networks. The network’s IP address together
      with its address mask defines a range of IP addresses. For IP
      subnets, the address mask is referred to as the subnet mask.
      For host routes, the mask is "all ones" (0xffffffff). */
	in_addr_t addr_mask;

   /* Optional Capabilities - 
      When the destination is a router this field indicates the
      optional OSPF capabilities supported by the destination router.
      The only optional capability defined by this specification is
      the ability to process AS-external-LSAs. For a further
      discussion of OSPF’s optional capabilities, see Section 4.5. */
   uint8_t options;

   /* The set of paths to use for a destination may vary based on the OSPF
      area to which the paths belong. This means that there may be
      multiple routing table entries for the same destination, depending
      on the values of the next field. */

   /* Area - 
      This field indicates the area whose link state information has
      led to the routing table entry’s collection of paths. This is
      called the entry’s associated area. For sets of AS external
      paths, this field is not defined. For destinations of type
      "router", there may be separate sets of paths (and therefore
      separate routing table entries) associated with each of several
      areas. For example, this will happen when two area border
      routers share multiple areas in common. For destinations of
      type "network", only the set of paths associated with the best
      area (the one providing the preferred route) is kept. */
   uint32_t area_id;

   /* The rest of the routing table entry describes the set of paths to
      the destination. The following fields pertain to the set of paths
      as a whole. In other words, each one of the paths contained in a
      routing table entry is of the same path-type and cost (see below). */

   /* Path-type - 
      There are four possible types of paths used to route traffic to
      the destination, listed here in decreasing order of preference:
      intra-area, inter-area, type 1 external or type 2 external.
      Intra-area paths indicate destinations belonging to one of the
      router’s attached areas. Inter-area paths are paths to
      destinations in other OSPF areas. These are discovered through
      the examination of received summary-LSAs. AS external paths are
      paths to destinations external to the AS. These are detected
      through the examination of received AS-external-LSAs. */
   int path_type;

   /* Cost - 
      The link state cost of the path to the destination. For all
      paths except type 2 external paths this describes the entire
      path’s cost. For Type 2 external paths, this field describes
      the cost of the portion of the path internal to the AS. This
      cost is calculated as the sum of the costs of the path’s
      constituent links. */

   /* Type 2 cost - 
      Only valid for type 2 external paths. For these paths, this
      field indicates the cost of the path’s external portion. This
      cost has been advertised by an AS boundary router, and is the
      most significant part of the total path cost. For example, a
      type 2 external path with type 2 cost of 5 is always preferred
      over a path with type 2 cost of 10, regardless of the cost of
      the two paths’ internal components. */
   uint16_t cost;

   /* Link State Origin - 
      Valid only for intra-area paths, this field indicates the LSA
      (router-LSA or network-LSA) that directly references the
      destination. For example, if the destination is a transit
      network, this is the transit network’s network-LSA. If the
      destination is a stub network, this is the router-LSA for the
      attached router. The LSA is discovered during the shortest-path
      tree calculation (see Section 16.1). Multiple LSAs may
      reference the destination, however a tie-breaking scheme always
      reduces the choice to a single LSA. The Link State Origin field
      is not used by the OSPF protocol, but it is used by the routing
      table calculation in OSPF’s Multicast routing extensions
      (MOSPF). */
   const ospf_lsa_header *lsa;

   /* When multiple paths of equal path-type and cost exist to a
      destination (called elsewhere "equal-cost" paths), they are stored
      in a single routing table entry. Each one of the "equal-cost" paths
      is distinguished by the following fields: */

   /* Next hop - 
      The outgoing router interface to use when forwarding traffic to
      the destination. On broadcast, Point-to-MultiPoint and NBMA
      networks, the next hop also includes the IP address of the next
      router (if any) in the path towards the destination. */
	in_addr_t next_hop;

   /* Advertising router - 
      Valid only for inter-area and AS external paths. This field
      indicates the Router ID of the router advertising the summary-
      LSA or AS-external-LSA that led to this path. */
   uint32_t adv_router;

	const char *iface;
}route;


/* 11.1. Routing table lookup */
/* When an IP data packet is received, an OSPF router finds the
   routing table entry that best matches the packet’s destination.
   This routing table entry then provides the outgoing interface
   and next hop router to use in forwarding the packet. This
   section describes the process of finding the best matching
   routing table entry.

   Before the lookup begins, "discard" routing table entries should
   be inserted into the routing table for each of the router’s
   active area address ranges (see Section 3.5). (An area range is
   considered "active" if the range contains one or more networks
   reachable by intra-area paths.) The destination of a "discard"
   entry is the set of addresses described by its associated active
   area address range, and the path type of each "discard" entry is
   set to "inter-area".[10]

   Several routing table entries may match the destination address.
   In this case, the "best match" is the routing table entry that
   provides the most specific (longest) match. Another way of
   saying this is to choose the entry that specifies the narrowest
   range of IP addresses.[11] For example, the entry for the
   address/mask pair of (128.185.1.0, 0xffffff00) is more specific
   than an entry for the pair (128.185.0.0, 0xffff0000). The
   default route is the least specific match, since it matches all
   destinations. (Note that for any single routing table entry,
   multiple paths may be possible. In these cases, the calculations
   in Sections 16.1, 16.2, and 16.4 always yield the paths having
   the most preferential path-type, as described in Section 11).

   If there is no matching routing table entry, or the best match
   routing table entry is one of the above "discard" routing table
   entries, then the packet’s IP destination is considered
   unreachable. Instead of being forwarded, the packet should then
   be discarded and an ICMP destination unreachable message should
   be returned to the packet’s source. */

void invalidated_old_routing_table();
int lookup_route_by_dst(in_addr_t dest_id);
int lookup_route_by_least_cost();
void update_routing_table();

#endif
