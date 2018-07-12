#ifndef _SPF_H
#define _SPF_H

#include "area.h"
#include "shared.h"

/* 16. Calculation of the routing table */
/* This section details the OSPF routing table calculation. Using its
   attached areas’ link state databases as input, a router runs the
   following algorithm, building its routing table step by step. At
   each step, the router must access individual pieces of the link
   state databases (e.g., a router-LSA originated by a certain router).
   This access is performed by the lookup function discussed in Section
   12.2. The lookup process may return an LSA whose LS age is equal to
   MaxAge. Such an LSA should not be used in the routing table
   calculation, and is treated just as if the lookup process had
   failed.

   The OSPF routing table’s organization is explained in Section 11.
   Two examples of the routing table build process are presented in
   Sections 11.2 and 11.3. This process can be broken into the
   following steps:

   (1) The present routing table is invalidated. The routing table is
   built again from scratch. The old routing table is saved so
   that changes in routing table entries can be identified.

   (2) The intra-area routes are calculated by building the shortest-
   path tree for each attached area. In particular, all routing
   table entries whose Destination Type is "area border router" are
   calculated in this step. This step is described in two parts.
   At first the tree is constructed by only considering those links
   between routers and transit networks. Then the stub networks
   are incorporated into the tree. During the area’s shortest-path
   tree calculation, the area’s TransitCapability is also
   calculated for later use in Step 4.

   (3) The inter-area routes are calculated, through examination of
   summary-LSAs. If the router is attached to multiple areas
   (i.e., it is an area border router), only backbone summary-LSAs
   are examined.

   (4) In area border routers connecting to one or more transit areas
   (i.e, non-backbone areas whose TransitCapability is found to be
   TRUE), the transit areas’ summary-LSAs are examined to see
   whether better paths exist using the transit areas than were
   found in Steps 2-3 above.

   (5) Routes to external destinations are calculated, through
   examination of AS-external-LSAs. The locations of the AS
   boundary routers (which originate the AS-external-LSAs) have
   been determined in steps 2-4.

   Changes made to routing table entries as a result of these
   calculations can cause the OSPF protocol to take further actions.
   For example, a change to an intra-area route will cause an area
   border router to originate new summary-LSAs (see Section 12.4). See
   Section 16.7 for a complete list of the OSPF protocol actions
   resulting from routing table changes. */

/* The first stage of the procedure (i.e., the Dijkstra algorithm)
   can now be summarized as follows. At each iteration of the
   algorithm, there is a list of candidate vertices. Paths from
   the root to these vertices have been found, but not necessarily
   the shortest ones. However, the paths to the candidate vertex
   that is closest to the root are guaranteed to be shortest; this
   vertex is added to the shortest-path tree, removed from the
   candidate list, and its adjacent vertices are examined for
   possible addition to/modification of the candidate list. The
   algorithm then iterates again. It terminates when the candidate
   list becomes empty.

   The following steps describe the algorithm in detail. Remember
   that we are computing the shortest path tree for Area A. All
   references to link state database lookup below are from Area A’s
   database.

   (1) Initialize the algorithm’s data structures. Clear the list
   of candidate vertices. Initialize the shortest-path tree to
   only the root (which is the router doing the calculation).
   Set Area A’s TransitCapability to FALSE.

   (2) Call the vertex just added to the tree vertex V. Examine
   the LSA associated with vertex V. This is a lookup in the
   Area A’s link state database based on the Vertex ID. If
   this is a router-LSA, and bit V of the router-LSA (see
   Section A.4.2) is set, set Area A’s TransitCapability to
   TRUE. In any case, each link described by the LSA gives the
   cost to an adjacent vertex. For each described link, (say
   it joins vertex V to vertex W):

       (a) If this is a link to a stub network, examine the next
       link in V’s LSA. Links to stub networks will be
       considered in the second stage of the shortest path
       calculation.

       (b) Otherwise, W is a transit vertex (router or transit
       network). Look up the vertex W’s LSA (router-LSA or
       network-LSA) in Area A’s link state database. If the
       LSA does not exist, or its LS age is equal to MaxAge, or
       it does not have a link back to vertex V, examine the
       next link in V’s LSA.[23]

       (c) If vertex W is already on the shortest-path tree,
       examine the next link in the LSA.

       (d) Calculate the link state cost D of the resulting path
       from the root to vertex W. D is equal to the sum of the
       link state cost of the (already calculated) shortest
       path to vertex V and the advertised cost of the link
       between vertices V and W. If D is:

           o Greater than the value that already appears for
           vertex W on the candidate list, then examine the
           next link.

           o Equal to the value that appears for vertex W on the
           candidate list, calculate the set of next hops that
           result from using the advertised link. Input to
           this calculation is the destination (W), and its
           parent (V). This calculation is shown in Section
           16.1.1. This set of hops should be added to the
           next hop values that appear for W on the candidate
           list.

           o Less than the value that appears for vertex W on the
           candidate list, or if W does not yet appear on the
           candidate list, then set the entry for W on the
           candidate list to indicate a distance of D from the
           root. Also calculate the list of next hops that
           result from using the advertised link, setting the
           next hop values for W accordingly. The next hop
           calculation is described in Section 16.1.1; it takes
           as input the destination (W) and its parent (V). */

/* 16.2. Calculating the inter-area routes */
/* The inter-area routes are calculated by examining summary-LSAs.
   If the router has active attachments to multiple areas, only
   backbone summary-LSAs are examined. Routers attached to a
   single area examine that area’s summary-LSAs. In either case,
   the summary-LSAs examined below are all part of a single area’s
   link state database (call it Area A).

   Summary-LSAs are originated by the area border routers. Each
   summary-LSA in Area A is considered in turn. Remember that the
   destination described by a summary-LSA is either a network (Type
   3 summary-LSAs) or an AS boundary router (Type 4 summary-LSAs).
   For each summary-LSA:

   (1) If the cost specified by the LSA is LSInfinity, or if the
   LSA’s LS age is equal to MaxAge, then examine the the next
   LSA.

   (2) If the LSA was originated by the calculating router itself,
   examine the next LSA.

   (3) If it is a Type 3 summary-LSA, and the collection of
   destinations described by the summary-LSA equals one of the
   router’s configured area address ranges (see Section 3.5),
   and the particular area address range is active, then the
   summary-LSA should be ignored. "Active" means that there
   are one or more reachable (by intra-area paths) networks
   contained in the area range.

   (4) Else, call the destination described by the LSA N (for Type
   3 summary-LSAs, N’s address is obtained by masking the LSA’s
   Link State ID with the network/subnet mask contained in the
   body of the LSA), and the area border originating the LSA
   BR. Look up the routing table entry for BR having Area A as
   its associated area. If no such entry exists for router BR
   (i.e., BR is unreachable in Area A), do nothing with this
   LSA and consider the next in the list. Else, this LSA
   describes an inter-area path to destination N, whose cost is
   the distance to BR plus the cost specified in the LSA. Call
   the cost of this inter-area path IAC.

   (5) Next, look up the routing table entry for the destination N.
   (If N is an AS boundary router, look up the "router" routing
   table entry associated with Area A). If no entry exists for
   N or if the entry’s path type is "type 1 external" or "type
   2 external", then install the inter-area path to N, with
   associated area Area A, cost IAC, next hop equal to the list
   of next hops to router BR, and Advertising router equal to
   BR.

   (6) Else, if the paths present in the table are intra-area
   paths, do nothing with the LSA (intra-area paths are always
   preferred).

   (7) Else, the paths present in the routing table are also
   inter-area paths. Install the new path through BR if it is
   cheaper, overriding the paths in the routing table.
   Otherwise, if the new path is the same cost, add it to the
   list of paths that appear in the routing table entry. */

void shortest_path_tree(area *a);

#endif
