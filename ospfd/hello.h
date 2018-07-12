#ifndef _HELLO_H
#define _HELLO_H

#include "interface.h"
#include "shared.h"

/* 9.5. Sending Hello packets */
/* Hello packets are sent out each functioning router interface.
   They are used to discover and maintain neighbor
   relationships. On broadcast and NBMA networks, Hello Packets
   are also used to elect the Designated Router and Backup
   Designated Router. */

void encapsulate_hello_pkt(const interface_data *iface, ospf_header *ospf_hdr);

void process_hello_pkt(interface_data *iface, neighbor *nbr, const ospf_header *ospf_hdr, in_addr_t src);

#endif
