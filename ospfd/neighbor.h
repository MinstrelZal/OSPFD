#ifndef _NEIGHBOR_H
#define _NEIGHBOR_H

#include "ospf_packets.h"
#include "interface.h"
#include "area.h"
#include "shared.h"

#include <netinet/in.h>
#include <arpa/inet.h>
/*
typedef struct ospf_lsa_header ospf_lsa_header;
typedef struct ospf_lsr_pkt ospf_lsr_pkt;
typedef struct ospf_hello_pkt ospf_hello_pkt;
*/
/*
typedef struct interface_data interface_data;
typedef struct area area;
*/
struct interface_data;
struct area;
typedef enum{
	/* Down - This is the initial state of a neighbor conversation.
	   It indicates that there has been no recent information received
       from the neighbor.  On NBMA networks, Hello packets may still
	   be sent to "Down" neighbors, although at a reduced frequency
	   (see Section 9.5.1). */
	NEIGHBOR_STATE_DOWN,

	/* Attempt - This state is only valid for neighbors attached
	   to NBMA networks. It indicates that no recent information
	   has been received from the neighbor, but that a more
	   concerted effort should be made to contact the neighbor.
	   This is done by sending the neighbor Hello packets at intervals
	   of HelloInterval (see Section 9.5.1). */
	NEIGHBOR_STATE_ATTEMPT,

	/* Init - In this state, an Hello packet has recently been seen
	   from the neighbor. However, bidirectional communication has not
	   yet been established with the neighbor (i.e., the router itself
	   did not appear in the neighbor's Hello packet).  All neighbors
	   in this state (or higher) are listed in the Hello packets sent
	   from the associated interface. */
	NEIGHBOR_STATE_INIT,

	/* 2-Way - In this state, communication between the two routers is
       bidirectional.  This has been assured by the operation of the
	   Hello Protocol.  This is the most advanced state short of beginning
	   adjacency establishment.  The (Backup) Designated Router is selected
	   from the set of neighbors in state 2-Way or greater. */
	NEIGHBOR_STATE_TWO_WAY,

	/* ExStart - This is the first step in creating an adjacency between
	   the two neighboring routers.  The goal of this step is to decide
       which router is the master, and to decide upon the initial DD sequence
	   number. Neighbor conversations in this state or greater are called
	   adjacencies. */
	NEIGHBOR_STATE_EX_START,

	/* Exchange - In this state the router is describing its entire link
	   state database by sending Database Description packets to the neighbor.
	   Each Database Description Packet has a DD sequence number, and is
	   explicitly acknowledged.  Only one Database Description Packet is
	   allowed outstanding at any one time. In this state, Link State Request
	   Packets may also be sent asking for the neighbor's more recent LSAs.
	   All adjacencies in Exchange state or greater are used by the flooding
	   procedure.  In fact, these adjacencies are fully capable of transmitting
	   and receiving all types of OSPF routing protocol packets. */
	NEIGHBOR_STATE_EXCHANGE,

	/* Loading - In this state, Link State Request packets are sent to the
	   neighbor asking for the more recent LSAs that have been discovered
	   (but not yet received) in the Exchange state. */
	NEIGHBOR_STATE_LOADING,

	/* Full - In this state, the neighboring routers are fully adjacent.
       These adjacencies will now appear in router-LSAs and network-LSAs. */
	NEIGHBOR_STATE_FULL
}neighbor_state;


typedef struct neighbor{
	/* State - the functional level of the neighbor
	   conversation. This is described in more detail
	   in Section 10.1. */
	neighbor_state state;

    /* Inactive Timer - a single shot timer whose firing
	   indicates that no Hello Packet has been seen from
	   this neighbor recently. The length of the timer is
	   RouterDeadInterval seconds. */
	int inactivity_timer;

	/* Master/Slave - When the two neighbors are exchanging
	   databases, they form a master/slave relationship.
	   The master sends the first Database Description Packet,
	   and is the only part that is allowed to retransmit.
	   The slave can only respond to the master's Database
       Description Packets. The master/slave relationship is
       negotiated in state ExStart. */
	int master_slave_relationship;

    /* DD Sequence Number -- the DD Sequence number of the
	   Database Description packet that is currently being
	   sent to the neighbor. */
	int dd_seqnum;

    /* Last received Database Description packet -- The initialize(I),
	   more (M) and master(MS) bits, Options field, and DD sequence number
	   contained in the last Database Description packet received from
	   the neighbor. Used to determine whether the next Database
	   Description packet received from the neighbor is a duplicate. */
	uint8_t last_dd_flags;
	uint8_t last_dd_options;
	int last_dd_seqnum;

	/* Neighbor ID -- The OSPF Router ID of the neighboring router.
	   The Neighbor ID is learned when Hello packets are received from
	   the neighbor, or is configured if this is a virtual adjacency
	   (see Section C.4). */
	in_addr_t neighbor_id;

	/* Neighbor Priority -- the Router Priority of the neighboring router.
	   Contained in the neighbor's Hello packets, this item is used when
	   selecting the Designated Router for the attached network. */
	uint8_t neighbor_priority;

    /* Neighbor IP address -- The IP address of the neighboring router's
	   interface to the attached network. Used as the Destination IP address
	   when protocol packets are sent as unicasts along this adjacency.
       Also used in router-LSAs as the Link ID for the attached network
       if the neighboring router is selected to be Designated Router
       (see Section 12.4.1).  The Neighbor IP address is learned when
       Hello packets are received from the neighbor. For virtual links,
	   the Neighbor IP address is learned during the routing table build
	   process (see Section 15). */
	in_addr_t neighbor_ip;

	/* Neighbor Options -- the optional OSPF capabilities supported by
	   the neighbor. Learned during the Database Exchange process
	   (see Section 10.6). The neighbor's optional OSPF capabilities are
	   also listed in its Hello packets. This enables received Hello Packets
	   to be rejected (i.e., neighbor relationships will not even start to
       form) if there is a mismatch in certain crucial OSPF capabilities
	   (see Section 10.5). The optional OSPF capabilities are documented in
	   Section 4.5. */
	int options;

	/* Neighbor’s Designated Router -- The neighbor’s idea of the Designated 
	Router. If this is the neighbor itself, this is important in the local 
	calculation of the Designated Router. Defined only on broadcast and NBMA
    networks. */
	in_addr_t d_router;

	/* Neighbor’s Backup Designated Router -- The neighbor’s idea of the 
	   Backup Designated Router. If this is the neighbor itself, this is 
	   important in the local calculation of the Backup Designated Router. 
	   Defined only on broadcast and NBMA networks. */
	in_addr_t bd_router;

	/* The next set of variables are lists of LSAs. These lists describe
           subsets of the area link-state database. This memo defines five
           distinct types of LSAs, all of which may be present in an area
           link-state database: router-LSAs, network-LSAs, and Type 3 and 4
           summary-LSAs (all stored in the area data structure), and ASexternal-
           LSAs (stored in the global data structure). */

	/* Link state retransmission list */
        /* The list of LSAs that have been flooded but not acknowledged on
           this adjacency. These will be retransmitted at intervals until
           they are acknowledged, or until the adjacency is destroyed. */

	/* Database summary list */
	/* The complete list of LSAs that make up the area link-state
           database, at the moment the neighbor goes into Database Exchange
           state. This list is sent to the neighbor in Database
           Description packets. */

	/* Link state request list */
	/* The list of LSAs that need to be received from this neighbor in
           order to synchronize the two neighbors’ link-state databases.
           This list is created as Database Description packets are
           received, and is then sent to the neighbor in Link State Request
           packets. The list is depleted as appropriate Link State Update
           packets are received. */
	int num_lsa_hdr;
	ospf_lsa_header lsa_hdrs[LIST_MAX];

	/* The LSR packet received from the neighbor */
	int num_lsr;
	ospf_lsr_pkt lsrs[LIST_MAX];

	/* The updated LSA header that need to ack */
	int num_lsack;
	ospf_lsa_header lsacks[LIST_MAX];

	/* next neighbor */
	struct neighbor *next;

	int more;

	uint8_t pre_dd_pkt[BUFFER_SIZE];
}neighbor;


/* 10.2. Events causing neighbor state changes */
typedef enum{
	/* An Hello packet has been received from the neighbor. */
	NEIGHBOR_EV_HELLO_RECEIVED,

	/* This is an indication that Hello Packets should now be sent
       to the neighbor at intervals of HelloInterval seconds. This
       event is generated only for neighbors associated with NBMA
       networks. */
	NEIGHBOR_EV_START,

	/* Bidirectional communication has been realized between the
       two neighboring routers. This is indicated by the router
       seeing itself in the neighbor’s Hello packet. */
	NEIGHBOR_EV_TWO_WAY_RECEIVED,

	/* The Master/Slave relationship has been negotiated, and DD
       sequence numbers have been exchanged. This signals the
       start of the sending/receiving of Database Description
       packets. For more information on the generation of this
       event, consult Section 10.8. */
	NEIGHBOR_EV_NEGOTIATION_DONE,

	/* Both routers have successfully transmitted a full sequence
       of Database Description packets. Each router now knows what
       parts of its link state database are out of date. For more
       information on the generation of this event, consult Section
       10.8. */
	NEIGHBOR_EV_EXCHANGE_DONE,

	/* A Link State Request has been received for an LSA not
       contained in the database. This indicates an error in the
       Database Exchange process. */
	NEIGHBOR_EV_BAD_LS_REQ,

	/* Link State Updates have been received for all out-of-date
	   portions of the database. This is indicated by the Link
       state request list becoming empty after the Database
       Exchange process has completed. */
	NEIGHBOR_EV_LOADING_DONE,

	/* A decision must be made as to whether an adjacency should be
       established/maintained with the neighbor. This event will
       start some adjacencies forming, and destroy others. */
	NEIGHBOR_EV_ADJ_OK,
	NEIGHBOR_EV_ADJ_NO,

	/* A Database Description packet has been received that either
       a) has an unexpected DD sequence number, b) unexpectedly has
       the Init bit set or c) has an Options field differing from
       the last Options field received in a Database Description
       packet. Any of these conditions indicate that some error
       has occurred during adjacency establishment. */
	NEIGHBOR_EV_SEQUENCE_NUMBER_MISMATCH,

	/* An Hello packet has been received from the neighbor, in
       which the router is not mentioned. This indicates that
       communication with the neighbor is not bidirectional. */
	NEIGHBOR_EV_ONE_WAY,

	/* This is an indication that all communication with the
       neighbor is now impossible, forcing the neighbor to
       revert to Down state. */
	NEIGHBOR_EV_KILL_NBR,

	/* The inactivity Timer has fired. This means that no Hello
       packets have been seen recently from the neighbor. The
       neighbor reverts to Down state. */
	NEIGHBOR_EV_INACTIVITY_TIMER,

	/* This is an indication from the lower level protocols that
       the neighbor is now unreachable. For example, on an X.25
       network this could be indicated by an X.25 clear indication 
       with appropriate cause and diagnostic fields. This event
       forces the neighbor into Down state. */
	NEIGHBOR_EV_LL_DOWN
}neighbor_event;

typedef struct neighbor_sm_entry{
	neighbor_state cur_state;
	neighbor_event recv_event;
	neighbor_state new_state;
}neighbor_sm_entry;

/* neighbor state machine */
extern const neighbor_sm_entry nsm[];

void add_neighbor_event(struct interface_data *iface, neighbor *nbr, neighbor_event event);

in_addr_t lookup_neighbor_ip_by_id(const struct area *a, in_addr_t id);

neighbor *neighbor_init(ospf_hello_pkt *hello, uint32_t router_id, in_addr_t src);

void clear_neighbor_lsas(neighbor *nbr);

#endif
