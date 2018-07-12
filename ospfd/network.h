#ifndef _NETWORK_H
#define _NETWORK_H

#include "shared.h"

/* 7.2. The Synchronization of Databases */
/* In a link-state routing algorithm, it is very important for all
   routers’ link-state databases to stay synchronized. OSPF
   simplifies this by requiring only adjacent routers to remain
   synchronized. The synchronization process begins as soon as the
   routers attempt to bring up the adjacency. Each router
   describes its database by sending a sequence of Database
   Description packets to its neighbor. Each Database Description
   Packet describes a set of LSAs belonging to the router’s
   database. When the neighbor sees an LSA that is more recent
   than its own database copy, it makes a note that this newer LSA
   should be requested.

   This sending and receiving of Database Description packets is
   called the "Database Exchange Process". During this process,
   the two routers form a master/slave relationship. Each Database
   Description Packet has a sequence number. Database Description
   Packets sent by the master (polls) are acknowledged by the slave
   through echoing of the sequence number. Both polls and their
   responses contain summaries of link state data. The master is
   the only one allowed to retransmit Database Description Packets.
   It does so only at fixed intervals, the length of which is the
   configured per-interface constant RxmtInterval.

   Each Database Description contains an indication that there are
   more packets to follow --- the M-bit. The Database Exchange
   Process is over when a router has received and sent Database
   Description Packets with the M-bit off.

   During and after the Database Exchange Process, each router has
   a list of those LSAs for which the neighbor has more up-to-date
   instances. These LSAs are requested in Link State Request
   Packets. Link State Request packets that are not satisfied are
   retransmitted at fixed intervals of time RxmtInterval. When the
   Database Description Process has completed and all Link State
   Requests have been satisfied, the databases are deemed
   synchronized and the routers are marked fully adjacent. At this
   time the adjacency is fully functional and is advertised in the
   two routers’ router-LSAs.

   The adjacency is used by the flooding procedure as soon as the
   Database Exchange Process begins. This simplifies database
   synchronization, and guarantees that it finishes in a
   predictable period of time. */

interface_data *recv_ospf(int sock, uint8_t buf[], int size, in_addr_t *src);
void send_ospf(const interface_data *iface, struct iphdr *ip_hdr, in_addr_t dst);
void network_init();
void *recv_and_process();
void *encapsulate_and_send();


#endif
