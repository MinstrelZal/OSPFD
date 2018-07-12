#ifndef _LSR_H
#define _LSR_H

#include "ospf_packets.h"
#include "neighbor.h"
#include "shared.h"

/* 10.7. Receiving Link State Request Packets */
/* This section explains the detailed processing of received Link
   State Request packets. Received Link State Request Packets
   specify a list of LSAs that the neighbor wishes to receive.
   Link State Request Packets should be accepted when the neighbor
   is in states Exchange, Loading, or Full. In all other states
   Link State Request Packets should be ignored.

   Each LSA specified in the Link State Request packet should be
   located in the router’s database, and copied into Link State
   Update packets for transmission to the neighbor. These LSAs
   should NOT be placed on the Link state retransmission list for
   the neighbor. If an LSA cannot be found in the database,
   something has gone wrong with the Database Exchange process, and
   neighbor event BadLSReq should be generated. */


/* 10.9. Sending Link State Request Packets */
/* In neighbor states Exchange or Loading, the Link state request
   list contains a list of those LSAs that need to be obtained from
   the neighbor. To request these LSAs, a router sends the
   neighbor the beginning of the Link state request list, packaged
   in a Link State Request packet.

   When the neighbor responds to these requests with the proper
   Link State Update packet(s), the Link state request list is
   truncated and a new Link State Request packet is sent. This
   process continues until the Link state request list becomes
   empty. LSAs on the Link state request list that have been
   requested, but not yet received, are packaged into Link State
   Request packets for retransmission at intervals of RxmtInterval.
   There should be at most one Link State Request packet
   outstanding at any one time.

   When the Link state request list becomes empty, and the neighbor
   state is Loading (i.e., a complete sequence of Database
   Description packets has been sent to and received from the
   neighbor), the Loading Done neighbor event is generated. */

void process_lsr_pkt(interface_data *iface, neighbor *nbr, ospf_header *ospf_hdr);
void encapsulate_lsr_pkt(const neighbor *nbr, ospf_header *ospf_hdr);

#endif            
