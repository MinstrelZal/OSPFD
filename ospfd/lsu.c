#include "lsu.h"
#include "lsa.h"

#include <string.h>

void process_lsu_pkt(area *a, neighbor *nbr, ospf_header *ospf_hdr){
	ospf_lsu_pkt *lsu = (ospf_lsu_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	uint8_t *lsa_begin = (uint8_t *)ospf_hdr + sizeof(ospf_header) + sizeof(ospf_lsu_pkt);

	// int num = ntohl(*((uint32_t *)((uint8_t *)ospf_hdr + sizeof(struct ospf_header))));
	int num = lsu->num_of_lsa;
	while(num--){
		ospf_lsa_header *lsa_hdr = (ospf_lsa_header *)lsa_begin;

		uint16_t sum = ntohs(lsa_hdr->ls_chksum);
		lsa_hdr->ls_chksum = 0;
		/* check the checksum */
		if(sum != fletcher16(lsa_begin + sizeof(lsa_hdr->ls_age), ntohs(lsa_hdr->length) - sizeof(lsa_hdr->ls_age))){
			continue;
		}
		lsa_hdr->ls_chksum = htons(sum);

		for(int i = 0; i < nbr->num_lsa_hdr; i++){
			if(lsa_hdr_eql(nbr->lsa_hdrs + i, lsa_hdr)){
				// nbr->lsa_hdrs[i] = nbr->lsa_hdrs[--nbr->num_lsa_hdr];
				nbr->num_lsa_hdr -= 1;
				for(int j = i; j < nbr->num_lsa_hdr; j++){
					nbr->lsa_hdrs[j] = nbr->lsa_hdrs[j+1];
				}
				break;
			}
		}
		/* add it to the ack list */
		nbr->lsacks[nbr->num_lsack++] = *lsa_hdr;
		/* install it to the link state database of area a */
		install_lsa(a, lsa_hdr);
		lsa_begin += ntohs(lsa_hdr->length);
	}
}

void encapsulate_lsu_pkt(const area *a, const neighbor *nbr, ospf_header *ospf_hdr){
	ospf_lsu_pkt *lsu = (ospf_lsu_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	uint8_t *lsa_begin = (uint8_t *)ospf_hdr + sizeof(ospf_header) + sizeof(ospf_lsu_pkt);
	// uint8_t *p = (uint8_t *)ospf_hdr + sizeof(ospf_header) + 4;
	
	// *((uint32_t *)((uint8_t *)ospf_hdr + sizeof(struct ospf_header))) = ntohl(nbr->num_lsr);
	lsu->num_of_lsa = ntohl(nbr->num_lsr);

	for(int i = 0; i < nbr->num_lsr; i++){
		for(int j = 0; j < a->num_lsa; j++){
			if(nbr->lsrs[i].ls_type == htonl(a->lsas[j]->ls_type) &&
				nbr->lsrs[i].link_state_id == a->lsas[j]->link_state_id &&
				nbr->lsrs[i].adv_router == a->lsas[j]->adv_router){
				size_t len = htons(a->lsas[j]->length);
				memcpy(lsa_begin, a->lsas[j], len);
				lsa_begin += len;
				break;
			}
		}
	}
	ospf_hdr->type = MSG_TYPE_LINK_STATE_UPDATE;
	ospf_hdr->pktlen = htons(lsa_begin - (uint8_t *)ospf_hdr);
}
