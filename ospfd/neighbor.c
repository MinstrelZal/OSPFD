#include <stdio.h>
#include <stdlib.h>

#include "neighbor.h"
#include "ospfd.h"

const neighbor_sm_entry nsm[] = {
	/* Send an Hello Packet to the neighbor (this neighbor is always associated with an NBMA network) and start
	the Inactivity Timer for the neighbor. The timer’s later firing would indicate that communication with
	the neighbor was not attained. */
	/* {NEIGHBOR_STATE_DOWN    , NEIGHBOR_EV_START                   , NEIGHBOR_STATE_ATTEMPT}, */

	/* Restart the Inactivity Timer for the neighbor, since the neighbor has now been heard from. */
	/* {NEIGHBOR_STATE_ATTEMPT , NEIGHBOR_EV_HELLO_RECEIVED          , NEIGHBOR_STATE_INIT}, */

	/* Start the Inactivity Timer for the neighbor. The timer’s later firing would indicate that the neighbor is dead. */
	{NEIGHBOR_STATE_DOWN    , NEIGHBOR_EV_HELLO_RECEIVED          , NEIGHBOR_STATE_INIT},

	/* Restart the Inactivity Timer for the neighbor, since the neighbor has again been heard from. */
	/* {NEIGHBOR_STATE_INIT or greater, NEIGHBOR_EV_HELLO_RECEIVED, No state change} */

	/* Determine whether an adjacency should be established with the neighbor (see Section 10.4). If not, the
       new neighbor state is 2-Way.
       Otherwise (an adjacency should be established) the neighbor state transitions to ExStart. Upon entering 
       this state, the router increments the DD sequence number in the neighbor data structure. If this is the 
       first time that an adjacency has been attempted, the DD sequence number should be assigned some unique 
       value (like the time of day clock). It then declares itself master (sets the master/slave bit to master), 
       and starts sending Database Description Packets, with the initialize (I), more (M) and master (MS) bits 
       set. This Database Description Packet should be otherwise empty. This Database Description Packet should 
       be retransmitted at intervals of RxmtInterval until the next state is entered (see Section 10.8). */
	{NEIGHBOR_STATE_INIT    , NEIGHBOR_EV_TWO_WAY_RECEIVED        , NEIGHBOR_STATE_TWO_WAY},
	/* {NEIGHBOR_STATE_INIT    , NEIGHBOR_EV_TWO_WAY_RECEIVED        , NEIGHBOR_STATE_EX_START}, */

	/* The router must list the contents of its entire area link state database in the neighbor Database summary
       list. The area link state database consists of the router-LSAs, network-LSAs and summary-LSAs contained
       in the area structure, along with the AS-external-LSAs contained in the global structure. ASexternal-LSAs 
       are omitted from a virtual neighbor’s Database summary list. AS-external-LSAs are omitted from the Database 
       summary list if the area has been configured as a stub (see Section 3.6). LSAs whose age is equal to MaxAge 
       are instead added to the neighbor’s Link state retransmission list. A summary of the Database summary list 
       will be sent to the neighbor in Database Description packets. Each Database Description Packet has a DD 
       sequence number, and is explicitly acknowledged. Only one Database Description Packet is allowed outstanding
       at any one time. For more detail on the sending and receiving of Database Description packets, 
       see Sections 10.8 and 10.6. */
	{NEIGHBOR_STATE_EX_START, NEIGHBOR_EV_NEGOTIATION_DONE        , NEIGHBOR_STATE_EXCHANGE},

	/* If the neighbor Link state request list is empty, the new neighbor state is Full. No other action is
       required. This is an adjacency’s final state. 
       Otherwise, the new neighbor state is Loading. Start (or continue) sending Link State Request packets 
       to the neighbor (see Section 10.9). These are requests for the neighbor’s more recent LSAs (which 
       were discovered but not yet received in the Exchange state). These LSAs are listed in the Link state
       request list associated with the neighbor. */
	/* {NEIGHBOR_STATE_EXCHANGE, NEIGHBOR_EV_EXCHANGE_DONE           , NEIGHBOR_STATE_FULL}, */
	{NEIGHBOR_STATE_EXCHANGE, NEIGHBOR_EV_EXCHANGE_DONE           , NEIGHBOR_STATE_LOADING},

	/* No action required. This is an adjacency’s final state. */
	{NEIGHBOR_STATE_LOADING , NEIGHBOR_EV_LOADING_DONE            , NEIGHBOR_STATE_FULL}, 

	/* Determine whether an adjacency should be formed with the neighboring router (see Section 10.4). If not,
       the neighbor state remains at 2-Way. 
       Otherwise, transition the neighbor state to ExStart and perform the actions associated with the above 
       state machine entry for state Init and event 2-WayReceived. */
	{NEIGHBOR_STATE_TWO_WAY , NEIGHBOR_EV_ADJ_OK                  , NEIGHBOR_STATE_EX_START},

	/* Determine whether the neighboring router should still be adjacent. If yes, there is no state change
       and no further action is necessary.
       Otherwise, the (possibly partially formed) adjacency must be destroyed. The neighbor state transitions 
       to 2-Way. The Link state retransmission list, Database summary list and Link state request list
       are cleared of LSAs. */
	/* {NEIGHBOR_STATE_EX_START or greater, NEIGHBOR_EV_ADJ_OK                  , no state change}, */
	{NEIGHBOR_STATE_EX_START           , NEIGHBOR_EV_ADJ_NO                  , NEIGHBOR_STATE_TWO_WAY},
	{NEIGHBOR_STATE_EXCHANGE           , NEIGHBOR_EV_ADJ_NO                  , NEIGHBOR_STATE_TWO_WAY},
	{NEIGHBOR_STATE_LOADING            , NEIGHBOR_EV_ADJ_NO                  , NEIGHBOR_STATE_TWO_WAY},
	{NEIGHBOR_STATE_FULL               , NEIGHBOR_EV_ADJ_NO                  , NEIGHBOR_STATE_TWO_WAY},

	/* The (possibly partially formed) adjacency is torn down, and then an attempt is made at reestablishment. 
	   The neighbor state first transitions to ExStart. The Link state retransmission list, Database summary 
	   list and Link state request list are cleared of LSAs. Then the router increments the DD sequence number 
	   in the neighbor data structure, declares itself master (sets the master/slave bit to master), and starts 
	   sending Database Description Packets, with the initialize (I), more (M) and master (MS) bits set. This 
	   Database Description Packet should be otherwise empty (see Section 10.8). */
	/* {NEIGHBOR_STATE_EXCHANGE or greater, NEIGHBOR_EV_SEQUENCE_NUMBER_MISMATCH, NEIGHBOR_STATE_EX_START}, */

	/* The action for event BadLSReq is exactly the same as for the neighbor event SeqNumberMismatch. The
       (possibly partially formed) adjacency is torn down, and then an attempt is made at reestablishment. For
       more information, see the neighbor state machine entry that is invoked when event SeqNumberMismatch
       is generated in state Exchange or greater. */
	/* {NEIGHBOR_STATE_EXCHANGE or greater, NEIGHBOR_EV_BAD_LS_REQ              , NEIGHBOR_STATE_EX_START}, */

	/* The Link state retransmission list, Database summary list and Link state request list are cleared of
       LSAs. Also, the Inactivity Timer is disabled. */
	/* {Any state, NEIGHBOR_EV_KILL_NBR, NEIGHBOR_STATE_DOWN} */

	/* The Link state retransmission list, Database summary list and Link state request list are cleared of
       LSAs. Also, the Inactivity Timer is disabled. */
	/* {Any state, NEIGHBOR_EV_LL_DOWN, NEIGHBOR_STATE_DOWN} */

	/* The Link state retransmission list, Database summary list and Link state request list are cleared of
       LSAs. */
	/* {Any state, NEIGHBOR_EV_INACTIVITY_TIMER, NEIGHBOR_STATE_DOWN} */

	/* The Link state retransmission list, Database summary list and Link state request list are cleared of
       LSAs. */
	{NEIGHBOR_STATE_TWO_WAY , NEIGHBOR_EV_ONE_WAY                 , NEIGHBOR_STATE_INIT},
	/* {or greater , NEIGHBOR_EV_ONE_WAY                 , NEIGHBOR_STATE_INIT}, */

	/* No action required. */
	/* {NEIGHBOR_STATE_TWO_WAY or greater, NEIGHBOR_EV_TWO_WAY_RECEIVED, No state change} */

	/* No action required. */
	/* {NEIGHBOR_STATE_INIT, NEIGHBOR_EV_ONE_WAY, No state change} */
};

void print_neighbor_info(neighbor *nbr){
        printf("----------Neighbor Info---------\n");
        printf("state: %s\n", neighbor_state_str[nbr->state]);
        printf("Router ID: %s\n",  inet_ntoa((struct in_addr){nbr->neighbor_id}));
        printf("priority: %d\n", nbr->neighbor_priority);
        printf("ip: %s\n",  inet_ntoa((struct in_addr){nbr->neighbor_ip}));
        printf("master/slave: %d\n", nbr->master_slave_relationship);
        printf("lsa number: %d\n", nbr->num_lsa_hdr);
        printf("--------------------------------\n");
}

void add_neighbor_event(interface_data *iface, neighbor *nbr, neighbor_event event){

	printf("--------------------------------\n");

	printf("Interface: %s\n", iface->if_name);
	printf("Neighbor: %s\n", inet_ntoa((struct in_addr){nbr->neighbor_ip}));
	printf("Current State: %s\n", neighbor_state_str[nbr->state]);
	printf("Event: %s\n", neighbor_event_str[event]);

	for (int i = 0; i < NEIGHBOR_SM_ENTRY_NUM; i++){
		if((nbr->state == nsm[i].cur_state) && (event == nsm[i].recv_event)){
			nbr->state = nsm[i].new_state;
			if(nbr->state == NEIGHBOR_STATE_FULL){
				if(iface->d_router == nbr->neighbor_ip){
					iface->state = 1;
				}
			}
			break;
		}
	}

	printf("New State: %s\n", neighbor_state_str[nbr->state]);

	printf("--------------------------------\n");
        print_neighbor_info(nbr);
}

in_addr_t lookup_neighbor_ip_by_id(const area *a, in_addr_t id){
	for (int i = 0; i < a->num_if; i++){
		for(neighbor *p = a->ifs[i]->neighbors; p; p = p->next){
			if (p->neighbor_id == id){
				return p->neighbor_ip;
			}
		}
	}
	return 0;
}

neighbor *neighbor_init(ospf_hello_pkt *hello, uint32_t router_id, in_addr_t src){
	neighbor *nbr = (neighbor *)malloc(sizeof(neighbor));
	nbr->state = NEIGHBOR_STATE_DOWN;
	nbr->inactivity_timer = 0;
	nbr->master_slave_relationship = DD_MASTER;
	nbr->dd_seqnum = DEFAULT_DD_SEQ_NUM_BEGIN;
	nbr->last_dd_flags = 0;
	nbr->last_dd_options = 0;
	nbr->last_dd_seqnum = 0;
	nbr->neighbor_id = router_id;
	nbr->neighbor_priority = hello->router_priority;
	nbr->neighbor_ip = src;
	nbr->options = hello->options;
	nbr->d_router = hello->d_router;
	nbr->bd_router = hello->bd_router;
	nbr->num_lsa_hdr = 0;
	nbr->num_lsr = 0;
	nbr->num_lsack = 0;
	nbr->next = NULL;
	nbr->more = 1;
	printf("create a new neighbor\n");
	print_neighbor_info(nbr);
	return nbr;
}

void clear_neighbor_lsas(neighbor *nbr){
	nbr->num_lsa_hdr = 0;
	nbr->num_lsr = 0;
	nbr->num_lsack = 0;
}
