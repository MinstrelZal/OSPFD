#ifndef _DD_H
#define _DD_H

#include "interface.h"
#include "shared.h"

/* 10.6. Receiving Database Description Packets */
/* This section explains the detailed processing of a received
   Database Description Packet. The incoming Database Description
   Packet has already been associated with a neighbor and receiving
   interface by the generic input packet processing (Section 8.2).
   Whether the Database Description packet should be accepted, and
   if so, how it should be further processed depends upon the
   neighbor state.
   If a Database Description packet is accepted, the following
   packet fields should be saved in the corresponding neighbor data
   structure under "last received Database Description packet":
   the packet’s initialize(I), more (M) and master(MS) bits,
   Options field, and DD sequence number. If these fields are set
   identically in two consecutive Database Description packets
   received from the neighbor, the second Database Description
   packet is considered to be a "duplicate" in the processing
   described below.
   If the Interface MTU field in the Database Description packet
   indicates an IP datagram size that is larger than the router can
   accept on the receiving interface without fragmentation, the
   Database Description packet is rejected. Otherwise, if the
   neighbor state is:
   Down
       The packet should be rejected.
   Attempt
       The packet should be rejected.
   Init
       The neighbor state machine should be executed with the event
       2-WayReceived. This causes an immediate state change to
       either state 2-Way or state ExStart. If the new state is
       ExStart, the processing of the current packet should then
       continue in this new state by falling through to case
       ExStart below.
   2-Way
       The packet should be ignored. Database Description Packets
       are used only for the purpose of bringing up adjacencies.
   ExStart
       If the received packet matches one of the following cases,
       then the neighbor state machine should be executed with the
       event NegotiationDone (causing the state to transition to
       Exchange), the packet’s Options field should be recorded in
       the neighbor structure’s Neighbor Options field and the
       packet should be accepted as next in sequence and processed
       further (see below). Otherwise, the packet should be
       ignored.
       o The initialize(I), more (M) and master(MS) bits are set,
       the contents of the packet are empty, and the neighbor’s
       Router ID is larger than the router’s own. In this case
       the router is now Slave. Set the master/slave bit to
       slave, and set the neighbor data structure’s DD sequence
       number to that specified by the master.
       o The initialize(I) and master(MS) bits are off, the
       packet’s DD sequence number equals the neighbor data
       structure’s DD sequence number (indicating
       acknowledgment) and the neighbor’s Router ID is smaller
       than the router’s own. In this case the router is
       Master.
   Exchange
       Duplicate Database Description packets are discarded by the
       master, and cause the slave to retransmit the last Database
       Description packet that it had sent. Otherwise (the packet
       is not a duplicate):
       o If the state of the MS-bit is inconsistent with the
       master/slave state of the connection, generate the
       neighbor event SeqNumberMismatch and stop processing the
       packet.
       o If the initialize(I) bit is set, generate the neighbor
       event SeqNumberMismatch and stop processing the packet.
       o If the packet’s Options field indicates a different set
       of optional OSPF capabilities than were previously
       received from the neighbor (recorded in the Neighbor
       Options field of the neighbor structure), generate the
       neighbor event SeqNumberMismatch and stop processing the
       packet.
       o Database Description packets must be processed in
       sequence, as indicated by the packets’ DD sequence
       numbers. If the router is master, the next packet
       received should have DD sequence number equal to the DD
       sequence number in the neighbor data structure. If the
       router is slave, the next packet received should have DD
       sequence number equal to one more than the DD sequence
       number stored in the neighbor data structure. In either
       case, if the packet is the next in sequence it should be
       accepted and its contents processed as specified below.
       o Else, generate the neighbor event SeqNumberMismatch and
       stop processing the packet.
   Loading or Full
       In this state, the router has sent and received an entire
       sequence of Database Description Packets. The only packets
       received should be duplicates (see above). In particular,
       the packet’s Options field should match the set of optional
       OSPF capabilities previously indicated by the neighbor
       (stored in the neighbor structure’s Neighbor Options field).
       Any other packets received, including the reception of a
       packet with the Initialize(I) bit set, should generate the
       neighbor event SeqNumberMismatch. Duplicates should be
       discarded by the master. The slave must respond to
       duplicates by repeating the last Database Description packet
       that it had sent. */

void process_dd_pkt(interface_data *iface, neighbor *nbr, const ospf_header *ospf_hdr);

/* 10.8. Sending Database Description Packets */
/* This section describes how Database Description Packets are sent
   to a neighbor. The Database Description packet’s Interface MTU
   field is set to the size of the largest IP datagram that can be
   sent out the sending interface, without fragmentation. Common
   MTUs in use in the Internet can be found in Table 7-1 of
   [Ref22]. Interface MTU should be set to 0 in Database
   Description packets sent over virtual links.

   The router’s optional OSPF capabilities (see Section 4.5) are
   transmitted to the neighbor in the Options field of the Database
   Description packet. The router should maintain the same set of
   optional capabilities throughout the Database Exchange and
   flooding procedures. If for some reason the router’s optional
   capabilities change, the Database Exchange procedure should be
   restarted by reverting to neighbor state ExStart. One optional
   capability is defined in this specification (see Sections 4.5
   and A.2). The E-bit should be set if and only if the attached
   network belongs to a non-stub area. Unrecognized bits in the
   Options field should be set to zero.

   The sending of Database Description packets depends on the
   neighbor’s state. In state ExStart the router sends empty
   Database Description packets, with the initialize (I), more (M)
   and master (MS) bits set. These packets are retransmitted every
   RxmtInterval seconds.

   In state Exchange the Database Description Packets actually
   contain summaries of the link state information contained in the
   router’s database. Each LSA in the area’s link-state database
   (at the time the neighbor transitions into Exchange state) is
   listed in the neighbor Database summary list. Each new Database
   Description Packet copies its DD sequence number from the
   neighbor data structure and then describes the current top of
   the Database summary list. Items are removed from the Database
   summary list when the previous packet is acknowledged.

   In state Exchange, the determination of when to send a Database
   Description packet depends on whether the router is master or
   slave:
   Master
       Database Description packets are sent when either a) the
       slave acknowledges the previous Database Description packet
       by echoing the DD sequence number or b) RxmtInterval seconds
       elapse without an acknowledgment, in which case the previous
       Database Description packet is retransmitted.
   Slave
       Database Description packets are sent only in response to
       Database Description packets received from the master. If
       the Database Description packet received from the master is
       new, a new Database Description packet is sent, otherwise
       the previous Database Description packet is resent.

   In states Loading and Full the slave must resend its last
   Database Description packet in response to duplicate Database
   Description packets received from the master. For this reason
   the slave must wait RouterDeadInterval seconds before freeing
   the last Database Description packet. Reception of a Database
   Description packet from the master after this interval will
   generate a SeqNumberMismatch neighbor event. */

void encapsulate_dd_pkt(const interface_data *iface, neighbor *nbr, ospf_header *ospf_hdr);

#endif
