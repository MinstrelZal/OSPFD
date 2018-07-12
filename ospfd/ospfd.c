#include "ospfd.h"
#include "network.h"
#include "lsa.h"
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#define NIPQUAD_FMT "%u.%u.%u.%u"
#define NIPQUAD(addr) \
 ((unsigned char *)&addr)[0],  ((unsigned char *)&addr)[1],  ((unsigned char *)&addr)[2],  ((unsigned char *)&addr)[3]

const char *ospf_type_name[] = {"Unknown", "Hello", "DD", "LSR", "LSU", "LSAck"};

const char *neighbor_state_str[] = {"Down", "Attempt", "Init", "2-Way", "ExStart", "Exchange", "Loading", "Full"};

const char *neighbor_event_str[] = {"HelloReceived", "Start", "2-WayReceived", "NegotiationDone", "ExchangeDone",
                                    "BadLSReq", "LoadingDone", "AdjOK" , "AdjNO", "SeqNumberMismatch", "1-Way", 
                                    "KillNbr", "InactivityTimer", "LLDown"};

int sock;

int num_area;
area areas[NUM_AREA];

int num_if;
interface_data ifs[NUM_INTERFACE];

in_addr_t my_router_id;

ospf_lsa_header *my_router_lsa;

int num_route;
route routing_table[NUM_ROUTE];

int old_num_route;
route old_routing_table[NUM_ROUTE];

int RFC1583Compatibility;

void global_value_init(){
	num_area = 0;
	num_if = 0;
	my_router_id = 0;
	my_router_lsa = NULL;
	num_route = 0;
	old_num_route = 0;
	RFC1583Compatibility = ENABLED;
}

void set_my_router_id(){
	puts("set router id for my router:");
	scanf("%u", &my_router_id);
	/*
	int index = -1;
	for(int i = 0; i < num_if; i++){
		if(index == -1 || (index > -1 && ifs[i].ip > ifs[index].ip)){
			index = i;
		}
	}
	my_router_id = ifs[index].ip;
	*/
}

void print_global_info(){
    printf("\n------------------------------------------\n");
    printf("---------------Router Info----------------\n");
    printf("Area num: %d\n", num_area);
    printf("Interface num: %d\n", num_if);
    printf("Router ID: " NIPQUAD_FMT "\n", NIPQUAD(my_router_id));
    printf("------------------------------------------\n\n");
}

int main(void){
	int ret;
	int area_id;
	pthread_t t_recv, t_send;

    network_init();

	global_value_init();

	ret = interface_init();
	if(ret == FAILURE){
		printf("Interface initialize failed.\n");
	}
	printf("%d interfaces is on.\n", num_if);
	for(int i = 0; i < num_if; i++){
		printf("%d: %s\n", i, ifs[i].if_name);
		puts("set area id for this interface:");
		scanf("%u", &area_id);
		area *a = area_init(area_id);
		add_area_ifs(a, ifs + i);
		set_interface_area(&ifs[i], area_id);
	}

	/* set my router id */
	set_my_router_id();

    print_global_info();

    pthread_create(&t_recv, NULL, recv_and_process, NULL);
	pthread_create(&t_send, NULL, encapsulate_and_send, NULL);
    /* main loop */
    while(1){
    	/* step 1: originate self LSA */
    	for(int i = 0; i < num_area; i++){
			my_router_lsa = originate_router_lsa(&areas[i]);
		}
		invalidated_old_routing_table();
		update_routing_table();
		sleep(5);
    }
    pthread_join(t_recv, NULL);
	pthread_join(t_send, NULL);

	return 0;
}
