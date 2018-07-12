#ifndef _LSU_H
#define _LSU_H

#include "area.h"
#include "shared.h"

/* 13. The Flooding Procedure */
/* Link State Update packets provide the mechanism for flooding LSAs.
   A Link State Update packet may contain several distinct LSAs, and
   floods each LSA one hop further from its point of origination. To
   make the flooding procedure reliable, each LSA must be acknowledged
   separately. Acknowledgments are transmitted in Link State
   Acknowledgment packets. Many separate acknowledgments can also be
   grouped together into a single packet.

   The flooding procedure starts when a Link State Update packet has
   been received. Many consistency checks have been made on the
   received packet before being handed to the flooding procedure (see
   Section 8.2). In particular, the Link State Update packet has been
   associated with a particular neighbor, and a particular area. If
   the neighbor is in a lesser state than Exchange, the packet should
   be dropped without further processing.

   All types of LSAs, other than AS-external-LSAs, are associated with
   a specific area. However, LSAs do not contain an area field. An
   LSA’s area must be deduced from the Link State Update packet header.

   For each LSA contained in a Link State Update packet, the following
   steps are taken:

   (1) Validate the LSA’s LS checksum. If the checksum turns out to be
   invalid, discard the LSA and get the next one from the Link
   State Update packet.

   (2) Examine the LSA’s LS type. If the LS type is unknown, discard
   the LSA and get the next one from the Link State Update Packet.
   This specification defines LS types 1-5 (see Section 4.3).

   (3) Else if this is an AS-external-LSA (LS type = 5), and the area
   has been configured as a stub area, discard the LSA and get the
   next one from the Link State Update Packet. AS-external-LSAs
   are not flooded into/throughout stub areas (see Section 3.6).

   (4) Else if the LSA’s LS age is equal to MaxAge, and there is
   currently no instance of the LSA in the router’s link state
   database, and none of router’s neighbors are in states Exchange
   or Loading, then take the following actions: a) Acknowledge the
   receipt of the LSA by sending a Link State Acknowledgment packet
   back to the sending neighbor (see Section 13.5), and b) Discard
   the LSA and examine the next LSA (if any) listed in the Link
   State Update packet.

   (5) Otherwise, find the instance of this LSA that is currently
   contained in the router’s link state database. If there is no
   database copy, or the received LSA is more recent than the
   database copy (see Section 13.1 below for the determination of
   which LSA is more recent) the following steps must be performed:

       (a) If there is already a database copy, and if the database
       copy was received via flooding and installed less than
       MinLSArrival seconds ago, discard the new LSA (without
       acknowledging it) and examine the next LSA (if any) listed
       in the Link State Update packet.

       (b) Otherwise immediately flood the new LSA out some subset of
       the router’s interfaces (see Section 13.3). In some cases
       (e.g., the state of the receiving interface is DR and the
       LSA was received from a router other than the Backup DR) the
       LSA will be flooded back out the receiving interface. This
       occurrence should be noted for later use by the
       acknowledgment process (Section 13.5).

       (c) Remove the current database copy from all neighbors’ Link
       state retransmission lists.

       (d) Install the new LSA in the link state database (replacing
       the current database copy). This may cause the routing
       table calculation to be scheduled. In addition, timestamp
       the new LSA with the current time (i.e., the time it was
       received). The flooding procedure cannot overwrite the
       newly installed LSA until MinLSArrival seconds have elapsed.
       The LSA installation process is discussed further in Section
       13.2.

       (e) Possibly acknowledge the receipt of the LSA by sending a
       Link State Acknowledgment packet back out the receiving
       interface. This is explained below in Section 13.5.

       (f) If this new LSA indicates that it was originated by the
       receiving router itself (i.e., is considered a selforiginated
       LSA), the router must take special action, either
       updating the LSA or in some cases flushing it from the
       routing domain. For a description of how self-originated
       LSAs are detected and subsequently handled, see Section
       13.4.

   (6) Else, if there is an instance of the LSA on the sending
   neighbor’s Link state request list, an error has occurred in the
   Database Exchange process. In this case, restart the Database
   Exchange process by generating the neighbor event BadLSReq for
   the sending neighbor and stop processing the Link State Update
   packet.

   (7) Else, if the received LSA is the same instance as the database
   copy (i.e., neither one is more recent) the following two steps
   should be performed:

       (a) If the LSA is listed in the Link state retransmission list
       for the receiving adjacency, the router itself is expecting
       an acknowledgment for this LSA. The router should treat the
       received LSA as an acknowledgment by removing the LSA from
       the Link state retransmission list. This is termed an
       "implied acknowledgment". Its occurrence should be noted
       for later use by the acknowledgment process (Section 13.5).

       (b) Possibly acknowledge the receipt of the LSA by sending a
       Link State Acknowledgment packet back out the receiving
       interface. This is explained below in Section 13.5.

   (8) Else, the database copy is more recent. If the database copy
   has LS age equal to MaxAge and LS sequence number equal to
   MaxSequenceNumber, simply discard the received LSA without
   acknowledging it. (In this case, the LSA’s LS sequence number is
   wrapping, and the MaxSequenceNumber LSA must be completely
   flushed before any new LSA instance can be introduced).
   Otherwise, as long as the database copy has not been sent in a
   Link State Update within the last MinLSArrival seconds, send the
   database copy back to the sending neighbor, encapsulated within
   a Link State Update Packet. The Link State Update Packet should
   be sent directly to the neighbor. In so doing, do not put the
   database copy of the LSA on the neighbor’s link state
   retransmission list, and do not acknowledge the received (less
   recent) LSA instance. */


void process_lsu_pkt(area *a, neighbor *nbr, ospf_header *ospf_hdr);

/* On broadcast networks, the Link State Update packets are
   multicast. The destination IP address specified for the
   Link State Update Packet depends on the state of the
   interface. If the interface state is DR or Backup, the
   address AllSPFRouters should be used. Otherwise, the
   address AllDRouters should be used.

   On non-broadcast networks, separate Link State Update
   packets must be sent, as unicasts, to each adjacent neighbor
   (i.e., those in state Exchange or greater). The destination
   IP addresses for these packets are the neighbors’ IP
   addresses. */

void encapsulate_lsu_pkt(const area *a, const neighbor *nbr, ospf_header *ospf_hdr);

#endif
