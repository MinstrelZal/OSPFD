#include "dd.h"
#include "ospfd.h"
#include "lsa.h"
#include <stdio.h>
#include <string.h>

void encapsulate_dd_pkt(const interface_data *iface, neighbor *nbr, ospf_header *ospf_hdr){
	ospf_dd_pkt *dd = (ospf_dd_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	ospf_lsa_header *lsa_hdr = dd->lsa_hdrs;
	area *a = lookup_area_by_if(iface);

	dd->interface_mtu = htons(DEFAULT_MTU);
	dd->options = OSPF_OPTIONS;
	dd->flags = 0;
	if(nbr->master_slave_relationship == DD_MASTER){
		dd->flags |= DD_FLAG_MS;
	}
	if(nbr->state == NEIGHBOR_STATE_EX_START){
		dd->flags |= DD_FLAG_I;
	}
	if(nbr->more){
		dd->flags |= DD_FLAG_M;
	}
	dd->dd_seqnum = htonl(nbr->dd_seqnum);
	/* In state Exchange the Database Description Packets actually
       contain summaries of the link state information contained in the
       router’s database. Each LSA in the area’s link-state database
       (at the time the neighbor transitions into Exchange state) is
       listed in the neighbor Database summary list. Each new Database
       Description Packet copies its DD sequence number from the
       neighbor data structure and then describes the current top of
       the Database summary list. Items are removed from the Database
       summary list when the previous packet is acknowledged. */
	if(nbr->state == NEIGHBOR_STATE_EXCHANGE && nbr->more){
		for(int i = 0; i < a->num_lsa; i++){
			memcpy(lsa_hdr++, a->lsas[i], sizeof(ospf_lsa_header));
		}
	}
	ospf_hdr->type = MSG_TYPE_DATABASE_DESCRIPTION;
	ospf_hdr->pktlen = htons((uint8_t *)lsa_hdr - (uint8_t *)ospf_hdr);

	memcpy(nbr->pre_dd_pkt + sizeof(struct iphdr), ospf_hdr, BUFFER_SIZE - sizeof(struct iphdr));
}

void process_dd_pkt(interface_data *iface, neighbor *nbr, const ospf_header *ospf_hdr){
	ospf_dd_pkt *dd = (ospf_dd_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	area *a = lookup_area_by_if(iface);
	/*
	if(nbr->last_dd_flags == dd->flags && nbr->last_dd_options == dd->options 
		&& htonl(nbr->last_dd_seqnum) == dd->dd_seqnum){
		printf("Duplicate Database Description packet.\n");
	    return;
	}
	*/

	if(dd->flags & DD_FLAG_I){
		if((dd->flags & DD_FLAG_M) && ntohl(my_router_id) < ntohl(ospf_hdr->router_id)){
			nbr->master_slave_relationship = DD_SLAVE;
			nbr->dd_seqnum = dd->dd_seqnum;
			add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
		}
	}
	else{
		if(!(dd->flags & DD_FLAG_MS)){
			nbr->master_slave_relationship = DD_MASTER;
			add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
		}
		if(nbr->master_slave_relationship == DD_MASTER){
			if(htonl(nbr->dd_seqnum) == dd->dd_seqnum){
				add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
				nbr->dd_seqnum += 1;
				if(!(dd->flags & DD_FLAG_M)){
					add_neighbor_event(iface, nbr, NEIGHBOR_EV_EXCHANGE_DONE);
				}
			}
		}
		else{
			if(htonl(nbr->dd_seqnum) == dd->dd_seqnum-1){
				nbr->dd_seqnum += 1;
				nbr->more = dd->flags & DD_FLAG_M;
			}
		}
		/* calculate the number of LSA */
		int num = (ntohs(ospf_hdr->pktlen) - sizeof(ospf_header) - sizeof(ospf_dd_pkt)) / sizeof(ospf_lsa_header);
		if(num == 0){
		    nbr->more = 0;
		}
		for(int i = 0; i < num; i++){
			ospf_lsa_header *lsa_hdr = lookup_lsa(a, dd->lsa_hdrs + i);
			if(!lsa_hdr || cmp_lsa_hdr(lsa_hdr, dd->lsa_hdrs + i) < 0){
				add_lsa_hdr(nbr, dd->lsa_hdrs + i);
		    }
	    }
	}
}

/* the following code are the initial version of my implementation of process dd packet 
   but it seems like there exists some bugs */
	/*
	int num = 0;

	switch(nbr->state){
		case NEIGHBOR_STATE_DOWN:
		case NEIGHBOR_STATE_ATTEMPT:
		case NEIGHBOR_STATE_TWO_WAY:break;
		case NEIGHBOR_STATE_INIT:
		    add_neighbor_event(iface, nbr, NEIGHBOR_EV_TWO_WAY_RECEIVED);
		    add_neighbor_event(iface, nbr, NEIGHBOR_EV_ADJ_OK);
		    if(nbr->state != NEIGHBOR_STATE_EX_START){
		    	break;
		    }
		case NEIGHBOR_STATE_EX_START:*/
		    /* calculate the number of LSA */
		    //num = (ntohs(ospf_hdr->pktlen) - sizeof(ospf_header) - sizeof(ospf_dd_pkt)) / sizeof(ospf_lsa_header);
		    /* The initialize(I), more (M) and master(MS) bits are set,
               the contents of the packet are empty, and the neighbor’s
               Router ID is larger than the router’s own. */
		    /*if((dd->flags & DD_FLAG_I) && (dd->flags & DD_FLAG_M) && (dd->flags & DD_FLAG_MS) && 
		    	num == 0 && ntohl(my_router_id) < ntohl(ospf_hdr->router_id)){
		    	nbr->options = dd->options;
		    	nbr->master_slave_relationship = DD_SLAVE;
		        nbr->dd_seqnum = ntohl(dd->dd_seqnum);
		        add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
		    }*/
		    /* The initialize(I) and master(MS) bits are off, the
               packet’s DD sequence number equals the neighbor data
               structure’s DD sequence number (indicating
               acknowledgment) and the neighbor’s Router ID is smaller
               than the router’s own. */
		    /*else if(!(dd->flags & DD_FLAG_I) && !(dd->flags & DD_FLAG_MS) 
		    	&& dd->dd_seqnum == htonl(nbr->dd_seqnum) && 
		    	ntohl(my_router_id) > ntohl(ospf_hdr->router_id)){
		    	nbr->options = dd->options;
		    	nbr->master_slave_relationship = DD_MASTER;
		    	add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
		    }
		    break;
		case NEIGHBOR_STATE_EXCHANGE:*/
		    /* If the router is master, the next packet received should have DD sequence 
		       number equal to the DD sequence number in the neighbor data structure. */
		    /*if(nbr->master_slave_relationship == DD_MASTER){
			    if(dd->dd_seqnum == htonl(nbr->dd_seqnum)){
				    add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
				    nbr->dd_seqnum += 1;*/
				    /* M-bit = 0, the last dd packet */
				    /*if(!(dd->flags & DD_FLAG_M)){
				    	add_neighbor_event(iface, nbr, NEIGHBOR_EV_EXCHANGE_DONE);
				    }
			    }
			    nbr->last_dd_flags = dd->flags;
	            nbr->last_dd_options = dd->options;
	            nbr->last_dd_seqnum = ntohl(dd->dd_seqnum);
		    } */
		    /* If the router is slave, the next packet received should have DD sequence 
		       number equal to one more than the DD sequence number stored in the neighbor 
		       data structure. */
		   /* else{
			    if(dd->dd_seqnum == htonl(nbr->dd_seqnum + 1)){
				    nbr->dd_seqnum += 1;*/
				    // nbr->last_dd_flags = dd->flags;
				    //add_neighbor_event(iface, nbr, NEIGHBOR_EV_NEGOTIATION_DONE);
				    /* M-bit = 0, the last dd packet */
				    /*if((dd->flags & DD_FLAG_M) != DD_FLAG_M){
					    add_neighbor_event(iface, nbr, NEIGHBOR_EV_EXCHANGE_DONE);
				    }
				    nbr->last_dd_flags = dd->flags;
	                nbr->last_dd_options = dd->options;
	                nbr->last_dd_seqnum = ntohl(dd->dd_seqnum);
	                nbr->more = dd->flags & DD_FLAG_M;
			    }
		    }*/
		    /* calculate the number of LSA */
		    /*num = (ntohs(ospf_hdr->pktlen) - sizeof(ospf_header) - sizeof(ospf_dd_pkt)) / sizeof(ospf_lsa_header);
		    if(num == 0){
		    	nbr->more = 0;
		    }
		    for(int i = 0; i < num; i++){
			    ospf_lsa_header *lsa_hdr = lookup_lsa(a, dd->lsa_hdrs + i);
			    if(!lsa_hdr || cmp_lsa_hdr(lsa_hdr, dd->lsa_hdrs + i) < 0){
				    add_lsa_hdr(nbr, dd->lsa_hdrs + i);
			    }
		    }
		    break;
		case NEIGHBOR_STATE_LOADING:
		case NEIGHBOR_STATE_FULL:
		default: break;
	}

}*/
