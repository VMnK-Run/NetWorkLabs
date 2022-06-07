#include <stdio.h>
#include <stdlib.h>
#include "project3.h"

#define NODEID 0

extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt0;
struct NeighborCosts   *neighbor0;
int numNodes;

void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr );

// returns the minimum value in an array of count integers
int min0(int count, int array[]) {
	int minimum;

	minimum = array[--count];

	while (count >= 0) {
		if (array[count] < minimum) minimum = array[count];
		count--;
	}

	return minimum;
}


/* students to write the following two routines, and maybe some others */

/* rtinit0()
 * This routine is called once at the beginning of the emulation. It takes no
 * arguments. It should initialize the distance table in node0 to reflect the
 * direct costs to its neighbors by using GetNeighborCosts(). It should also
 * send to its neighbors the minimum cost paths to all other network nodes in a
 * routing packet using toLayer2().
 */
void rtinit0() {
	int i, j;
	struct RoutePacket toSend;		// 广播给其他节点的数据包

	if (TraceLevel >= 2) {
		printf("At time t=%.3f,rtinid%d() called.\n", clocktime, NODEID);
	}

	// get the initial neighbors
	neighbor0 = getNeighborCosts(NODEID);		// 构造节点
	numNodes = neighbor0->NodesInNetwork;		// 节点数量

	// initialize distance table
	for (i = 0; i < numNodes; i++) {
		for (j = 0; j < numNodes; j++) {
			if (i == j) { // directly connected neighbor
				dt0.costs[i][j] = neighbor0->NodeCosts[i];		// i==j表示经过j到达i，即有一条直接通路
			}
			else {
				dt0.costs[i][j] = INFINITY;
			}
		}
	}

	printf("node%d initial distance table:\n", NODEID);
	printdt0(NODEID, getNeighborCosts(NODEID), &dt0);
	
	if (TraceLevel >= 2) {
		printf("constructing packets to send...\n");
	}

	// construct packets to send to neighbors
	// 广播给其他节点
	for (i = 0; i < numNodes; i++) {
		if (i != NODEID && neighbor0->NodeCosts[i] < INFINITY) {  // 表明这是其邻居节点
			if (TraceLevel >= 3) printf("making packet %d...\n", i);
			toSend.sourceid = NODEID;		// 设置源节点
			toSend.destid = i;				// 设置目的节点
			for (j = 0; j < numNodes; j++) {
				toSend.mincost[j] = dt0.costs[j][j];	// 当前到其他节点的最小代价即直接经过j到j
			}
			printf("node%d told node %d the initial DV{%d, %d, %d, %d}\n", NODEID, i, toSend.mincost[0], toSend.mincost[1], toSend.mincost[2], toSend.mincost[3]);
			// send packet to neighbor
			toLayer2(toSend);
		}
	}
}

/* rtupdate0(struct RoutePacket *rcvdpkt)
 * Called when this node receives a routing packet that was sent to it by one of
 * its directly connected neighbors. The parameter *recvdpkt is a pointer to the
 * packet that was received. rtupdate0() is the heart of the distance vector
 * algorithm. The values it receives in a routing pcket from some other node i
 * contains i's current shortest path costs to all other network nodes.
 * rtupdate0() uses these received values to update its own distance table. If
 * its own minimum cost to another node changes as a result of the update, this
 * node informs its directly connected neighbors of this change by sending them
 * a routing packet.
 */
void rtupdate0( struct RoutePacket *rcvdpkt ) {
	int i, j, n, tableUpdated, src;
	struct RoutePacket toSend;

	if (TraceLevel >= 2) {
		printf("At time t=%.3f, node%d received routing packet from %d\n"
				, clocktime, NODEID, rcvdpkt->sourceid);
	}

	// don't break 18 USC Section 1702 (reading someone else's mail)!
	if (rcvdpkt->destid != NODEID) {	// 不是发给该节点的数据包
		printf("node%d received %d's mail\n", NODEID, rcvdpkt->destid);
		exit(1);
	}

	tableUpdated = 0; // flag，暂时假设并没有更新
	src = rcvdpkt->sourceid;	// 网络中发生状态变化的节点，需要根据该节点更新距离表

	for (i = 0; i < numNodes; i++) {
		if (dt0.costs[i][src] > dt0.costs[src][src] + rcvdpkt->mincost[i]) { // 如果通过该点能够更快到达i
			dt0.costs[i][src] = dt0.costs[src][src] + rcvdpkt->mincost[i];	 // 那么根据该点更新到i的距离
			if (i != NODEID) {
				tableUpdated = 1;	// 如果i不是本节点，则认为距离表发生了变化，需要再次广播
			}
		}
	}

	if (tableUpdated) {
		printf("node%d distance table updated: \n", NODEID);
		printdt0(NODEID, neighbor0, &dt0);

		// initialize toSend
		for (i = 0; i < numNodes; i++) {
			toSend.mincost[i] = INFINITY; // 初始化
		}
		
		// construct packets to send to neighbors
		toSend.sourceid = NODEID;
		for (i = 0; i < numNodes; i++) {
			for (j = 0; j < numNodes; j++) {
				if (toSend.mincost[i] > dt0.costs[i][j])
					toSend.mincost[i] = dt0.costs[i][j]; // 遍历图，更新mincost数组
			}
		}
		for (n = 0; n < numNodes; n++) { // 需要发送的编号，发送到所有邻居节点
			if(n == NODEID) continue;	// 不需要给本节点发送
			toSend.destid = n;
			if (TraceLevel >= 2) {
				printf("At time t=%.3f, node %d sends packet to node %d with: ", clocktime, NODEID, n);
				for (i = 0; i < numNodes; i++) {
					printf("%d ", toSend.mincost[i]);
				}
				printf("\n");
			}
			toLayer2(toSend);
		}

	} 
	printf("node%d distance table updated finished! \n", NODEID);
}


/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr ) {
    int i, j;
    int TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int NumberOfNeighbors = 0;                     // How many neighbors
    int Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt0

