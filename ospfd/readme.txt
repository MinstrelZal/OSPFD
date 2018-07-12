ospf协议基本原理

ospf协议是IETF组织开发的一个基于链路状态(Link State)算法的内部网关协议，ospf
是开放最短路由(Open Shortest Path First)的缩写。其核心思想是，每一台路由器将其周边
的链路状态(包括接口的直连网段、相连的路由器等信息)描述出来，发送给网络中相邻的路由
器，经过一段时间的链路状态信息交互，每台路由器都保存了一个链路状态数据库，该数据库是
整个网络完整的链路状态描述，在此基础上，应用Shortest Path First算法就可以计算路由。

本项目是基于RFC2328实现的针对IPv4协议使用的简单ospf协议，实现了ospf协议中一部分简单
的功能，包括：

自治系统AS：
Autonomy System是指由同一机构管理，使用同一组选路策略的路由器的集合。它的最主要的特
点是有权决定在本系统内采用何种路由选择协议。ospf协议是一种内部网关协议，其应用范围为
一个AS内部。

区域Area：
指一个路由器的集合，相同的区域有着相同的拓扑结构数据库。ospf用区域把一个AS分成多个链
路状态域。

邻居neighbor：
ospf路由器启动后，便会通过ospf接口向外发送Hello报文。收到Hello报文的ospf路由器会检
查报文中所定义的一些参数，如果双方一致就会形成邻居关系。

邻接adjacencies：
形成邻居关系的双方不一定都能形成邻接关系，这要视网络的类型而定。只有当双方成功交换链
路状态通告信息，才形成真正意义上的邻接关系。

Router ID：
ospf协议使用一个被称作Router ID的32位无符号整数来唯一标识一台路由器，它采用IP地址
形式。Router ID通常要求人为指定，如果没有人为指定，路由器会自动选择一个接口的IP地址
为Router ID。一般有限选择路由器器回环接口(Loopback)中最大IP地址为路由器的Router ID。
如果没有配置Loopback接口，就选取物理接口中最大的IP地址。

ospf协议的可靠传输机制：
由于ospf协议直接用IP报文来封装自己的协议报文，使得ospf协议直接基于IP，而IP无法提供
可靠传输服务，但是ospf协议又要求路由信息可靠地传输，因此ospf协议采用了确认和重传的
机制，以确保ospf路由信息的可靠传输。例如，基于DD报文主从关系协商的隐含确认机制，以及
DD报文、LSR报文、LSU报文发出后，若在规定的时间内没有收到对方相应的确认报文，会自动重传。

Hello报文：
周期性地发送给本路由器的邻居，使用的组播地址是224.0.0.5。DR和BDR发送和接受报文使用的
组播地址是224.0.0.6。
默认情况下，P2P、Broadcast类型接口发送Hello报文时间间隔为10s；PTMP、NBMA类型接口发送
Hello报文的时间间隔为30s。

DD报文：
DD报文被用来交换邻居路由器之间链路状态数据库的摘要信息，因为链路状态描述LSA的摘要信息
通常是LSA的首部，只占整个LSA的一小部分，这样做是为了减少路由器之间信息传递量。
DD报文分为空DD报文和带有摘要信息的DD报文两种。通常开始时，两个邻居路由器相互发送空DD
报文，用来确定Master/Slave关系。确定Master/Slave关系后，才发送带有摘要信息的DD报文，
通过ospf建立主从隐含确认和超时重传机制，保证DD报文的有序可靠交互。

LSR报文：
两台路由器互相交换了DD报文之后，通过比较，就能确定本地的LSDB所缺少的LSA和需要更新的LSA，
这时就需要发送LSR报文向对方请求所需的LSA。报文的内容包括所需要的LSA的摘要。

LSU报文：
用来向发送LSR报文的路由器发送其所需要的LSA，报文的内容是多条LSA的集合。

LSAck报文：
与DD报文的情况类似，ospf协议通过发送与确认和超时重传机制来实现链路状态描述信息LSA的
可靠传输，LS Ack报文就是用来对接收到的LSU报文进行确认。报文的内容是需要确认的LSA首部
（一个报文可对多个LSA进行确认）。

邻居状态机：
                                        +----+
                                        |Down|
                                        +----+
                                          |\
                                          | \Start
                                          |  \      +-------+
                                    Hello |   +---->|Attempt|
                                 Received |         +-------+
                                          |             |
                                  +----+<-+             |HelloReceived
                                  |Init|<---------------+
                                  +----+<--------+
                                     |           |
                                     |2-Way      |1-Way
                                     |Received   |Received
                                     |           |
                   +-------+         |        +-----+
                   |ExStart|<--------+------->|2-Way|
                   +-------+                  +-----+  
                      |
       NegotiationDone|
                      +->+--------+
                         |Exchange|
                      +--+--------+
                      |
              Exchange|
                 Done |
      +----+          |      +-------+
      |Full|<---------+----->|Loading|
      +----+<-+              +-------+
              | LoadingDone      |
              +------------------+

本项目中，对邻居状态机进行了简化：
                                        +----+
                                        |Down|
                                        +----+
                                          |
                                    Hello |
                                 Received |
                                          |
                                  +----+<-+             
                                  |Init|
                                  +----+<--------+
                                     |           |
                                     |2-Way      |1-Way
                                     |Received   |Received
                                     |           |
                                     |        +-----+
                                     +------->|2-Way|
                                              +-----+                   
                                                 |
                                                 |
                   +-------+<--------------------+
                   |ExStart|
                   +-------+
                      |
       NegotiationDone|
                      +->+--------+
                         |Exchange|
                      +--+--------+
                      |
              Exchange|
                 Done |
      +----+          |      +-------+
      |Full|          +----->|Loading|
      +----+<-+              +-------+
              | LoadingDone      |
              +------------------+


指定路由器DR和备份指定路由器BDR：
为了解决全连接网络中的全连接问题，ospf协议从该网络中自动选取一台路由器为指定路由器DR，DR来负
责传递信息。所有路由器都只将路由信息发送给DR，再由DR将路由信息发送给本网段内的其他路由器。
同时，考虑到如果DR由于某种故障而失效，这时必须重新选举DR，并进行新一轮的链路状态信息同步。这需
要较长的时间。为了进行快速响应，ospf提出了备份指定路由器BDR。它与DR一同选举，DR失效后，BDR能立
即成为DR，然后其他路由器再重新选举BDR。
这样，任意两台不是DR和BDR的路由器之间不再建立邻接关系，也不再交换任何路由信息。它们只相互周期
性地交换Hello报文，它们之间的邻居状态机停留在2-way状态。

DR和BDR的选举过程：
1.比较interface->router_priority和neighbor->neighbor_priority
2.比较interface->ip和neighbor->neighbor_ip

ospf报文交互过程：
1.初始化RT1的neighbor list为空
2.RT1在一个连接到广播类型网络的接口上激活ospf协议，并发送Hello报文，Hello报文中包含RT1的
  neighbor list中的所有neighbor的id
3.设RT2收到了RT1发的Hello报文，于是RT2新建一个neighbor，其中neighbor_id为RT1的Router ID,
  将neighbor state设置为init
4.由于RT2发送的Hello报文中包含其neighbor list中的所有neighbor的id，所以也一定包含RT1的id，
  因此当RT1检查到来自RT2的Hello报文中包含自己本身的router id时，将neighbor state设置为
  2-way
5.在RT1中为RT2建立的neighbor的state到达2-way以后，就要根据后面接收到的报文确定是否建立邻接
  关系，根据RFC2328，在7种情况下RT1和RT2会建立邻接关系，本项目中只取其中4中，即RT1和RT2中
  至少有一个为DR或BDR的时候，这时将neighbor state设置为ExStart
6.在neighbor state到达ExStart以后，就要开始进行DD报文的交换了。首先需要确定邻接关系中两台
  路由器的主从关系。因此设RT1先发送一个空DD报文给RT2，宣称自己是Master（MS = 1），并且将
  flags字段的I和M都设置为1，I = 1表示这是第一条DD报文，报文中并不包含LSA的摘要，只是为了
  协商主从关系。M = 1说明这不是最后一条DD报文。
7.RT2在接收到RT1的这条DD报文以后，将对应的neighbor state设置为ExStart，比较报文中的Router
  id和自己的Router id的大小，若自己的更大，则在neighbor中将自己设置为Master并重新生成一个
  初始的DD Sequence Number，并将neighbor state设置成Exchange
8.RT2向RT1也发送一条空DD报文，其中也是将I、MS、M均设置为1，RT1接收到这条报文以后，同样比较
  Router id，同意将RT2作为Master，并将自己对应的neighbor中的DD Sequence Number设置为
  该报文中的DD Sequence Number，并将neighbor state设置成Exchange。之后，RT1开始向RT2发送
  携带LSA header的DD报文，报文的I = 0，MS = 0，M = 1，DD Sequence Number = neighbor的
  DD Sequence Number
9.RT2收到RT1发的这条报文，检查报文中的DD Sequence Number，由于RT1是Slave，该报文中的DD 
  Sequence Number应该等于RT2对应的neighbor中的DD Sequence Number，将neighbor中的DD 
  Sequcnce Number自增1，将该报文中携带的，对应area的LSDB中没有的或者是时间更新的LSA 
  header加入到neighbor的lsa_hdrs中
  ...
10.双方经过一系列交换后，发送的DD报文M = 0，说明是最后一条DD报文，将各自的neighbor state设置
  为Loading
11.当neighbor state为Exchange或者Loading时，neighbor双方可以互相发送lsr报文来请求完整的lsa。
  每过RxmtInterval时间就发送请求neighbor->lsa_hdrs中所有lsa完整信息的lsr报文，直到neighbor->
  lsa_hdrs为空。neighbor接收到的lsr报文保存在neighbor->lsrs中。当neighbor->lsa_hdrs为空并且
  neighbor state是Loading时，将neighbor state设置为Full
12.在收到对方的lsr报文并且自身状态是Exchange时即向对方发送lsu报文，在对方收到lsu报文以后需要发
  送lsack报文进行确认
13.重复上述过程直到双方都已经有了对方的LSA

LSA生成：
1.生成Router-LSA

路由计算：
1.invalidate原来的路由表
2.先计算区域内路由，根据该区域的LSDB中Router-LSA和Network-LSA进行计算，在计算时若碰到StubNet
  的链路类型，需要先跳过，等最后再以叶子节点的的形式挂到最小生成树上
3.计算区域间路由，使用两类Summary-LSA
4.检查连接了多个transit area的ABR，若有比之前得到的路径更短的路径，更新路由表
5.使用AS-external-LSA计算AS外的路由

报文转发：
1.添加路由：route add
2.删除路由：route del
3.在network_init()函数中需要执行系统命令：sudo echo 1 > /proc/sys/net/ipv4/ip_forward



"shared.h"
1.全局宏定义



"ospfd.h"
1.一系列全局变量

"ospf_packets.h"
1.定义了ospf协议中报文以及LSA的结构

"area.h"
1.定义了area data structure

2.函数
查找interface所在的area
struct area *lookup_area_by_if(const struct interface_data *iface);

根据id查找vertex
int lookup_vertex_by_id(struct area *a, in_addr_t id);

根据id查找area
struct area *lookup_area_by_id(uint32_t area_id);

初始化area
struct area *area_init(uint32_t area_id);

将interface加入到area中
void add_area_ifs(struct area *a, struct interface_data *iface);

找到区域内路由表中的最短路径
int lookup_least_cost_vertex(area *a);

找到区域内路由表中到达某个目的地的最短路径
int lookup_least_cost_vertex_by_id(area *a, in_addr_t id);



"neighbor.h"
1.定义了neighbor state
2.定义了neighbor data structure
3.定义了Events causing neighbor state changes
4.定义了neighbor state machine的转移节点
5.全局变量
6.函数
neighbor state machine的状态转移函数
void add_neighbor_event(struct interface_data *iface, struct neighbor *nbr, neighbor_event event);

根据neighbor id查找neighbor ip
in_addr_t lookup_neighbor_ip_by_id(const struct area *a, in_addr_t id);

初始化neighbor
neighbor *neighbor_init(ospf_hello_pkt *hello, uint32_t router_id, in_addr_t src);

清空neighbor中的lsa
void clear_neighbor_lsas(neighbor *nbr);



"interface.h"
1.定义了interface data structure

2.函数
初始化interface
int interface_init();

设置interface的area id
void set_interface_area(struct interface_data *iface, uint32_t area_id);

根据ip查找interface name
const char *lookup_ifname_by_ip(const struct area *a, in_addr_t ip);



"lsa.h"

1.函数
生成checksum
uint16_t fletcher16(const uint8_t *data, size_t len);

比较两个LSA是否相同
int lsa_hdr_eql(const struct ospf_lsa_header *a, const struct ospf_lsa_header *b);

查找某个area中的头部为lsa_hdr的LSA
struct ospf_lsa_header *lookup_lsa(const struct area *a, const struct ospf_lsa_header *lsa_hdr);

比较两个lSA哪个更新
int cmp_lsa_hdr(const struct ospf_lsa_header *a, const struct ospf_lsa_header *b);

将LSA添加到对应的neighbor的lsa_hdrs中
void add_lsa_hdr(struct neighbor *nbr, const struct ospf_lsa_header *lsa_hdr);

将LSA载入对应area的link state database中
struct ospf_lsa_header *install_lsa(struct area *a, const struct ospf_lsa_header *lsa_hdr);

获取下一个LS sequence number
int32_t get_ls_seqnum();

生成自己的router LSA
struct ospf_lsa_header *originate_router_lsa(struct area *a);

封装自己生成的LSA
void encapsulate_self_lsa(const struct area *a, const struct ospf_lsa_header *lsa, struct ospf_header *ospf_hdr);



"hello.h"

1.函数
封装ospf hello报文的body部分
void encapsulate_hello_pkt(const struct interface_data *iface, struct ospf_header *ospf_hdr);

处理接收到的ospf hello报文
void process_hello_pkt(struct interface_data *iface, struct neighbor *nbr, const struct ospf_header *ospf_hdr, in_addr_t src);



"dd.h"

1.函数
封装ospf dd报文的body部分
void encapsulate_dd_pkt(const struct interface_data *iface, const struct neighbor *nbr, struct ospf_header *ospf_hdr);

处理接收到的ospf dd报文
void process_dd_pkt(struct interface_data *iface, struct neighbor *nbr, const struct ospf_header *ospf_hdr);



"lsr.h"

1.函数
封装ospf lsr报文的body部分
void encapsulate_lsr_pkt(struct interface_data *iface, const struct neighbor *nbr, struct ospf_header *ospf_hdr);

处理接收到的ospf lsr报文
void process_lsr_pkt(struct neighbor *nbr, struct ospf_header *ospf_hdr);



"lsu.h"

1.函数
封装ospf lsu报文的body部分
void encapsulate_lsu_pkt(const struct area *a, const struct neighbor *nbr, struct ospf_header *ospf_hdr);

处理接收到的ospf lsu报文
void process_lsu_pkt(struct area *a, struct neighbor *nbr, struct ospf_header *ospf_hdr);



"lsack.h"

1.函数
封装ospf lsack报文的body部分
void encapsulate_lsack_pkt(const struct neighbor *nbr, struct ospf_header *ospf_hdr);

处理接收到的ospf lsack报文
void process_lsack_pkt(const struct neighbor *nbr, struct ospf_header *ospf_header);



"route.h"

1.定义了The Routing Table Structure
2.函数
invalidate旧路由表
void invalidated_old_routing_table();

根据destination id查找路由
int lookup_route_by_dst(in_addr_t dest_id);

找到路由表中的最短路径
int lookup_route_by_least_cost();

更新路由表
void update_routing_table();



"spf.h"

1.函数
计算整个shortest path tree
void shortest_path_tree(struct area *a);



"network.h"

1.函数

从interface接收ospf报文
struct interface *recv_ospf(int sock, uint8_t buf[], int size, in_addr_t *src);

从interface发送ospf报文
void send_ospf(const struct interface_data *iface, struct iphdr *ip_hdr, in_addr_t dst);

初始化网络
void network_init();

处理接收到的ospf报文的线程
void *recv_and_process();

发送ospf报文的线程
void *encapsulate_and_send();