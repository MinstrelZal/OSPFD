#ifndef _OSPF_PACKETS_H
#define _OSPF_PACKETS_H

#include <netinet/ip.h>
#include <netinet/in.h>
#include "shared.h"

/* A.3.1 The OSPF packet header */
typedef struct ospf_header{
    /* The OSPF version number. This specification documents version 2
       of the protocol. */
    uint8_t version;

    /* The OSPF packet types are as follows. See Sections A.3.2 through
       A.3.6 for details. 
       Type    Description
       ----------------------------------
       1       Hello
       2       Database Description
       3       Link State Request
       4       Link State Update
       5       Link State Acknowledgment */
    uint8_t type; 

    /* The length of the OSPF protocol packet in bytes. This length
       includes the standard OSPF header. */
    uint16_t pktlen; 

    /* The Router ID of the packet’s source. */
    uint32_t router_id; 

    /* A 32 bit number identifying the area that this packet belongs
       to. All OSPF packets are associated with a single area. Most
       travel a single hop only. Packets travelling over a virtual
       link are labelled with the backbone Area ID of 0.0.0.0. */
    uint32_t area_id; 

    /* The standard IP checksum of the entire contents of the packet,
       starting with the OSPF packet header but excluding the 64-bit
       authentication field. This checksum is calculated as the 16-bit
       one’s complement of the one’s complement sum of all the 16-bit
       words in the packet, excepting the authentication field. If the
       packet’s length is not an integral number of 16-bit words, the
       packet is padded with a byte of zero before checksumming. The
       checksum is considered to be part of the packet authentication
       procedure; for some authentication types the checksum
       calculation is omitted. */
    uint16_t checksum; 

    /* Identifies the authentication procedure to be used for the
       packet. Authentication is discussed in Appendix D of the
       specification. Consult Appendix D for a list of the currently
       defined authentication types. */
    uint16_t autype; 

    /* A 64-bit field for use by the authentication scheme. See
       Appendix D for details. */
    union{
        uint8_t auth_data[OSPF_AUTH_SIMPLE_SIZE];
        struct{
            uint16_t unused;
            uint8_t key_id;
            uint8_t auth_data_len;
            uint32_t crypt_seq_number;
        }crypt;
    }u;
}ospf_header;


/* A.3.2 The Hello packet */
typedef struct ospf_hello_pkt{
    /* The network mask associated with this interface. For example,
       if the interface is to a class B network whose third byte is
       used for subnetting, the network mask is 0xffffff00. */
    in_addr_t network_mask;

    /* The number of seconds between this router’s Hello packets. */
    uint16_t hello_interval; 

    /* The optional capabilities supported by the router, as documented 
       in Section A.2. */
    uint8_t options; 

    /* This router’s Router Priority. Used in (Backup) Designated
       Router election. If set to 0, the router will be ineligible to
       become (Backup) Designated Router. */
    uint8_t router_priority;

    /* The number of seconds before declaring a silent router down. */
    uint32_t router_dead_interval;

    /* The identity of the Designated Router for this network, in the
       view of the sending router. The Designated Router is identified
       here by its IP interface address on the network. Set to 0.0.0.0
       if there is no Designated Router. */
    in_addr_t d_router; 

    /* The identity of the Backup Designated Router for this network,
       in the view of the sending router. The Backup Designated Router
       is identified here by its IP interface address on the network.
       Set to 0.0.0.0 if there is no Backup Designated Router. */
    in_addr_t bd_router; 

    /* The Router IDs of each router from whom valid Hello packets have
       been seen recently on the network. Recently means in the last
       RouterDeadInterval seconds. */
    in_addr_t neighbors[]; 
}ospf_hello_pkt;


/* A.4.1 The LSA header */
typedef struct ospf_lsa_header{
    /* This field is the age of the LSA in seconds. It should be
       processed as an unsigned 16-bit integer. It is set to 0
       when the LSA is originated. It must be incremented by
       InfTransDelay on every hop of the flooding procedure. LSAs
       are also aged as they are held in each router’s database.

       The age of an LSA is never incremented past MaxAge. LSAs
       having age MaxAge are not used in the routing table
       calculation. When an LSA’s age first reaches MaxAge, it is
       reflooded. An LSA of age MaxAge is finally flushed from the
       database when it is no longer needed to ensure database
       synchronization. For more information on the aging of LSAs,
       consult Section 14.

       The LS age field is examined when a router receives two
       instances of an LSA, both having identical LS sequence
       numbers and LS checksums. An instance of age MaxAge is then
       always accepted as most recent; this allows old LSAs to be
       flushed quickly from the routing domain. Otherwise, if the
       ages differ by more than MaxAgeDiff, the instance having the
       smaller age is accepted as most recent.[12] See Section 13.1
       for more details. */
    uint16_t ls_age;

    /* The optional capabilities supported by the described portion of
       the routing domain. OSPF’s optional capabilities are documented
       in Section A.2.

       The Options field in the LSA header indicates which optional
       capabilities are associated with the LSA. OSPF’s optional
       capabilities are described in Section 4.5. One optional
       capability is defined by this specification, represented by
       the E-bit found in the Options field. The unrecognized bits
       in the Options field should be set to zero.

       The E-bit represents OSPF’s ExternalRoutingCapability. This
       bit should be set in all LSAs associated with the backbone,
       and all LSAs associated with non-stub areas (see Section
       3.6). It should also be set in all AS-external-LSAs. It
       should be reset in all router-LSAs, network-LSAs and
       summary-LSAs associated with a stub area. For all LSAs, the
       setting of the E-bit is for informational purposes only; it
       does not affect the routing table calculation. */
    uint8_t options;

    /* The LS type field dictates the format and function of the
       LSA. LSAs of different types have different names (e.g.,
       router-LSAs or network-LSAs). All LSA types defined by this
       memo, except the AS-external-LSAs (LS type = 5), are flooded
       throughout a single area only. AS-external-LSAs are flooded
       throughout the entire Autonomous System, excepting stub
       areas (see Section 3.6). Each separate LSA type is briefly
       described below in Table 15.

       The type of the LSA. Each LSA type has a separate advertisement
       format. The LSA types defined in this memo are as follows (see
       Section 12.1.3 for further explanation): 
       LS Type    Description
       -------------------------------------
       1          Router-LSAs
       2          Network-LSAs
       3          Summary-LSAs (IP network)
       4          Summary-LSAs (ASBR)
       5          AS-external-LSAs            */
    uint8_t ls_type;

    /* This field identifies the portion of the internet environment
       that is being described by the LSA. The contents of this field
       depend on the LSA’s LS type. For example, in network-LSAs the
       Link State ID is set to the IP interface address of the
       network’s Designated Router (from which the network’s IP address
       can be derived). The Link State ID is further discussed in
       Section 12.1.4.

       This field identifies the piece of the routing domain that
       is being described by the LSA. Depending on the LSA’s LS
       type, the Link State ID takes on the values listed in Table 16 
       Actually, for Type 3 summary-LSAs (LS type = 3) and ASexternal-
       LSAs (LS type = 5), the Link State ID may additionally have one 
       or more of the destination network’s "host" bits set. For 
       example, when originating an ASexternal-LSA for the network 
       10.0.0.0 with mask of 255.0.0.0, the Link State ID can be set 
       to anything in the range 10.0.0.0 through 10.255.255.255 
       inclusive (although 10.0.0.0 should be used whenever possible). 
       The freedom to set certain host bits allows a router to 
       originate separate LSAs for two networks having the same 
       address but different masks. See Appendix E for details.

       When the LSA is describing a network (LS type = 2, 3 or 5),
       the network’s IP address is easily derived by masking the
       Link State ID with the network/subnet mask contained in the
       body of the LSA. When the LSA is describing a router (LS
       type = 1 or 4), the Link State ID is always the described
       router’s OSPF Router ID.
       When an AS-external-LSA (LS Type = 5) is describing a
       default route, its Link State ID is set to
       DefaultDestination (0.0.0.0).

       LS        Type Link State ID
       -----------------------------------------------------------------------
       1         The originating router’s Router ID.
       2         The IP interface address of the network’s Designated Router.
       3         The destination network’s IP address.
       4         The Router ID of the described AS boundary router.
       5         The destination network’s IP address.                        */
    in_addr_t link_state_id;

    /* The Router ID of the router that originated the LSA. For
       example, in network-LSAs this field is equal to the Router ID of
       the network’s Designated Router.

       This field specifies the OSPF Router ID of the LSA’s
       originator. For router-LSAs, this field is identical to the
       Link State ID field. Network-LSAs are originated by the
       network’s Designated Router. Summary-LSAs originated by
       area border routers. AS-external-LSAs are originated by AS
       boundary routers. */
    in_addr_t adv_router;

    /* Detects old or duplicate LSAs. Successive instances of an LSA
       are given successive LS sequence numbers. See Section 12.1.6
       for more details.

       The sequence number field is a signed 32-bit integer. It is
       used to detect old and duplicate LSAs. The space of
       sequence numbers is linearly ordered. The larger the
       sequence number (when compared as signed 32-bit integers)
       the more recent the LSA. To describe to sequence number
       space more precisely, let N refer in the discussion below to
       the constant 2**31.

       The sequence number -N (0x80000000) is reserved (and
       unused). This leaves -N + 1 (0x80000001) as the smallest
       (and therefore oldest) sequence number; this sequence number
       is referred to as the constant InitialSequenceNumber. A
       router uses InitialSequenceNumber the first time it
       originates any LSA. Afterwards, the LSA’s sequence number
       is incremented each time the router originates a new
       instance of the LSA. When an attempt is made to increment
       the sequence number past the maximum value of N - 1
       (0x7fffffff; also referred to as MaxSequenceNumber), the
       current instance of the LSA must first be flushed from the
       routing domain. This is done by prematurely aging the LSA
       (see Section 14.1) and reflooding it. As soon as this flood
       has been acknowledged by all adjacent neighbors, a new
       instance can be originated with sequence number of
       InitialSequenceNumber.

       The router may be forced to promote the sequence number of
       one of its LSAs when a more recent instance of the LSA is
       unexpectedly received during the flooding process. This
       should be a rare event. This may indicate that an out-ofdate
       LSA, originated by the router itself before its last
       restart/reload, still exists in the Autonomous System. For
       more information see Section 13.4. */
    int32_t ls_seqnum;

    /* The Fletcher checksum of the complete contents of the LSA,
       including the LSA header but excluding the LS age field. See
       Section 12.1.7 for more details.

       This field is the checksum of the complete contents of the
       LSA, excepting the LS age field. The LS age field is
       excepted so that an LSA’s age can be incremented without
       updating the checksum. The checksum used is the same that
       is used for ISO connectionless datagrams; it is commonly
       referred to as the Fletcher checksum. It is documented in
       Annex B of [Ref6]. The LSA header also contains the length
       of the LSA in bytes; subtracting the size of the LS age
       field (two bytes) yields the amount of data to checksum.

       The checksum is used to detect data corruption of an LSA.
       This corruption can occur while an LSA is being flooded, or
       while it is being held in a router’s memory. The LS
       checksum field cannot take on the value of zero; the
       occurrence of such a value should be considered a checksum
       failure. In other words, calculation of the checksum is not
       optional.

       The checksum of an LSA is verified in two cases: a) when it
       is received in a Link State Update Packet and b) at times
       during the aging of the link state database. The detection
       of a checksum failure leads to separate actions in each
       case. See Sections 13 and 14 for more details.

       Whenever the LS sequence number field indicates that two
       instances of an LSA are the same, the LS checksum field is
       examined. If there is a difference, the instance with the
       larger LS checksum is considered to be most recent. See
       Section 13.1 for more details. */
    uint16_t ls_chksum;

    /* The length in bytes of the LSA. This includes the 20 byte LSA
       header. */
    uint16_t length;
}ospf_lsa_header;


/* A.3.3 The Database Description packet */
typedef struct ospf_dd_pkt{
    /* The size in bytes of the largest IP datagram that can be sent
       out the associated interface, without fragmentation. The MTUs
       of common Internet link types can be found in Table 7-1 of
       [Ref22]. Interface MTU should be set to 0 in Database
       Description packets sent over virtual links. */
    uint16_t interface_mtu;

    /* The optional capabilities supported by the router, as documented
       in Section A.2. */
    uint8_t options;

    /* I-bit
       The Init bit. When set to 1, this packet is the first in the
       sequence of Database Description Packets. 
       
       M-bit 
       The More bit. When set to 1, it indicates that more Database
       Description Packets are to follow.
       
       MS-bit
       The Master/Slave bit. When set to 1, it indicates that the
       router is the master during the Database Exchange process.
       Otherwise, the router is the slave. */
    uint8_t flags;

    /* Used to sequence the collection of Database Description Packets.
       The initial value (indicated by the Init bit being set) should
       be unique. The DD sequence number then increments until the
       complete database description has been sent. */
    uint32_t dd_seqnum;

    /* LSA header */
    ospf_lsa_header lsa_hdrs[];
}ospf_dd_pkt;


/* A.3.4 The Link State Request packet */
typedef struct ospf_lsr_pkt{
    uint32_t ls_type;
    uint32_t link_state_id;
    uint32_t adv_router;
}ospf_lsr_pkt;


/* A.3.5 The Link State Update packet */
typedef struct ospf_lsu_pkt{
    /* The number of LSAs included in this update. */
    uint32_t num_of_lsa;
}ospf_lsu_pkt;


/* A.4.2 Router-LSAs */
/* In router-LSAs, the Link State ID field is set to the router’s OSPF
   Router ID. Router-LSAs are flooded throughout a single area only. */

/* The following fields are used to describe each router link (i.e.,
   interface). Each router link is typed (see the below Type field).
   The Type field indicates the kind of link being described. It may
   be a link to a transit network, to another router or to a stub
   network. The values of all the other fields describing a router
   link depend on the link’s Type. For example, each link has an
   associated 32-bit Link Data field. For links to stub networks this
   field specifies the network’s IP address mask. For other link types
   the Link Data field specifies the router interface’s IP address. */

typedef struct mylink{
	/* Link ID - Identifies the object that this router link connects to. Value
       depends on the link’s Type. When connecting to an object that
       also originates an LSA (i.e., another router or a transit
       network) the Link ID is equal to the neighboring LSA’s Link
       State ID. This provides the key for looking up the neighboring
       LSA in the link state database during the routing table
       calculation. See Section 12.2 for more details.
       Type      Link ID
    ----------------------------------------------
       1         Neighboring router’s Router ID
       2         IP address of Designated Router
       3         IP network/subnet number
       4         Neighboring router’s Router ID */
	in_addr_t id;

	/* Link Data - Value again depends on the link’s Type field. For connections to
       stub networks, Link Data specifies the network’s IP address
       mask. For unnumbered point-to-point connections, it specifies
       the interface’s MIB-II [Ref8] ifIndex value. For the other link
       types it specifies the router interface’s IP address. This
       latter piece of information is needed during the routing table
       build process, when calculating the IP address of the next hop.
       See Section 16.1.1 for more details. */
	in_addr_t data;

	/* Type - A quick description of the router link. One of the following.
       Note that host routes are classified as links to stub networks
       with network mask of 0xffffffff.
       Type      Description
    ----------------------------------------------------------
       1         Point-to-point connection to another router
       2         Connection to a transit network
       3         Connection to a stub network
       4         Virtual link */
	uint8_t type;

	/* # TOS - The number of different TOS metrics given for this link, not
       counting the required link metric (referred to as the TOS 0
       metric in [Ref9]). For example, if no additional TOS metrics
       are given, this field is set to 0. */
	uint8_t num_diff_tos;

	/* The cost of using this router link. */
	uint16_t metric;

}mylink;

/* 12.3. Representation of TOS */
/* For backward compatibility with previous versions of the OSPF
   specification ([Ref9]), TOS-specific information can be included
   in router-LSAs, summary-LSAs and AS-external-LSAs. The encoding
   of TOS in OSPF LSAs is specified in Table 17. That table relates
   the OSPF encoding to the IP packet header’s TOS field (defined
   in [Ref12]). The OSPF encoding is expressed as a decimal integer, 
   and the IP packet header’s TOS field is expressed in the binary 
   TOS values used in [Ref12].

   OSPF encoding      RFC 1349 TOS values
--------------------------------------------------
   0                  0000 normal service
   2                  0001 minimize monetary cost
   4                  0010 maximize reliability
   6                  0011
   8                  0100 maximize throughput
   10                 0101
   12                 0110
   14                 0111
   16                 1000 minimize delay
   18                 1001
   20                 1010
   22                 1011
   24                 1100
   26                 1101
   28                 1110
   30                 1111                              */

/* Additional TOS-specific information may also be included, for
   backward compatibility with previous versions of the OSPF
   specification ([Ref9]). Within each link, and for each desired TOS,
   TOS TOS-specific link information may be encoded as follows: */
typedef struct router_lsa_tos{
	/* IP Type of Service that this metric refers to. The encoding of
	   TOS in OSPF LSAs is described in Section 12.3. */
	uint8_t tos;
	uint8_t padding;

	/* TOS-specific metric information. */
	uint16_t tos_metric;
}router_lsa_tos;

typedef struct router_lsa{
	/* bit V
           When set, the router is an endpoint of one or more fully
           adjacent virtual links having the described area as Transit area
           (V is for virtual link endpoint).
       bit E
           When set, the router is an AS boundary router (E is for external).
       bit B
           When set, the router is an area border router (B is for border). */
	uint8_t flags;
	uint8_t padding;

	/* # links
           The number of router links described in this LSA. This must be
           the total collection of router links (i.e., interfaces) to the
           area. */
	uint16_t num_link;

	/* links */
	mylink links[];

}router_lsa;


/* A.4.3 Network-LSAs */
/* Network-LSAs are the Type 2 LSAs. A network-LSA is originated for
   each broadcast and NBMA network in the area which supports two or
   more routers. The network-LSA is originated by the network’s
   Designated Router. The LSA describes all routers attached to the
   network, including the Designated Router itself. The LSA’s Link
   State ID field lists the IP interface address of the Designated
   Router.
   The distance from the network to all attached routers is zero. This
   is why metric fields need not be specified in the network-LSA. For
   details concerning the construction of network-LSAs, see Section
   12.4.2. */
typedef struct network_lsa{
	/* Network Mask - The IP address mask for the network. For example, a class A
       network would have the mask 0xff000000. */
	in_addr_t network_mask;

	/* Attached Router - The Router IDs of each of the routers attached to the network.
       Actually, only those routers that are fully adjacent to the
       Designated Router are listed. The Designated Router includes
       itself in this list. The number of routers included can be
       deduced from the LSA header’s length field. */
	in_addr_t *attached_rtrs;
}network_lsa;


/* A.4.4 Summary-LSAs */
/* Summary-LSAs are the Type 3 and 4 LSAs. These LSAs are originated
   by area border routers. Summary-LSAs describe inter-area
   destinations. For details concerning the construction of summary-
   LSAs, see Section 12.4.3.
   Type 3 summary-LSAs are used when the destination is an IP network.
   In this case the LSA’s Link State ID field is an IP network number
   (if necessary, the Link State ID can also have one or more of the
   network’s "host" bits set; see Appendix E for details). When the
   destination is an AS boundary router, a Type 4 summary-LSA is used,
   and the Link State ID field is the AS boundary router’s OSPF Router
   ID. (To see why it is necessary to advertise the location of each
   ASBR, consult Section 16.4.) Other than the difference in the Link
   State ID field, the format of Type 3 and 4 summary-LSAs is
   identical.
   For stub areas, Type 3 summary-LSAs can also be used to describe a
   (per-area) default route. Default summary routes are used in stub
   areas instead of flooding a complete set of external routes. When
   describing a default summary route, the summary-LSA’s Link State ID
   is always set to DefaultDestination (0.0.0.0) and the Network Mask
   is set to 0.0.0.0. */
typedef struct summary_lsa{
	/* Network Mask - For Type 3 summary-LSAs, this indicates the destination
       network’s IP address mask. For example, when advertising the
       location of a class A network the value 0xff000000 would be
       used. This field is not meaningful and must be zero for Type 4
       summary-LSAs. */
	in_addr_t network_mask;

	/* metric - The cost of this route. Expressed in the same units as the
    interface costs in the router-LSAs. */
    uint32_t tos0metric;
}summary_lsa;


/* A.4.5 AS-external-LSAs */
/* AS-external-LSAs are the Type 5 LSAs. These LSAs are originated by
   AS boundary routers, and describe destinations external to the AS.
   For details concerning the construction of AS-external-LSAs, see
   Section 12.4.3.
   AS-external-LSAs usually describe a particular external destination.
   For these LSAs the Link State ID field specifies an IP network
   number (if necessary, the Link State ID can also have one or more of
   the network’s "host" bits set; see Appendix E for details). ASexternal-
   LSAs are also used to describe a default route. Default
   routes are used when no specific route exists to the destination.
   When describing a default route, the Link State ID is always set to
   DefaultDestination (0.0.0.0) and the Network Mask is set to 0.0.0.0. */

typedef struct as_external_lsa_tos{
  /* bit E - The type of external metric. If bit E is set, the metric
       specified is a Type 2 external metric. This means the metric is
       considered larger than any link state path. If bit E is zero,
       the specified metric is a Type 1 external metric. This means
       that it is expressed in the same units as the link state metric
       (i.e., the same units as interface cost). */
  /* metric - The cost of this route. Interpretation depends on the external
       type indication (bit E above). */
  uint32_t tos0metric;

  /* Forwarding address - Data traffic for the advertised destination will be forwarded to
       this address. If the Forwarding address is set to 0.0.0.0, data
       traffic will be forwarded instead to the LSA’s originator (i.e.,
       the responsible AS boundary router). */
  in_addr_t forward_addr;

  /* External Route Tag - A 32-bit field attached to each external route. This is not
       used by the OSPF protocol itself. It may be used to communicate
       information between AS boundary routers; the precise nature of
       such information is outside the scope of this specification. */
  uint32_t external_rtr_tag;
}as_external_lsa_tos;


typedef struct as_external_lsa{
	/* Network Mask - The IP address mask for the advertised destination. For
       example, when advertising a class A network the mask 0xff000000
       would be used. */
	in_addr_t network_mask;

	as_external_lsa_tos tos0;

	as_external_lsa_tos *tos;
}as_external_lsa;

#endif
