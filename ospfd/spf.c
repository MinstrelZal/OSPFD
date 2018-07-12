#include "spf.h"
#include "ospfd.h"
#include <stdio.h>

void dijkstra(area *a, int root){
	vertex *leaf = a->vertices + a->num_vertex;
	int use[NUM_VERTEX];
	int pre[NUM_VERTEX];
	a->vertices[root].dist = 0;
	pre[root] = root;
	while(1){
		int p = -1, q;
		/* find out the nearest unused node */
		for(int j = 0; j < a->num_vertex; j++){
			if (!use[j] && (p == -1 || a->vertices[p].dist > a->vertices[j].dist))
				p = j;
		}
		if(p == -1 || a->vertices[p].dist == INF){
			break;
		}
		use[p] = 1;
		/* update distance of the rest nodes */
		if(a->vertices[p].lsa->ls_type == OSPF_ROUTER_LSA){
			router_lsa *rtr_lsa = (router_lsa *)((uint8_t *)a->vertices[p].lsa + 
				sizeof(ospf_lsa_header));
			int j = ntohs(rtr_lsa->num_link);
			a->vertices[p].network_mask = 0;
			for(const mylink *lnk = rtr_lsa->links; j--; lnk++){
				int k = a->num_vertex;
				if(lnk->type == RTR_LSA_ROUTER || lnk->type == RTR_LSA_TRANSIT){
					k = lookup_vertex_by_id(a, lnk->id);
				}
				else{
					if(lnk->type == RTR_LSA_STUB){
						printf("leaf: %s\n", inet_ntoa((struct in_addr){lnk->id}));
						leaf->id = lnk->id;
						leaf->network_mask = lnk->data;
						leaf->next_hop = a->vertices[p].next_hop;
						leaf->dist = a->vertices[p].dist + ntohs(lnk->metric);
						leaf->lsa = a->vertices[p].lsa;
						pre[leaf - a->vertices] = p;
						leaf++;
				    }
				    continue;
				}
				if(k == a->num_vertex || use[k]){
					continue;
				}
				int det = ntohs(lnk->metric);
				if(a->vertices[k].dist > a->vertices[p].dist + det){
					a->vertices[k].dist = a->vertices[p].dist + det;
					pre[k] = p;
				}
			}
		} 
		else if(a->vertices[p].lsa->ls_type == OSPF_NETWORK_LSA){
			network_lsa *net_lsa = (network_lsa *)((uint8_t *)a->vertices[p].lsa + 
				sizeof(ospf_lsa_header));
			int j = (ntohs(a->vertices[p].lsa->length) - sizeof(ospf_lsa_header) - 
				sizeof(net_lsa->network_mask)) /sizeof(in_addr_t);
			a->vertices[p].network_mask = net_lsa->network_mask;
			for(const in_addr_t *rtr = net_lsa->attached_rtrs; j--; rtr++){
				int k = lookup_vertex_by_id(a, *rtr);
				if(k == a->num_vertex || use[k]){
					continue;
				}
				router_lsa *rtr_lsa = (router_lsa *)((uint8_t *)a->vertices[k].lsa + 
					sizeof(ospf_lsa_header));
				int num = ntohs(rtr_lsa->num_link);
				while(num--){
					if(rtr_lsa->links[num].type == RTR_LSA_TRANSIT &&
						rtr_lsa->links[num].id == a->vertices[p].id){
						break;
					}
					if(rtr_lsa->links[num].type == RTR_LSA_STUB && 
						rtr_lsa->links[num].id == (a->vertices[p].id & net_lsa->network_mask)){
						break;
					}
				}
				if(num < 0){
					continue;
				}
				int det = ntohs(rtr_lsa->links[num].metric);
				if(a->vertices[k].dist > a->vertices[p].dist + det){
					a->vertices[k].dist = a->vertices[p].dist + det;
					pre[k] = p;
				}
			}
		}
		/* calculate the next router id */
		for(q = p; q != root; q = pre[q]){
			if(!a->vertices[q].network_mask){
				a->vertices[p].next_hop = a->vertices[q].id;
			}
		}
	}
	a->num_vertex = leaf - a->vertices;
}


/* (2) The intra-area routes are calculated by building the shortest-
   path tree for each attached area. In particular, all routing
   table entries whose Destination Type is "area border router" are
   calculated in this step. This step is described in two parts.
   At first the tree is constructed by only considering those links
   between routers and transit networks. Then the stub networks
   are incorporated into the tree. During the area’s shortest-path
   tree calculation, the area’s TransitCapability is also
   calculated for later use in Step 4. */
void calculate_intra_routes(area *a){
	int root;
	for(int i = 0 ; i < a->num_lsa; i++){
		if(a->lsas[i]->ls_type == OSPF_ROUTER_LSA || a->lsas[i]->ls_type == OSPF_NETWORK_LSA){
			a->vertices[a->num_vertex].id = a->lsas[i]->link_state_id;
			a->vertices[a->num_vertex].lsa = a->lsas[i];
			a->vertices[a->num_vertex].dist = INF;
			if(a->vertices[a->num_vertex].id == my_router_id){
				root = a->num_vertex;
			}
			a->num_vertex++;
		}
	}
	dijkstra(a, root);	
}


/* (3) The inter-area routes are calculated, through examination of
   summary-LSAs. If the router is attached to multiple areas
   (i.e., it is an area border router), only backbone summary-LSAs
   are examined. */
void calculate_inter_routes(area *a){
	for(int i = 0 ; i < a->num_lsa; i++){
		if(a->lsas[i]->ls_type == OSPF_SUMMARY_LSA || a->lsas[i]->ls_type == OSPF_ASBR_SUMMARY_LSA){
			summary_lsa *slsa = (summary_lsa *)((uint8_t *)a->lsas[i] + 
				sizeof(ospf_lsa_header));
			if(lookup_vertex_by_id(a, a->lsas[i]->link_state_id) < a->num_vertex){
				continue;
			}
			int k = lookup_vertex_by_id(a, a->lsas[i]->adv_router);
			if(k == a->num_vertex || a->vertices[k].dist == INF){
				continue;
			}
			a->vertices[a->num_vertex].id = a->lsas[i]->link_state_id;
			a->vertices[a->num_vertex].network_mask = slsa->network_mask;
			a->vertices[a->num_vertex].next_hop = a->vertices[k].next_hop;
			a->vertices[a->num_vertex].dist = a->vertices[k].dist + ntohl(slsa->tos0metric >> 4 << 4);
			a->vertices[a->num_vertex].lsa = a->lsas[i];
			a->num_vertex++;
		}
	}
}

/* (4) In area border routers connecting to one or more transit areas
   (i.e, non-backbone areas whose TransitCapability is found to be
   TRUE), the transit areas’ summary-LSAs are examined to see
   whether better paths exist using the transit areas than were
   found in Steps 2-3 above. */

/* not support yet */
void recalculate_transit_area_routes(area *a){
	return ;
}

/* (5) Routes to external destinations are calculated, through
   examination of AS-external-LSAs. The locations of the AS
   boundary routers (which originate the AS-external-LSAs) have
   been determined in steps 2-4. */
/* only support a part */
void calculate_as_external_routes(area *a){
	for(int i = 0; i < a->num_lsa; i++){
		if(a->lsas[i]->ls_type == OSPF_AS_EXTERNAL_LSA){
			as_external_lsa *aelsa = (as_external_lsa *)((uint8_t *)a->lsas[i] + 
				sizeof(ospf_lsa_header));
			if((ntohl(aelsa->tos0.tos0metric) & 0x00ffffff) == LSINFINITY || a->lsas[i]->adv_router == my_router_id){
				continue;
			}
			int k = lookup_vertex_by_id(a, a->lsas[i]->adv_router);
			if(k == a->num_vertex){
				continue;
			}
			/* If the forwarding address is set to 0.0.0.0, packets should
               be sent to the ASBR itself. Among the multiple routing table
               entries for the ASBR, select the preferred entry as follows.
               If RFC1583Compatibility is set to "disabled", prune the set
               of routing table entries for the ASBR as described in
               Section 16.4.1. In any case, among the remaining routing
               table entries, select the routing table entry with the least
               cost; when there are multiple least cost routing table
               entries the entry whose associated area has the largest OSPF
               Area ID (when considered as an unsigned 32-bit integer) is
               chosen. */
			if(aelsa->tos0.forward_addr == 0){
				if(RFC1583Compatibility == DISABLED){

				}
				int t = lookup_least_cost_vertex_by_id(a, a->lsas[i]->adv_router);
				if(t == a->num_vertex){
					continue;
				}
				a->vertices[a->num_vertex].id = a->lsas[i]->adv_router;
			    a->vertices[a->num_vertex].network_mask = aelsa->network_mask;
			    a->vertices[a->num_vertex].next_hop = a->vertices[t].next_hop;
			    a->vertices[a->num_vertex].dist = a->vertices[t].dist + ntohl(aelsa->tos0.tos0metric >> 4 << 4);
			    a->vertices[a->num_vertex].lsa = a->lsas[i];
			    a->num_vertex++;
			}
			else{
				k = lookup_vertex_by_id(a, aelsa->tos0.forward_addr);
				if(k == a->num_vertex){
					continue;
				}

			}
		}
	}
}

void shortest_path_tree(area *a){
	/* (1) The present routing table is invalidated. The routing table is
       built again from scratch. The old routing table is saved so
       that changes in routing table entries can be identified. */
	a->num_vertex = 0;
	calculate_intra_routes(a);
	calculate_inter_routes(a);
	recalculate_transit_area_routes(a);
	calculate_as_external_routes(a);
}
