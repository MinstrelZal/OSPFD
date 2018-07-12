#include <net/ethernet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "interface.h"
#include "ospfd.h"
#include "hello.h"
#include "dd.h"
#include "lsr.h"
#include "lsu.h"
#include "lsack.h"
#include "lsa.h"

void network_init(){
	puts("sudo echo 1 > /proc/sys/net/ipv4/ip_forward");
	system("sudo echo 1 > /proc/sys/net/ipv4/ip_forward");
}

uint16_t cksum(const uint16_t *data, size_t len){
	uint32_t sum = 0;
	while (len > 1) {
		sum += *data++;
		len -= 2;
	}
	/* mop up an odd byte, if necessary */
	if (len) sum += *(uint8_t *)data;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

interface_data *recv_ospf(int sock, uint8_t buf[], int size, in_addr_t *src){
	/* ip header */
	struct iphdr *ip_hdr;
	/* ospf header */
	ospf_header *ospf_hdr;

	int count = 0;

	while(1){
		if(recvfrom(sock, buf, size, 0, NULL, NULL) < sizeof(struct iphdr)){
			continue;
		}
		ip_hdr = (struct iphdr *)buf;
		/* source ip address */
		*src = ip_hdr->saddr;

		if(ip_hdr->protocol == IPPROTO_OSPF){
			ospf_hdr = (ospf_header *)(buf + sizeof(struct iphdr));
			memset(ospf_hdr->u.auth_data, 0, sizeof(ospf_hdr->u.auth_data));

			if(cksum((uint16_t *)ospf_hdr, ntohs(ospf_hdr->pktlen))){
				continue;
			}
			printf("recv %s packet from %s\n", ospf_type_name[ospf_hdr->type], inet_ntoa((struct in_addr){*src}));

			for(int i = 0; i < num_if; i++){
				if((ip_hdr->saddr & ifs[i].network_mask) == (ifs[i].ip & ifs[i].network_mask)){
					/* return the interface */
					return ifs + i;
			        }
            }
		}
	}
}

void send_ospf(const interface_data *iface, struct iphdr *ip_hdr, in_addr_t dst){
	static int id;
	struct sockaddr_in addr;
	ospf_header * ospf_hdr = (ospf_header *)((uint8_t *)ip_hdr + sizeof(struct iphdr));

	/* fill out the ospf header */
	ospf_hdr->version = OSPFV2;
	ospf_hdr->router_id = my_router_id;
	ospf_hdr->area_id = iface->area_id;
	ospf_hdr->checksum = 0x0;
	ospf_hdr->autype = AUTH_TYPE_NULL;
	memset(ospf_hdr->u.auth_data, 0, sizeof(ospf_hdr->u.auth_data));
	ospf_hdr->checksum = cksum((uint16_t *)ospf_hdr, ntohs(ospf_hdr->pktlen));

	/* fill out the ip header */
	ip_hdr->ihl = COMMON_IPHDR_LEN;
	ip_hdr->version= IPV4;
	ip_hdr->tos = DEFAULT_OSPF_TOS;
	ip_hdr->tot_len = htons(sizeof(struct iphdr) + ospf_hdr->pktlen);
	ip_hdr->id = htons(id++);
	ip_hdr->frag_off = 0x0;
	ip_hdr->ttl = DEFAULT_OSPF_TTL;
	ip_hdr->protocol = IPPROTO_OSPF;
	ip_hdr->saddr = iface->ip;
	ip_hdr->daddr = dst;
	ip_hdr->check = 0x0;
	ip_hdr->check = cksum((uint16_t*)ip_hdr, sizeof(struct iphdr));

	/* send the packet */
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = dst;
	sendto(iface->sock, ospf_hdr, ntohs(ospf_hdr->pktlen), 0, (struct sockaddr *)&addr, sizeof(addr));
	printf("send %s packet to %s\n", ospf_type_name[ospf_hdr->type], inet_ntoa((struct in_addr){dst}));
}

void *recv_and_process(){
	uint8_t buf[BUFFER_SIZE];
	interface_data *iface;
	ospf_header *ospf_hdr;
	neighbor *nbr;
    area *a;
	in_addr_t src;
	while(1){
		iface = recv_ospf(sock, buf, BUFFER_SIZE, &src);
		if(iface == NULL){
			continue ;
		}
		a = lookup_area_by_if(iface);
		if(a == NULL){
			// printf("Recv Error: Can not find the area.\n");
			continue ;
		}

		ospf_hdr = (ospf_header *)(buf + sizeof(struct iphdr));

		/* check if the packet is from myself */
		if(ospf_hdr->router_id == my_router_id){
			continue ;
		}

		/* find the neighbor who send the packet */
		for(nbr = iface->neighbors; nbr != NULL; nbr = nbr->next){
			if(ospf_hdr->router_id == nbr->neighbor_id){
				break;
			}
		}

		/* process packet */
		switch(ospf_hdr->type){
			case MSG_TYPE_HELLO:
			    process_hello_pkt(iface, nbr, ospf_hdr, src);
			    break;
			case MSG_TYPE_DATABASE_DESCRIPTION:
			    process_dd_pkt(iface, nbr, ospf_hdr);
			    break;
			case MSG_TYPE_LINK_STATE_REQUEST:
			    process_lsr_pkt(iface, nbr, ospf_hdr);
			    break;
			case MSG_TYPE_LINK_STATE_UPDATE:
			    process_lsu_pkt(a, nbr, ospf_hdr);
			    break;
			case MSG_TYPE_LINK_STATE_ACK:
			    process_lsack_pkt(nbr, ospf_hdr);
			    break;
			default:
			    break;
		}
	}
}

/* flood link state */
void flood(){
	uint8_t buf[BUFFER_SIZE];
	for(int i = 0; i < num_area; i++){
		my_router_lsa = originate_router_lsa(&areas[i]);
		if(my_router_lsa != NULL){
			encapsulate_self_lsa(my_router_lsa, (ospf_header *)(buf + sizeof(struct iphdr)));
			for(int j = 0; j < areas[i].num_if; j++){
				for(neighbor *nbr = areas[i].ifs[i]->neighbors; nbr != NULL; nbr = nbr->next){
					if(nbr->state == NEIGHBOR_STATE_FULL){
						send_ospf(areas[i].ifs[i], (struct iphdr *)buf, nbr->neighbor_ip);
					}
				}
			}
			my_router_lsa = NULL;
		}
	}
	// printf("try flood\n");
}

void *encapsulate_and_send(){
	uint8_t buf[BUFFER_SIZE];
	while(1){
		flood();
		for(int i = 0; i < num_if; i++){
			area *a = lookup_area_by_if(ifs + i);
			if(a == NULL){
				// printf("Send Error: Can not find area.\n");
				continue ;
			}
			/* send hello packet every time hello timer fires */
			if(ifs[i].hello_timer == 0){
				encapsulate_hello_pkt(ifs + i, (ospf_header *)(buf + sizeof(struct iphdr)));
				send_ospf(ifs + i, (struct iphdr *)buf, inet_addr(MCAST_ALL_SPF_ROUTERS));
			}
	        // printf("try hello\n");

			/* update timers */
			ifs[i].hello_timer += 1;
			ifs[i].rxmt_timer += 1;

			/* remove out-of-date neighbors */
			neighbor **p = &ifs[i].neighbors;
			for(neighbor *q = *p; q; q = *p){
				q->inactivity_timer += 1;
				if(q->inactivity_timer >= ifs[i].router_dead_interval){
					ifs[i].num_neighbor -= 1;
					*p = q->next;
					free(q);
				}
				else{
					p = &q->next;
				}
			}
			// printf("delete neighbors out of date\n");
			
			/* send hello packet every time hello timer fires */
			if(ifs[i].hello_timer >= ifs[i].hello_interval){
				ifs[i].hello_timer = 0;
			}
			for(neighbor *nbr = ifs[i].neighbors; nbr; nbr = nbr->next){
				/* send dd packet */
				if(nbr->state == NEIGHBOR_STATE_EXCHANGE){
					if((nbr->master_slave_relationship == DD_MASTER && nbr->last_dd_seqnum == nbr->dd_seqnum - 1) || 
						(nbr->master_slave_relationship == DD_SLAVE && nbr->last_dd_seqnum == nbr->dd_seqnum)){
						encapsulate_dd_pkt(ifs + i, nbr, (ospf_header *)(buf + sizeof(struct iphdr)));
					    send_ospf(ifs + i, (struct iphdr *)buf, nbr->neighbor_ip);
					}
					else if(nbr->master_slave_relationship == DD_SLAVE && nbr->last_dd_seqnum != nbr->dd_seqnum){
						send_ospf(ifs + i, (struct iphdr *)nbr->pre_dd_pkt, nbr->neighbor_ip);
					}
				}
				// printf("try dd 1\n");
				if(ifs[i].rxmt_timer >= ifs[i].rxmt_interval){
					/* send dd packet */
					if(nbr->state == NEIGHBOR_STATE_EX_START || nbr->state == NEIGHBOR_STATE_EXCHANGE){
						encapsulate_dd_pkt(ifs + i, nbr, (ospf_header *)(buf + sizeof(struct iphdr)));
						send_ospf(ifs + i, (struct iphdr *)buf, nbr->neighbor_ip);
						if(nbr->master_slave_relationship == DD_SLAVE && nbr->more == 0){
							add_neighbor_event(ifs + i, nbr, NEIGHBOR_EV_EXCHANGE_DONE);
						}
					}
					else if(nbr->master_slave_relationship == DD_SLAVE && nbr->last_dd_seqnum != nbr->dd_seqnum){
						send_ospf(ifs + i, (struct iphdr *)nbr->pre_dd_pkt, nbr->neighbor_ip);
					}
					// printf("try dd 2\n");
					/* send lsr packet */
					if(nbr->state == NEIGHBOR_STATE_EXCHANGE || nbr->state == NEIGHBOR_STATE_LOADING){
						if(nbr->num_lsa_hdr > 0){
							encapsulate_lsr_pkt(nbr, (ospf_header *)(buf + sizeof(struct iphdr)));
							send_ospf(ifs + i, (struct iphdr *)buf, nbr->neighbor_ip);
						}				
					}
					// printf("try lsr\n");
					/* send lsu packet for request */
					if(nbr->num_lsr > 0 && nbr->state >= NEIGHBOR_STATE_EXCHANGE){
						encapsulate_lsu_pkt(a, nbr, (ospf_header *)(buf + sizeof(struct iphdr)));
						send_ospf(ifs + i, (struct iphdr *)buf, nbr->neighbor_ip);
					}
					// printf("try lsu\n");
					
				}
				/* send ls ack packet */
				if(nbr->num_lsack > 0){
					encapsulate_lsack_pkt(nbr, (ospf_header *)(buf + sizeof(struct iphdr)));
					send_ospf(ifs + i, (struct iphdr *)buf, nbr->neighbor_ip);
				}
				// printf("try lsack\n");
			}
			if(ifs[i].rxmt_timer >= ifs[i].rxmt_interval){
				ifs[i].rxmt_timer = 0;
			}
		}
		sleep(1); 
	}
	return NULL;
}
