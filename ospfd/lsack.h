#ifndef _LSACK_H
#define _LSACK_H

#include "neighbor.h"
#include "shared.h"

/* 13.5. Sending Link State Acknowledgment packets */
/* Each newly received LSA must be acknowledged. This is usually
   done by sending Link State Acknowledgment packets. However,
   acknowledgments can also be accomplished implicitly by sending
   Link State Update packets (see step 7a of Section 13).

   Many acknowledgments may be grouped together into a single Link
   State Acknowledgment packet. Such a packet is sent back out the
   interface which received the LSAs. The packet can be sent in
   one of two ways: delayed and sent on an interval timer, or sent
   directly to a particular neighbor. The particular
   acknowledgment strategy used depends on the circumstances
   surrounding the receipt of the LSA.

   Sending delayed acknowledgments accomplishes several things: 1)
   it facilitates the packaging of multiple acknowledgments in a
   single Link State Acknowledgment packet, 2) it enables a single
   Link State Acknowledgment packet to indicate acknowledgments to
   several neighbors at once (through multicasting) and 3) it
   randomizes the Link State Acknowledgment packets sent by the
   various routers attached to a common network. The fixed
   interval between a router’s delayed transmissions must be short
   (less than RxmtInterval) or needless retransmissions will ensue.

   Direct acknowledgments are sent directly to a particular
   neighbor in response to the receipt of duplicate LSAs. Direct
   acknowledgments are sent immediately when the duplicate is
   received. On multi-access networks, these acknowledgments are
   sent directly to the neighbor’s IP address.

   The precise procedure for sending Link State Acknowledgment
   packets is described in Table 19. The circumstances surrounding
   the receipt of the LSA are listed in the left column. The
   acknowledgment action then taken is listed in one of the two
   right columns. This action depends on the state of the
   concerned interface; interfaces in state Backup behave
   differently from interfaces in all other states. Delayed
   acknowledgments must be delivered to all adjacent routers
   associated with the interface. On broadcast networks, this is
   accomplished by sending the delayed Link State Acknowledgment
   packets as multicasts. The Destination IP address used depends
   on the state of the interface. If the interface state is DR or
   Backup, the destination AllSPFRouters is used. In all other
   states, the destination AllDRouters is used. On non-broadcast
   networks, delayed Link State Acknowledgment packets must be
   unicast separately over each adjacency (i.e., neighbor whose
   state is >= Exchange).

   The reasoning behind sending the above packets as multicasts is
   best explained by an example. Consider the network
   configuration depicted in Figure 15. Suppose RT4 has been
   elected as Designated Router, and RT3 as Backup Designated
   Router for the network N3. When Router RT4 floods a new LSA to
   Network N3, it is received by routers RT1, RT2, and RT3. These
   routers will not flood the LSA back onto net N3, but they still
   must ensure that their link-state databases remain synchronized
   with their adjacent neighbors. So RT1, RT2, and RT4 are waiting
   to see an acknowledgment from RT3. Likewise, RT4 and RT3 are
   both waiting to see acknowledgments from RT1 and RT2. This is
   best achieved by sending the acknowledgments as multicasts.

   The reason that the acknowledgment logic for Backup DRs is
   slightly different is because they perform differently during
   the flooding of LSAs (see Section 13.3, step 4).

                                                       Action taken in state
Circumstances                  Backup                  All other states
--------------------------------------------------------------------------------
LSA has                    No acknowledgment           No acknowledgment
been flooded back          sent.                       sent.
out receiving interface
(see Section
13, step 5b).
--------------------------------------------------------------------------------
LSA is                     Delayed acknowledg-         Delayed ack-
more recent than           ment sent if adver-         nowledgment sent.
database copy, but         tisement received
was not flooded            from Designated
back out receiving         Router, otherwise
interface                  do nothing
--------------------------------------------------------------------------------
LSA is a                   Delayed acknowledg-         No acknowledgment
duplicate, and was         ment sent if adver-         sent.
treated as an im-          tisement received
plied acknowledg-          from Designated
ment (see Section          Router, otherwise
13, step 7a).              do nothing
--------------------------------------------------------------------------------
LSA is a                   Direct acknowledg-          Direct acknowledg-
duplicate, and was         ment sent.                  ment sent.
not treated as an
implied ack-
nowledgment.
--------------------------------------------------------------------------------
LSA’s LS                   Direct acknowledg-          Direct acknowledg-
age is equal to            ment sent.                  ment sent.
MaxAge, and there is
no current instance
of the LSA
in the link state
database, and none
of router’s neighbors
are in states Exchange
or Loading (see
Section 13, step 4).                                                     */

void encapsulate_lsack_pkt(neighbor *nbr, ospf_header *ospf_hdr);


/* 13.7. Receiving link state acknowledgments */
/* Many consistency checks have been made on a received Link State
   Acknowledgment packet before it is handed to the flooding
   procedure. In particular, it has been associated with a
   particular neighbor. If this neighbor is in a lesser state than
   Exchange, the Link State Acknowledgment packet is discarded.

   Otherwise, for each acknowledgment in the Link State
   Acknowledgment packet, the following steps are performed:

   o Does the LSA acknowledged have an instance on the Link state
   retransmission list for the neighbor? If not, examine the
   next acknowledgment. Otherwise:

   o If the acknowledgment is for the same instance that is
   contained on the list, remove the item from the list and
   examine the next acknowledgment. Otherwise:

   o Log the questionable acknowledgment, and examine the next
   one. */

void process_lsack_pkt(neighbor *nbr, ospf_header *ospf_header);

#endif
