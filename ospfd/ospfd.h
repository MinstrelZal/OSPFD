#ifndef _OSPFD_H
#define _OSPFD_H

#include <unistd.h>

#include "route.h"
#include "area.h"
#include "shared.h"

extern const char *ospf_type_name[];

/* name string of event and state */
extern const char *neighbor_event_str[];
extern const char *neighbor_state_str[];

extern int sock;
extern int num_area;
extern area areas[];
extern int num_if;
extern interface_data ifs[];
extern in_addr_t my_router_id;
extern ospf_lsa_header *my_router_lsa;
extern int num_route;
extern route routing_table[];
extern int old_num_route;
extern route old_routing_table[];
extern int RFC1583Compatibility;

#endif
