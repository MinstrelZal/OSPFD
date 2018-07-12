#include "interface.h"
#include "ospfd.h"
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>

int interface_init(){
	int ret;
	/* request for interface configuration */
	struct ifconf ifc;
	/* save interface configuration */
	struct ifreq ifrs[NUM_INTERFACE];

	/* create socket */
	sock = socket(AF_PACKET, SOCK_DGRAM, htons(ETHERTYPE_IP));

	ifc.ifc_len = sizeof(ifrs);
	ifc.ifc_req = ifrs;
	// printf("ifc len:%d\n", ifc.ifc_len); 

	/* get interface list */
	ret = ioctl(sock, SIOCGIFCONF, &ifc);
	if(ret == FAILURE){
		printf("Error: Can not get interface list.\n");
		return FAILURE;
	}

	// printf("ifc len:%d\n", ifc.ifc_len);
	int num = ifc.ifc_len / sizeof(struct ifreq);
	num_if = 0;
	// printf("interface num: %d\n", num);

	for(int i = 0; i < num; i++){
		/* except loop-back */
		if(strcmp(ifrs[i].ifr_name, "lo")){
			/* interface name */
			strcpy(ifs[num_if].if_name, ifrs[i].ifr_name);

			/* ip address */
			ret = ioctl(sock, SIOCGIFADDR, ifrs + i);
			if(ret == FAILURE){
				printf("Error: Can not get ip address of interface %s.\n", ifrs[i].ifr_name);
				return FAILURE;
			}
			ifs[num_if].ip = ((struct sockaddr_in *)&ifrs[i].ifr_addr)->sin_addr.s_addr;

			/* subnet mask */
			ret = ioctl(sock, SIOCGIFNETMASK, ifrs + i);
			if(ret == FAILURE){
				printf("Error: Can not get netmask of interface %s.\n", ifrs[i].ifr_name);
				return FAILURE;
			}
			ifs[num_if].network_mask = ((struct sockaddr_in *)&ifrs[i].ifr_netmask)->sin_addr.s_addr;

			/* turn on promisc mode */
			ret = ioctl(sock, SIOCGIFFLAGS, ifrs + i);
			if(ret == FAILURE){
				printf("Error: Can not get flags of interface %s.\n", ifrs[i].ifr_name);
				return FAILURE;
			}
			ifrs[i].ifr_flags |= IFF_PROMISC;
			ret = ioctl(sock, SIOCSIFFLAGS, ifrs + i);
			if(ret == FAILURE){
				printf("Error: Can not set flags of interface %s.\n", ifrs[i].ifr_name);
				return FAILURE;
			}

			/* bind socket to this interface */
			ifs[num_if].sock = socket(AF_INET, SOCK_RAW, IPPROTO_OSPF);
			setsockopt(ifs[num_if].sock, SOL_SOCKET, SO_BINDTODEVICE, ifrs + i, sizeof(struct ifreq));

		    ifs[num_if].state = 0;
		    ifs[num_if].hello_interval = OSPF_DEFAULT_HELLO_INTERVAL;
		    ifs[num_if].router_dead_interval = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL;
		    ifs[num_if].inf_trans_delay = 1;
		    ifs[num_if].router_priority = OSPF_DEFAULT_ROUTER_PRIORITY;
		    ifs[num_if].hello_timer = 0;
		    ifs[num_if].wait_timer = 0;
		    ifs[num_if].num_neighbor = 0;
		    ifs[num_if].neighbors = NULL;
		    ifs[num_if].d_router = my_router_id;
		    ifs[num_if].bd_router = 0;
		    ifs[num_if].rxmt_interval =  OSPF_DEFAULT_RXMT_INTERVAL;
            ifs[num_if].rxmt_timer = 0;
		    ifs[num_if].cost = htons(5);
		    num_if++;
		}
	}

	return SUCCESS;
}

void set_interface_area(interface_data *iface, uint32_t area_id){
	iface->area_id = htonl(area_id);
}

const char *lookup_ifname_by_ip(const area *a, in_addr_t ip){
	for(int i = 0; i < a->num_if; i++){
		if((a->ifs[i]->ip & a->ifs[i]->network_mask) == (ip & a->ifs[i]->network_mask)){
			return a->ifs[i]->if_name;
		}
	}
	return 0;
}
