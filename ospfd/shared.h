#ifndef _SHARED_H
#define _SHARED_H

#define BUFFER_SIZE	2048

#define NUM_AREA 256
#define NUM_INTERFACE 256
#define NUM_ROUTE 256

#define IF_NAMESIZE 16

/* function return codes */
#define SUCCESS 0
#define FAILURE -1

/* true & false codes */
#define OSPFD_TRUE 1
#define OSPFD_FALSE 0

/* 4.3. Routing protocol packets */
#define	MSG_TYPE_HELLO                1 /* Discover and maintain  neighbors */
#define	MSG_TYPE_DATABASE_DESCRIPTION 2 /* Summarize database contents */
#define	MSG_TYPE_LINK_STATE_REQUEST   3 /* Database download */
#define	MSG_TYPE_LINK_STATE_UPDATE    4 /* Database update */
#define	MSG_TYPE_LINK_STATE_ACK       5 /* Flooding acknowledgment */

/* LSA type */
#define OSPF_ROUTER_LSA				1
#define OSPF_NETWORK_LSA			2
#define OSPF_SUMMARY_LSA			3
#define OSPF_ASBR_SUMMARY_LSA		4
#define OSPF_AS_EXTERNAL_LSA		5

/* Flags in Database Description packet */
/* +--------------------------------+
   | * | * | * | * | * | I | M | MS |
   +--------------------------------+ */
#define DD_FLAG_MS				0x01
#define DD_FLAG_M					0x02
#define DD_FLAG_I					0x04

/* Master/Slave relationship in Database Description packet */
#define DD_MASTER 1
#define DD_SLAVE  0

#define OSPF_AUTH_SIMPLE_SIZE 8u

#define LIST_MAX 256

#define MCAST_ALL_SPF_ROUTERS "224.0.0.5"
#define MCAST_ALL_DROUTERS    "224.0.0.6"

/* +------------------------------------+
   | * | * | DC | EA | N/P | MC | E | * |
   +------------------------------------+
            The Options field 

    E-bit - This bit describes the way AS-external-LSAs are flooded, as
    described in Sections 3.6, 9.5, 10.8 and 12.1.2 of this memo.

    MC-bit - This bit describes whether IP multicast datagrams are forwarded
    according to the specifications in [Ref18].

    N/P-bit - This bit describes the handling of Type-7 LSAs, as specified in
    [Ref19].

    EA-bit - This bit describes the router’s willingness to receive and
    forward External-Attributes-LSAs, as specified in [Ref20].

    DC-bit - This bit describes the router’s handling of demand circuits, as
    specified in [Ref21]. */
#define OSPF_OPTIONS 0x02 /* 00000010 */

#define	OSPF_DEFAULT_HELLO_INTERVAL 10
#define	OSPF_DEFAULT_ROUTER_PRIORITY 1
#define	OSPF_DEFAULT_ROUTER_DEAD_INTERVAL 40
#define OSPF_DEFAULT_RXMT_INTERVAL 5

/* MaxAge for LSA */
/* The value of MaxAge is set to 1 hour. */
#define MAX_AGE 3600
/* MaxAgeDiff for LSA */
/* The value of MaxAgeDiff is set to 15 minutes. */
#define MAX_AGE_DIFF 900

#define RTR_LSA_FLAGS_V 0x04
#define RTR_LSA_FLAGS_E 0x02
#define RTR_LSA_FLAGS_B 0x01

#define AS_EXT_RTR_FLAFS_E 0x80

#define LS_INIT_SEQ_NUM 0x80000001
#define LS_MAX_SEQ_NUM  0x7fffffff

#define RTR_LSA_ROUTER      1
#define RTR_LSA_TRANSIT     2
#define RTR_LSA_STUB        3
#define RTR_LSA_VIRTUAL     4

/* Maximum transmission unit (btye) */
#define DEFAULT_MTU 1500

/* for shortest path tree */
#define NUM_VERTEX 256
#define INF 0x7fff

/* for area address state */
#define ADVERTISE 1
#define NONADVERTISE 0

/* for neighbor state machine */
#define NEIGHBOR_SM_ENTRY_NUM 11

#define MAX_IF_PER_AREA 16

/* for route use */
#define DEST_ROUTER 1
#define DEST_NETWORK 2

#define ADDR_MASK_ALL_ONES 0xffffffff

#define ROUTE_PATH_TYPE_INTRA_AREA 1
#define ROUTE_PATH_TYPE_INTER_AREA 2
#define ROUTE_PATH_TYPE_ONE_EXTERNAL 3
#define ROUTE_PATH_TYPE_TWO_EXTERNAL 4

/* for network use */
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF 89
#endif

#define OSPFV2 2

#define MCAST_ALL_SPF_ROUTERS "224.0.0.5"
#define MCAST_ALL_DROUTERS    "224.0.0.6"

#define	AUTH_TYPE_NULL 0

#define COMMON_IPHDR_LEN 5
#define IPV4 4
#define DEFAULT_OSPF_TOS 0xc0
#define DEFAULT_OSPF_TTL 1

#define DEFAULT_DD_SEQ_NUM_BEGIN 1075

#define LSINFINITY 0xffffff

#define ENABLED 1
#define DISABLED 0

#endif
