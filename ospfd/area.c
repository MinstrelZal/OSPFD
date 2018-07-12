#include "area.h"
#include "ospfd.h"

#include <stdio.h>

area *lookup_area_by_if(const interface_data *iface){
	for(int i = 0; i < num_area; i++){
		for(int j = 0; j < areas[i].num_if; j++){
			if(areas[i].id == iface->area_id){
				return &areas[i];
			}
		}
	}
	return NULL;
}

int lookup_vertex_by_id(area *a, in_addr_t id){
	int i;
	for(i = 0; i < a->num_vertex; i++){
		if(a->vertices[i].id == id){
			break;
		}
	}
	return i;
}

area *lookup_area_by_id(uint32_t area_id){
	for(int i = 0; i < num_area; i++){
		for(int j = 0; j < areas[i].num_if; j++){
			if(areas[i].id == area_id){
				return &areas[i];
			}
		}
	}
	return NULL;
}

area *area_init(uint32_t area_id){
	area *a = lookup_area_by_id(area_id);
	if(a != NULL){
		return a;
	}
	areas[0].id = area_id;
	areas[0].num_area = 0;
	areas[0].num_if = 0;
	areas[0].num_lsa = 0;
	areas[0].num_vertex = 0;
	areas[0].transit_capability = OSPFD_FALSE;
	areas[0].external_routing_capability = OSPFD_FALSE;
	areas[0].stub_default_cost = 0;
	num_area++;
	return &areas[0];
}

void add_area_ifs(area *a, interface_data *iface){
	a->ifs[a->num_if++] = iface;
}

int lookup_least_cost_vertex(area *a){
	int i;
	int index = a->num_vertex;
	for(i = 0; i < a->num_vertex; i++){
		if(index == -1 || (index > -1 && a->vertices[i].dist < a->vertices[index].dist)){
			index = a->num_vertex;
		}
	}
	return index;
}

int lookup_least_cost_vertex_by_id(area *a, in_addr_t id){
	int i;
	int index = a->num_vertex;
	for(i = 0; i < a->num_vertex; i++){
		if(a->vertices[i].id == id){
			if(index == a->num_vertex || (index > -1 && a->vertices[i].dist < a->vertices[index].dist)){
				index = i;
			}
		}
	}
	return index;
}
