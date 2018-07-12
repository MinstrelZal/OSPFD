#include "route.h"
#include "ospfd.h"
#include "spf.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void del_route_from_host(route *my_route){
	static char cmd[128], ip[32], mask[32];
	strcpy(ip, inet_ntoa((struct in_addr){my_route->dest_id}));
	strcpy(mask, inet_ntoa((struct in_addr){my_route->addr_mask}));
	sprintf(cmd, "route del -net %s netmask %s", ip, mask);
	system(cmd);
	printf("%s\n", cmd);
}

void add_route_to_host(route *my_route){
	static char cmd[128], ip[32], mask[32], next[32];
	strcpy(ip, inet_ntoa((struct in_addr){my_route->dest_id}));
	strcpy(mask, inet_ntoa((struct in_addr){my_route->addr_mask}));
	strcpy(next, inet_ntoa((struct in_addr){my_route->next_hop}));
	sprintf(cmd, "route add -net %s netmask %s gw %s metric %hu dev %s", ip, mask, next, my_route->cost, my_route->iface);
	system(cmd);
	printf("%s\n", cmd);
}

void invalidated_old_routing_table(){
	for(old_num_route = 0; old_num_route < num_route; old_num_route++){
		old_routing_table[old_num_route] = routing_table[old_num_route];
		del_route_from_host(&old_routing_table[old_num_route]);
	}
	num_route = 0;
	for(int i = 0; i < num_area; i++){
		areas[i].num_vertex = 0;
	}
}

int lookup_route_by_dst(in_addr_t dest_id){
	for(int i = 0; i < num_route; i++){
		if(routing_table[i].dest_id == dest_id){
			return i;
		}
	}
	return -1;
}

int lookup_route_by_least_cost(){
	int index = -1;
	for(int i = 0; i < num_route; i++){
		if(index == -1){
			index = i;
		}
		else if(index > -1){
			if(routing_table[i].cost < routing_table[index].cost || 
				(routing_table[i].cost == routing_table[index].cost && 
				routing_table[i].area_id > routing_table[index].area_id)){
				index = i;
			}
		}
	}
	return index;
}

void update_routing_table(){
	for(int i = 0; i < num_area; i++){
		printf("\n-------------------------Routing table for Area %d---------------------------\n", areas[i].id);
                printf("----------------------------------------------------------------------------\n");
		shortest_path_tree(&areas[i]);
                printf("Destination/Mask\t\tcost\tnext hop\n");
		for(int j = 0; j < areas[i].num_vertex; j++){
                        printf("%s/%s\t\t%d\t%s\n", inet_ntoa((struct in_addr){areas[i].vertices[j].id}), inet_ntoa((struct in_addr){areas[i].vertices[j].network_mask}), 
                                areas[i].vertices[j].dist, inet_ntoa((struct in_addr){areas[i].vertices[j].next_hop}));
		}
                printf("----------------------------------------------------------------------------\n\n");
		for(int j = 0; j < areas[i].num_vertex; j++){
			if(areas[i].vertices[j].network_mask && areas[i].vertices[j].dist < INF){
				int route_index = lookup_route_by_dst(areas[i].vertices[j].id);
				if(route_index == -1){
					routing_table[num_route].addr_mask = areas[i].vertices[j].network_mask;
					routing_table[num_route].dest_id = areas[i].vertices[j].id;
					routing_table[num_route].next_hop = lookup_neighbor_ip_by_id(&areas[i], areas[i].vertices[j].next_hop);
					routing_table[num_route].lsa = areas[i].vertices[j].lsa;
					if(routing_table[num_route].next_hop){
						routing_table[num_route].iface = lookup_ifname_by_ip(&areas[i], routing_table[num_route].next_hop);
					}
					else{
						routing_table[num_route].iface = lookup_ifname_by_ip(&areas[i], routing_table[num_route].dest_id);
					}
					routing_table[num_route].cost = areas[i].vertices[j].dist;
					add_route_to_host(&routing_table[num_route]);
					num_route++;
				}
				else{
					if(routing_table[route_index].cost > areas[i].vertices[j].dist){
						del_route_from_host(&routing_table[route_index]);
						routing_table[route_index].addr_mask = areas[i].vertices[j].network_mask;
						routing_table[route_index].dest_id = areas[i].vertices[j].id;
						routing_table[route_index].next_hop = lookup_neighbor_ip_by_id(&areas[i], areas[i].vertices[j].next_hop);
						routing_table[route_index].lsa = areas[i].vertices[j].lsa;
						if(routing_table[route_index].next_hop){
							routing_table[route_index].iface = lookup_ifname_by_ip(&areas[i], routing_table[route_index].next_hop);
						}
						else{
							routing_table[route_index].iface = lookup_ifname_by_ip(&areas[i], routing_table[route_index].dest_id);
						}
						routing_table[route_index].cost = areas[i].vertices[j].dist;
						add_route_to_host(&routing_table[route_index]);
					}
				}
			}
		}
	}
}
