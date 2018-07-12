#include "lsr.h"

void process_lsr_pkt(interface_data *iface, neighbor *nbr, ospf_header *ospf_hdr){
	if(nbr->state < NEIGHBOR_STATE_EXCHANGE){
		return ;
	}
	ospf_lsr_pkt *lsr = (ospf_lsr_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	ospf_lsr_pkt *tail = (ospf_lsr_pkt *)((uint8_t *)ospf_hdr + ntohs(ospf_hdr->pktlen));

	int i = 0;
	while(lsr < tail){
		for(i = 0; i < nbr->num_lsr; i++){
			if(lsr->ls_type == (nbr->lsrs+i)->ls_type && 
				lsr->link_state_id == (nbr->lsrs+i)->link_state_id && 
				lsr->adv_router == (nbr->lsrs+i)->adv_router){
				break;
			}
		}
		if(i == nbr->num_lsr){
			nbr->lsrs[nbr->num_lsr++] = *lsr;
		}
		lsr++;
	}

	if(nbr->num_lsa_hdr == 0 && nbr->state == NEIGHBOR_STATE_LOADING){
		add_neighbor_event(iface, nbr, NEIGHBOR_EV_LOADING_DONE);
	}
}

void encapsulate_lsr_pkt(const neighbor *nbr, ospf_header *ospf_hdr){
	ospf_lsr_pkt *lsr = (ospf_lsr_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));

	for(int i = 0; i < nbr->num_lsa_hdr; i++){
		lsr->ls_type = htonl(nbr->lsa_hdrs[i].ls_type);
		lsr->link_state_id = nbr->lsa_hdrs[i].link_state_id;
		lsr->adv_router = nbr->lsa_hdrs[i].adv_router;
		lsr++;
	}

	ospf_hdr->type = MSG_TYPE_LINK_STATE_REQUEST;
	ospf_hdr->pktlen = htons((uint8_t *)lsr - (uint8_t *)ospf_hdr);
}
