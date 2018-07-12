#include "hello.h"
#include "ospfd.h"

void encapsulate_hello_pkt(const interface_data *iface, ospf_header *ospf_hdr){
	ospf_hello_pkt *hello = (ospf_hello_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	in_addr_t *nbr = hello->neighbors;

	/* encapsulate packet */
	hello->network_mask = iface->network_mask;
	hello->hello_interval = htons(iface->hello_interval);
	hello->options = OSPF_OPTIONS;
	hello->router_priority = 0; // this can also set to OSPF_DEFAULT_ROUTER_PRIORITY
	hello->router_dead_interval = htonl(iface->router_dead_interval);
	hello->d_router = (iface->d_router == 0) ? my_router_id : iface->d_router;
	hello->bd_router = iface->bd_router ? my_router_id : iface->bd_router;
	for(const neighbor *p = iface->neighbors; p; p = p->next){
		*nbr++ = p->neighbor_id;
	}

	/* set ospf header's type and packet length */
	ospf_hdr->type = MSG_TYPE_HELLO;
	ospf_hdr->pktlen = htons((uint8_t *)nbr - (uint8_t *)ospf_hdr);
}


void elect_d_bd_routers(interface_data *iface, neighbor *nbr){
	if(iface->router_priority < nbr->neighbor_priority){
		iface->d_router = nbr->d_router;
		iface->bd_router = nbr->bd_router;
	}
	else{
		if(iface->d_router < nbr->d_router){
			iface->d_router = nbr->d_router;
	    }
	    if(iface->bd_router < nbr->bd_router){
			iface->bd_router = nbr->bd_router;
	    }
	}
}


/* When receiving an Hello Packet from a neighbor on a broadcast,
   Point-to-MultiPoint or NBMA network, set the neighbor
   structure’s Neighbor ID equal to the Router ID found in the
   packet’s OSPF header. For these network types, the neighbor
   structure’s Router Priority field, Neighbor’s Designated Router
   field, and Neighbor’s Backup Designated Router field are also
   set equal to the corresponding fields found in the received
   Hello Packet; changes in these fields should be noted for
   possible use in the steps below. When receiving an Hello on a
   point-to-point network (but not on a virtual link) set the
   neighbor structure’s Neighbor IP address to the packet’s IP
   source address. */
void process_hello_pkt(interface_data *iface, neighbor *nbr, const ospf_header *ospf_hdr, in_addr_t src){
	ospf_hello_pkt *hello = (ospf_hello_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	/* calculate the number of neighbors */
	int num = (ntohs(ospf_hdr->pktlen) - (sizeof(ospf_header) + 20)) / 4;
	const in_addr_t *const nbrs = (in_addr_t *)hello->neighbors;

	/* create a new neighbor node in neighbors list */
	if(!nbr){
		nbr = neighbor_init(hello, ospf_hdr->router_id, src);
		/* add new neighbor struct to interface neighbors */
		nbr->next = iface->neighbors;
		iface->neighbors = nbr;
		iface->num_neighbor += 1;
	}

	/* Start/Restart the Inactivity Timer for the neighbor */
	nbr->inactivity_timer = 0;

	elect_d_bd_routers(iface, nbr);

	/* neighbor state machine change */
	add_neighbor_event(iface, nbr, NEIGHBOR_EV_HELLO_RECEIVED);

	/* check if the router it self is in the neighbor list */
	while(num--){
		if(nbrs[num] == my_router_id){
			add_neighbor_event(iface, nbr, NEIGHBOR_EV_TWO_WAY_RECEIVED);
			break;
		}
	}
	if(num == -1){
		add_neighbor_event(iface, nbr, NEIGHBOR_EV_ONE_WAY);
	}

	/* 10.4. Whether to become adjacent */
	/* Adjacencies are established with some subset of the router’s
       neighbors. Routers connected by point-to-point networks,
       Point-to-MultiPoint networks and virtual links always become
       adjacent. On broadcast and NBMA networks, all routers become
       adjacent to both the Designated Router and the Backup Designated
       Router.
       The adjacency-forming decision occurs in two places in the
       neighbor state machine. First, when bidirectional communication
       is initially established with the neighbor, and secondly, when
       the identity of the attached network’s (Backup) Designated
       Router changes. If the decision is made to not attempt an
       adjacency, the state of the neighbor communication stops at 2-
       Way.
       An adjacency should be established with a bidirectional neighbor
       when at least one of the following conditions holds:
       o The underlying network type is point-to-point
       o The underlying network type is Point-to-MultiPoint
       o The underlying network type is virtual link
       o The router itself is the Designated Router
       o The router itself is the Backup Designated Router
       o The neighboring router is the Designated Router
       o The neighboring router is the Backup Designated Router */
	if(nbr->d_router == src || nbr->d_router == iface->ip ||
		nbr->bd_router == src || nbr->bd_router == iface->ip){
			add_neighbor_event(iface, nbr, NEIGHBOR_EV_ADJ_OK);
	}
	else{
		clear_neighbor_lsas(nbr);
		add_neighbor_event(iface, nbr, NEIGHBOR_EV_ADJ_NO);
	}

}
