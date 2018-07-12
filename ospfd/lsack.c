#include "lsack.h"

void encapsulate_lsack_pkt(neighbor *nbr, ospf_header *ospf_hdr){
	ospf_lsa_header *lsa_hdr = (ospf_lsa_header *)((uint8_t *)ospf_hdr + sizeof(ospf_header));

	for(int i = 0; i < nbr->num_lsack; i++){
		*lsa_hdr++ = nbr->lsacks[i];
	}

	ospf_hdr->type = MSG_TYPE_LINK_STATE_ACK;
	ospf_hdr->pktlen = htons((uint8_t*)lsa_hdr - (uint8_t *)ospf_hdr);
	nbr->num_lsack = 0;
}

void process_lsack_pkt(neighbor *nbr, ospf_header *ospf_hdr){
	/* If this neighbor is in a lesser state than Exchange, the Link State Acknowledgment packet is discarded. */
	if(nbr->state < NEIGHBOR_STATE_EXCHANGE){
		return ;
	}

	ospf_lsa_header *lsa_hdr = (ospf_lsa_header *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	int num = (ntohs(ospf_hdr->pktlen) - sizeof(ospf_header)) / sizeof(ospf_lsa_header);

	int i = 0;
	int j = 0;
	while(num--){
		for(i = 0; i < nbr->num_lsr; i++){
			if(nbr->lsrs[i].ls_type == lsa_hdr->ls_type && 
				nbr->lsrs[i].link_state_id == lsa_hdr->link_state_id &&
				nbr->lsrs[i].adv_router == lsa_hdr->adv_router){
				nbr->num_lsr -= 1;
				for(j = i; j < nbr->num_lsr; j++){
					nbr->lsrs[j] = nbr->lsrs[j+1];
				}
				break;
			}
		}
	}
}
