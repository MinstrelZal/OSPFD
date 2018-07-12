#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "neighbor.h"
#include "area.h"
#include "shared.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
/*
typedef struct neighbor neighbor;
typedef struct area area;
*/
struct neighbor;
// typedef struct neighbor neighbor;
typedef struct interface_data{

	/* The name of the interface, e.g. eth0, ... */
	char if_name[IF_NAMESIZE];

	/* type - the OSPF interface type is either
       point-to-point, broadcast, NBMA, Point-to-MultiPoint
	   or virtual link. */
	int type;

	/* State - The functional level of an interface. State
	   determines whether or not full adjacencies are allowed
	   to form over the interface. State is also reflected in
	   the router's LSAs. */
	int state;

	/* IP interface address & IP interface mask:
	   The IP address associated with the interface.
       This appears as the IP source address in all routing
       protocol packets originated over this interface.
       Interfaces to unnumbered point-to-point networks do not
       have an associated IP address.
       Also referred to as the subnet mask, this indicates the portion
       of the IP interface address that identifies the attached network.
       Masking the IP interface address with the IP interface
       mask yields the IP network number of the attached network.  On
       point-to-point networks and virtual links, the IP interface mask
       is not defined. On these networks, the link itself is not
       assigned an IP network number, and so the addresses of each side
       of the link are assigned independently, if they are assigned at
       all. */
	in_addr_t ip;
	in_addr_t network_mask;

	/* The Area ID of the area to which the attached network
       belongs. All routing protocol packets originating from
       the interface are labelled with this Area ID. */
	uint32_t area_id;

	/* HelloInterval - The length of time, in seconds, between
       the Hello packets that the router sends on the interface.
       Advertised in Hello packets sent out this interface. */
	int hello_interval;

	/* The number of seconds before the router's neighbors will declare
       it down, when they stop hearing the router's Hello Packets.
       Advertised in Hello packets sent out this interface. */
	int router_dead_interval;

	/* The estimated number of seconds it takes to transmit a Link
       State Update Packet over this interface.  LSAs contained in the
       Link State Update packet will have their age incremented by this
       amount before transmission.  This value should take into account
       transmission and propagation delays; it must be greater than
       zero. */
	uint16_t inf_trans_delay;

	/* Router Priority - An 8-bit unsigned integer.  When two routers
	   attached to a network both attempt to become Designated Router,
	   the one with the highest Router Priority takes precedence.
	   A router whose Router Priority is set to 0 is ineligible to become
	   Designated Router on the attached network. Advertised in Hello
	   packets sent out this interface. */
	uint8_t router_priority;

	/* Hello Timer - An interval timer that causes the interface to send a Hello
	   packet. This timer fires every HelloInterval seconds. Note
	   that on non-broadcast networks a separate Hello packet is sent
	   to each qualified neighbor. */
	int hello_timer;

	/* Wait Timer - A single shot timer that causes the interface to exit the
	   Waiting state, and as a consequence select a Designated Router
	   on the network. The length of the timer is RouterDeadInterval
	   seconds. */
	int wait_timer;

	/* List of neighboring routers -- The other routers attached to
	   this network. This list is formed by the Hello Protocol. Adjacencies
	   will be formed to some of these neighbors. The set of adjacent neighbors
	   can be determined by an examination of all of the neighbors' states. */
	int num_neighbor;
	struct neighbor *neighbors;

	/* The Designated Router selected for the attached network. The
	   Designated Router is selected on all broadcast and NBMA networks
	   by the Hello Protocol. Two pieces of identification are kept
	   for the Designated Router: its Router ID and its IP interface
	   address on the network. The Designated Router advertises link
	   state for the network; this network-LSA is labelled with the
	   Designated Router’s IP address. The Designated Router is
	   initialized to 0.0.0.0, which indicates the lack of a Designated
	   Router. */
	in_addr_t d_router;
	in_addr_t bd_router;

	/* The cost of sending a data packet on the interface,
           expressed in the link state metric. This is advertised
           as the link cost for this interface in the router-LSA.
           The cost of an interface must be greater than zero. */
	uint32_t cost;

	/* RxmtInterval */
	/* The number of seconds between LSA retransmissions, for
	   adjacencies belonging to this interface. Also used when
           retransmitting Database Description and Link State Request
           Packets. */
	int rxmt_interval;
        int rxmt_timer;

	/* AuType */
	/* The type of authentication used on the attached network/subnet.
       Authentication types are defined in Appendix D. All OSPF packet
       exchanges are authenticated. Different authentication schemes
       may be used on different networks/subnets. */

	/* Authentication key */
	/* This configured data allows the authentication procedure to
	   generate and/or verify OSPF protocol packets. The
	   Authentication key can be configured on a per-interface basis.
	   For example, if the AuType indicates simple password, the
	   Authentication key would be a 64-bit clear password which is
	   inserted into the OSPF packet header. If instead Autype
	   indicates Cryptographic authentication, then the Authentication
	   key is a shared secret which enables the generation/verification
	   of message digests which are appended to the OSPF protocol
	   packets. When Cryptographic authentication is used, multiple
	   simultaneous keys are supported in order to achieve smooth key
	   transition (see Section D.3). */

	int sock;
}interface_data;


int interface_init();
void set_interface_area(interface_data *iface, uint32_t area_id);
const char *lookup_ifname_by_ip(const struct area *a, in_addr_t ip);

#endif
