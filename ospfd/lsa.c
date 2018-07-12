#include "lsa.h"
#include "ospfd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
 * This function is from GNU Zebra.
 * Copyright (C) 1999, 2000 Toshiaki Takada
 * Fletcher Checksum -- Refer to RFC1008.
 */

#define MODX						4102
#define LSA_CHECKSUM_OFFSET			15

uint16_t fletcher16(const uint8_t *data, size_t len){
    const uint8_t *sp, *ep, *p, *q;
	int c0 = 0, c1 = 0;
	int x, y;

	for (sp = data, ep = sp + len; sp < ep; sp = q){
		q = sp + MODX;
		if (q > ep) q = ep;
		for (p = sp; p < q; p++) {
			c0 += *p;
			c1 += c0;
		}
		c0 %= 255;
		c1 %= 255;
	}
	x = ((len - LSA_CHECKSUM_OFFSET) * c0 - c1) % 255;
	if (x <= 0) x += 255;
	y = 510 - c0 - x;
	if (y > 255) y -= 255;
	return (x << 8) + y;
}

int lsa_hdr_eql(const ospf_lsa_header *a, const ospf_lsa_header *b){
	return a->ls_type == b->ls_type && a->link_state_id == b->link_state_id && a->adv_router == b->adv_router;
}

ospf_lsa_header *lookup_lsa(const area *a, const ospf_lsa_header *lsa_hdr){
	for(int i = 0; i < a->num_lsa; i++){
		if(lsa_hdr_eql(a->lsas[i], lsa_hdr)){
			return a->lsas[i];
		}
	}
	return NULL;
}

/* return value < 0: b is newer, > 0: a is newer , = 0: the same */
int cmp_lsa_hdr(const ospf_lsa_header *a, const ospf_lsa_header *b){
	return -1;
	if(a->ls_seqnum != b->ls_seqnum){
		return ntohl(a->ls_seqnum) - ntohl(b->ls_seqnum);
	}
	else if(a->ls_chksum != b->ls_chksum){
		return ntohs(a->ls_chksum) - ntohs(b->ls_chksum);
	}
	else if(ntohs(a->ls_age) == MAX_AGE){
		return +1;
	}
	else if(ntohs(b->ls_age) == MAX_AGE){
		return -1;
	}
	else if(abs(ntohs(a->ls_age) - ntohs(b->ls_age)) < MAX_AGE_DIFF){
		return ntohs(b->ls_age) - ntohs(a->ls_age);
	}
	return 0;
}

void add_lsa_hdr(neighbor *nbr, const ospf_lsa_header *lsa_hdr){
	for(int i = 0; i < nbr->num_lsa_hdr; i++){
		if(lsa_hdr_eql(&nbr->lsa_hdrs[i], lsa_hdr)){
			if(cmp_lsa_hdr(&nbr->lsa_hdrs[i], lsa_hdr) < 0){
				nbr->lsa_hdrs[i] = *lsa_hdr;
			}
			return;
		}
	}
	nbr->lsa_hdrs[nbr->num_lsa_hdr++] = *lsa_hdr;
}

ospf_lsa_header *install_lsa(area *a, const ospf_lsa_header *lsa_hdr){
	int i;
	for(i = 0; i < a->num_lsa; i++){
		if(lsa_hdr_eql(a->lsas[i], lsa_hdr)){
			if(cmp_lsa_hdr(a->lsas[i], lsa_hdr) < 0){
				break;
			}
			else{
				return NULL;
			}
		}
	}
	size_t len = ntohs(lsa_hdr->length);
	a->lsas[i] = realloc(a->lsas[i], len);
	memcpy(a->lsas[i], lsa_hdr, len);
	if(i == a->num_lsa){
		a->num_lsa += 1;
	}
	// a->num_lsa += (i == a->num_lsa);
	return a->lsas[i];
}

int32_t get_ls_seqnum(){
	static int32_t ls_seqnum = LS_INIT_SEQ_NUM;
	return htonl(ls_seqnum++);
}

/* To further describe the process of building the list of link
   descriptions, suppose a router wishes to build a router-LSA
   for Area A. The router examines its collection of interface
   data structures. For each interface, the following steps
   are taken:

   o If the attached network does not belong to Area A, no
   links are added to the LSA, and the next interface
   should be examined.

   o If the state of the interface is Down, no links are
   added.

   o If the state of the interface is Loopback, add a Type 3
   link (stub network) as long as this is not an interface
   to an unnumbered point-to-point network. The Link ID
   should be set to the IP interface address, the Link Data
   set to the mask 0xffffffff (indicating a host route),
   and the cost set to 0.

   o Otherwise, the link descriptions added to the router-LSA
   depend on the OSPF interface type. Link descriptions
   used for point-to-point interfaces are specified in
   Section 12.4.1.1, for virtual links in Section 12.4.1.2,
   for broadcast and NBMA interfaces in 12.4.1.3, and for
   Point-to-MultiPoint interfaces in 12.4.1.4.

   After consideration of all the router interfaces, host links
   are added to the router-LSA by examining the list of
   attached hosts belonging to Area A. A host route is
   represented as a Type 3 link (stub network) whose Link ID is
   the host’s IP address, Link Data is the mask of all ones
   (0xffffffff), and cost the host’s configured cost (see
   Section C.7). */

/* 12.4.1.2. Describing broadcast and NBMA interfaces */
/* For operational broadcast and NBMA interfaces, a single
   link description is added to the router-LSA as follows:

   o If the state of the interface is Waiting, add a Type
   3 link (stub network) with Link ID set to the IP
   network number of the attached network, Link Data
   set to the attached network’s address mask, and cost
   equal to the interface’s configured output cost.

   o Else, there has been a Designated Router elected for
   the attached network. If the router is fully
   adjacent to the Designated Router, or if the router
   itself is Designated Router and is fully adjacent to
   at least one other router, add a single Type 2 link
   (transit network) with Link ID set to the IP
   interface address of the attached network’s
   Designated Router (which may be the router itself),
   Link Data set to the router’s own IP interface
   address, and cost equal to the interface’s
   configured output cost. Otherwise, add a link as if
   the interface state were Waiting (see above). */

ospf_lsa_header *originate_router_lsa(area *a){
	uint8_t buff[BUFFER_SIZE];
	ospf_lsa_header *lsa_hdr = (ospf_lsa_header *)buff;
	router_lsa *rtr_lsa = (router_lsa *)((uint8_t *)lsa_hdr + sizeof(ospf_lsa_header));
	mylink *lnk = rtr_lsa->links;

	/* encapsulate LSA header */
	lsa_hdr->ls_age = 0;
	lsa_hdr->options = OSPF_OPTIONS;
	lsa_hdr->ls_type = OSPF_ROUTER_LSA;
	lsa_hdr->link_state_id = my_router_id;
	lsa_hdr->adv_router = my_router_id;
	lsa_hdr->ls_seqnum = get_ls_seqnum();
	
    /* encapsulate body of Router_LSA */
	rtr_lsa->flags = 0x00;
	rtr_lsa->padding = 0x00;
	for(int i = 0; i < a->num_if; i++){
		if(a->ifs[i]->num_neighbor == 0){
			/* StubNet */
			lnk->id = a->ifs[i]->ip & a->ifs[i]->network_mask;
			lnk->data = a->ifs[i]->network_mask;
			lnk->type = RTR_LSA_STUB;
			lnk->num_diff_tos = 0;
			lnk->metric = a->ifs[i]->cost;
			lnk++;
		}
		else{
			if(a->ifs[i]->state == 1){
				/* TransNet */
				lnk->id = a->ifs[i]->d_router;
				lnk->data = a->ifs[i]->ip;
				lnk->type = RTR_LSA_TRANSIT;
				lnk->num_diff_tos = 0;
				lnk->metric = a->ifs[i]->cost;
				lnk++;
			}
			else{
			    /* StubNet */
			    lnk->id = a->ifs[i]->ip & a->ifs[i]->network_mask;
			    lnk->data = a->ifs[i]->network_mask;
			    lnk->type = RTR_LSA_STUB;
			    lnk->num_diff_tos = 0;
			    lnk->metric = a->ifs[i]->cost;
			    lnk++;
			}
		}
	}
	rtr_lsa->num_link = htons((lnk - rtr_lsa->links));
	size_t len = sizeof(ospf_lsa_header) + sizeof(router_lsa) + (lnk - rtr_lsa->links) * sizeof(link);
	lsa_hdr->length = htons(len);
	lsa_hdr->ls_chksum = 0;
	lsa_hdr->ls_chksum = htons(fletcher16(buff + sizeof(lsa_hdr->ls_age),
		ntohs(lsa_hdr->length) - sizeof(lsa_hdr->ls_age)));
	return install_lsa(a, lsa_hdr);
}

void encapsulate_self_lsa(const ospf_lsa_header *lsa, ospf_header *ospf_hdr){
	ospf_lsu_pkt *lsu = (ospf_lsu_pkt *)((uint8_t *)ospf_hdr + sizeof(ospf_header));
	uint8_t *lsa_begin = (uint8_t *)ospf_hdr + sizeof(ospf_header) + sizeof(ospf_lsu_pkt);
	// *((uint32_t *)((uint8_t *)ospf_hdr + sizeof(struct ospf_header))) = ntohl(1);
	lsu->num_of_lsa = ntohl(1);
	size_t len = htons(lsa->length);
	memcpy(lsa_begin, lsa, len);
	lsa_begin += len;
	ospf_hdr->type = MSG_TYPE_LINK_STATE_UPDATE;
	ospf_hdr->pktlen = htons(lsa_begin - (uint8_t *)ospf_hdr);
}
