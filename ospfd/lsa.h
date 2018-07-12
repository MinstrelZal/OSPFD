#ifndef _LSA_H
#define _LSA_H

#include "ospf_packets.h"
#include "area.h"
#include "shared.h"
#include <arpa/inet.h>

/* 12.2. The link state database */
/* A router has a separate link state database for every area to
   which it belongs. All routers belonging to the same area have
   identical link state databases for the area.

   The databases for each individual area are always dealt with
   separately. The shortest path calculation is performed
   separately for each area (see Section 16). Components of the
   area link-state database are flooded throughout the area only.
   Finally, when an adjacency (belonging to Area A) is being
   brought up, only the database for Area A is synchronized between
   the two routers.

   The area database is composed of router-LSAs, network-LSAs and
   summary-LSAs (all listed in the area data structure). In
   addition, external routes (AS-external-LSAs) are included in all
   non-stub area databases (see Section 3.6).

   An implementation of OSPF must be able to access individual
   pieces of an area database. This lookup function is based on an
   LSA’s LS type, Link State ID and Advertising Router.[14] There
   will be a single instance (the most up-to-date) of each LSA in
   the database. The database lookup function is invoked during
   the LSA flooding procedure (Section 13) and the routing table
   calculation (Section 16). In addition, using this lookup
   function the router can determine whether it has itself ever
   originated a particular LSA, and if so, with what LS sequence
   number.

   An LSA is added to a router’s database when either a) it is
   received during the flooding process (Section 13) or b) it is
   originated by the router itself (Section 12.4). An LSA is
   deleted from a router’s database when either a) it has been
   overwritten by a newer instance during the flooding process
   (Section 13) or b) the router originates a newer instance of one
   of its self-originated LSAs (Section 12.4) or c) the LSA ages
   out and is flushed from the routing domain (Section 14).
   Whenever an LSA is deleted from the database it must also be
   removed from all neighbors’ Link state retransmission lists (see
   Section 10). */


/* 12.4. Originating LSAs */
/* Into any given OSPF area, a router will originate several LSAs.
   Each router originates a router-LSA. If the router is also the
   Designated Router for any of the area’s networks, it will
   originate network-LSAs for those networks.

   Area border routers originate a single summary-LSA for each
   known inter-area destination. AS boundary routers originate a
   single AS-external-LSA for each known AS external destination.
   Destinations are advertised one at a time so that the change in
   any single route can be flooded without reflooding the entire
   collection of routes. During the flooding procedure, many LSAs
   can be carried by a single Link State Update packet.

   As an example, consider Router RT4 in Figure 6. It is an area
   border router, having a connection to Area 1 and the backbone.
   Router RT4 originates 5 distinct LSAs into the backbone (one
   router-LSA, and one summary-LSA for each of the networks N1-N4).
   Router RT4 will also originate 8 distinct LSAs into Area 1 (one
   router-LSA and seven summary-LSAs as pictured in Figure 7). If
   RT4 has been selected as Designated Router for Network N3, it
   will also originate a network-LSA for N3 into Area 1.

   In this same figure, Router RT5 will be originating 3 distinct
   AS-external-LSAs (one for each of the networks N12-N14). These
   will be flooded throughout the entire AS, assuming that none of
   the areas have been configured as stubs. However, if area 3 has
   been configured as a stub area, the AS-external-LSAs for
   networks N12-N14 will not be flooded into area 3 (see Section
   3.6). Instead, Router RT11 would originate a default summary-
   LSA that would be flooded throughout area 3 (see Section
   12.4.3). This instructs all of area 3’s internal routers to
   send their AS external traffic to RT11.

   Whenever a new instance of an LSA is originated, its LS sequence
   number is incremented, its LS age is set to 0, its LS checksum
   is calculated, and the LSA is added to the link state database
   and flooded out the appropriate interfaces. See Section 13.2
   for details concerning the installation of the LSA into the link
   state database. See Section 13.3 for details concerning the
   flooding of newly originated LSAs. 

   The ten events that can cause a new instance of an LSA to be
   originated are:

   (1) The LS age field of one of the router’s self-originated LSAs
       reaches the value LSRefreshTime. In this case, a new
       instance of the LSA is originated, even though the contents
       of the LSA (apart from the LSA header) will be the same.
       This guarantees periodic originations of all LSAs. This
       periodic updating of LSAs adds robustness to the link state
       algorithm. LSAs that solely describe unreachable
       destinations should not be refreshed, but should instead be
       flushed from the routing domain (see Section 14.1).
       When whatever is being described by an LSA changes, a new LSA is
       originated. However, two instances of the same LSA may not be
       originated within the time period MinLSInterval. This may
       require that the generation of the next instance be delayed by
       up to MinLSInterval. The following events may cause the
       contents of an LSA to change. These events should cause new
       originations if and only if the contents of the new LSA would be
       different:
   (2) An interface’s state changes (see Section 9.1). This may
       mean that it is necessary to produce a new instance of the
       router-LSA.
   (3) An attached network’s Designated Router changes. A new
       router-LSA should be originated. Also, if the router itself
       is now the Designated Router, a new network-LSA should be
       produced. If the router itself is no longer the Designated
       Router, any network-LSA that it might have originated for
       the network should be flushed from the routing domain (see
       Section 14.1).
   (4) One of the neighboring routers changes to/from the FULL
       state. This may mean that it is necessary to produce a new
       instance of the router-LSA. Also, if the router is itself
       the Designated Router for the attached network, a new
       network-LSA should be produced.

   The next four events concern area border routers only:

   (5) An intra-area route has been added/deleted/modified in the
       routing table. This may cause a new instance of a summary-
       LSA (for this route) to be originated in each attached area
       (possibly including the backbone).
   (6) An inter-area route has been added/deleted/modified in the
       routing table. This may cause a new instance of a summary-
       LSA (for this route) to be originated in each attached area
       (but NEVER for the backbone).
   (7) The router becomes newly attached to an area. The router
       must then originate summary-LSAs into the newly attached
       area for all pertinent intra-area and inter-area routes in
       the router’s routing table. See Section 12.4.3 for more
       details.
   (8) When the state of one of the router’s configured virtual
       links changes, it may be necessary to originate a new
       router-LSA into the virtual link’s Transit area (see the
       discussion of the router-LSA’s bit V in Section 12.4.1), as
       well as originating a new router-LSA into the backbone.

   The last two events concern AS boundary routers (and former AS
   boundary routers) only:

   (9) An external route gained through direct experience with an
       external routing protocol (like BGP) changes. This will
       cause an AS boundary router to originate a new instance of
       an AS-external-LSA.
   (10)
       A router ceases to be an AS boundary router, perhaps after
       restarting. In this situation the router should flush all
       AS-external-LSAs that it had previously originated. These
       LSAs can be flushed via the premature aging procedure
       specified in Section 14.1. */

uint16_t fletcher16(const uint8_t *data, size_t len);
int lsa_hdr_eql(const ospf_lsa_header *a, const ospf_lsa_header *b);
ospf_lsa_header *lookup_lsa(const area *a, const ospf_lsa_header *lsa_hdr);
int cmp_lsa_hdr(const ospf_lsa_header *a, const ospf_lsa_header *b);
void add_lsa_hdr(neighbor *nbr, const ospf_lsa_header *lsa_hdr);
ospf_lsa_header *install_lsa(area *a, const ospf_lsa_header *lsa_hdr);
int32_t get_ls_seqnum();
ospf_lsa_header *originate_router_lsa(area *a);
void encapsulate_self_lsa(const ospf_lsa_header *lsa, ospf_header *ospf_hdr);


#endif
