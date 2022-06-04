#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ******************************************************************
    ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

    This code should be used for PA2, unidirectional or bidirectional
    data transfer protocols (from A to B. Bidirectional transfer of data
    is for extra credit and is not required).  Network properties:
    - one way network delay averages five time units (longer if there
        are other messages in the channel for GBN), but can be larger
    - packets can be corrupted (either the header or the data portion)
        or lost, according to user-defined probabilities
    - packets will be delivered in the order in which they were sent
        (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/************************************************
这个版本的框架有些太老旧了，我把他们稍微改了一下
************************************************/

struct event {
    float evtime;           // event time
    int evtype;             // event type code
    int eventity;           // entity where event occurs
    struct pkt *pktptr;     // pointer to packet (if any) assoc w/ this event
    struct event *prev;
    struct event *next;
};

/* function prototypes. I think this is a fantastic bug in fact.*/
void init();
void generate_next_arrival();
void insertevent(struct event *p);
void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer3(int AorB, struct pkt);
void tolayer5(int AorB, char datasent[20]);

#define FILE_NAME "../RDT/log/sw_trace.txt"
#define WriteIn(format, ...) \
        do {\
            FILE *file;\
            file = fopen(FILE_NAME, "a+");\
            fprintf(file, format,  ##__VA_ARGS__);\
            fclose(file);\
        } while(0)

float A_inter = 20.0; // 重发时间间隔
enum state {READY, WAIT}; // 规定sender的两种状态
struct A_sender
{
    struct pkt A_buffer;
    enum state A_state;
    int next_seqnum;
    int wait_acknum;
} A;
int B_expectednum;


int getChecksum(struct pkt packet) {
    int sum = 0;
    sum += (packet.seqnum + packet.acknum);
    for(int i = 0; i < 20; i++) {
        sum += packet.payload[i];
    }
    return sum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    struct pkt out_pkt;
    memcpy(out_pkt.payload, message.data, 20);
    char temp[20];
    for(int i = 0; i < 20; i++) {
        if(out_pkt.payload[i] == '`') break;
        else temp[i] = out_pkt.payload[i];
    }
    printf("[Sender] Application data \"%s\" generated.\n", out_pkt.payload);
    WriteIn("[Sender] Application data \"%s\" generated.\n", out_pkt.payload);
    if(A.A_state == WAIT) {
        printf("[Sender] The above application data discarded since waiting for ACK\n");
        WriteIn("[Sender] The above application data discarded since waiting for ACK\n", 0);
        return;
    } else {
        out_pkt.acknum = A.wait_acknum;
        out_pkt.seqnum = A.next_seqnum;
        out_pkt.checksum = getChecksum(out_pkt);
        A.wait_acknum = (A.wait_acknum + 1) % 2;
        A.next_seqnum = (A.next_seqnum + 1) % 2;
        A.A_state = WAIT;
        A.A_buffer = out_pkt;
        starttimer(0, A_inter);
        tolayer3(0, out_pkt);
        printf("[Sender] Packet %d sent. seqnum: %d, checksum: %d\n", out_pkt.seqnum, out_pkt.seqnum, out_pkt.checksum);
        WriteIn("[Sender] Packet %d sent. seqnum: %d, checksum: %d\n", out_pkt.seqnum, out_pkt.seqnum, out_pkt.checksum);
        return;
    }
}

void B_output(struct msg message)  /* need be completed only for extra credit */
{

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    int checksum = getChecksum(packet);
    if(checksum != packet.checksum) {
        printf("[Sender] Corrupt packet received.\n");
        WriteIn("[Sender] Corrupt packet received.\n", 0);
        return;
    }
    if(packet.acknum == A.wait_acknum) {
        printf("[Sender] ACK %d received.\n", packet.seqnum);
        WriteIn("[Sender] ACK %d received.\n", packet.seqnum);
        stoptimer(0);
        A.A_state = READY;
        return;
    } else {
        printf("[Sender] Corrupted packet Received At %d\n", packet.acknum);
        WriteIn("[Sender] Corrupted packet Received At %d\n", packet.acknum);
        return;
    }
}

/* called when A's timer goes off */
int A_timerinterrupt()
{
    printf("[Sender] Timeout. Re-sending packet %d\n", A.A_buffer.seqnum);
    WriteIn("[Sender] Timeout. Re-sending packet %d\n", A.A_buffer.seqnum);
    tolayer3(0, A.A_buffer);
    starttimer(0, A_inter);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    A.A_state = READY;
    A.next_seqnum = 0;
    A.wait_acknum = 0;
    memset(&A.A_buffer, 0, sizeof(A.A_buffer));
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    struct pkt out_pkt; // 回传的确认包
    struct msg message;
    memset(out_pkt.payload, 0, sizeof(out_pkt.payload));
    out_pkt.seqnum = packet.seqnum;
    int checksum = getChecksum(packet);

    if(checksum != packet.checksum) {
        printf("[Receiver] Corrupt packet received.\n");
        WriteIn("[Receiver] Corrupt packet received.\n", 0);
        out_pkt.acknum = B_expectednum;
        out_pkt.checksum = getChecksum(out_pkt);
        tolayer3(1, out_pkt);
        printf("[Receiver] Re-sending ACK %d.\n", out_pkt.seqnum);
        WriteIn("[Receiver] Re-sending ACK %d.\n", out_pkt.seqnum);
        return;
    }
    if(packet.seqnum != B_expectednum) {
        printf("[Receiver] Corrupt packet received.\n");
        WriteIn("[Receiver] Corrupt packet received.\n", 0);
        out_pkt.acknum = B_expectednum;
        out_pkt.checksum = getChecksum(out_pkt);
        tolayer3(1, out_pkt);
        printf("[Receiver] Re-sending ACK %d.\n", out_pkt.seqnum);
        WriteIn("[Receiver] Re-sending ACK %d.\n", out_pkt.seqnum);
        return;
    }
    printf("[Receiver] Packet %d received.\n", packet.seqnum);
    WriteIn("[Receiver] Packet %d received.\n", packet.seqnum);
    printf("[Receiver] Data \"%s\" handed over to application layer.\n", packet.payload);
    WriteIn("[Receiver] Data \"%s\" handed over to application layer.\n", packet.payload);
    B_expectednum = (B_expectednum + 1) % 2;
    out_pkt.acknum = B_expectednum;
    out_pkt.checksum = getChecksum(out_pkt);
    tolayer3(1, out_pkt);
    printf("[Receiver] ACK %d sent.\n", out_pkt.seqnum);
    WriteIn("[Receiver] ACK %d sent.\n", out_pkt.seqnum);
    memcpy(message.data, packet.payload, 20);
    tolayer5(1, message.data);
}

/* called when B's timer goes off */
void B_timerinterrupt()
{
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    B_expectednum = 0;
}


/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */ 
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */   
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

int main()
{
    struct event *eventptr;
    struct msg  msg2give;
    struct pkt  pkt2give;

    int i,j;
    char c; 

    init();
    A_init();
    B_init();

    while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
            goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
            evlist->prev=NULL;
        if (TRACE>=2) {
            printf("\nEVENT time: %f,",eventptr->evtime);
            printf("  type: %d",eventptr->evtype);
            WriteIn("\nEVENT time: %f,",eventptr->evtime);
            WriteIn("  type: %d",eventptr->evtype);
            if (eventptr->evtype==0)
                {printf(", timerinterrupt  ");WriteIn(", timerinterrupt  ", 0);}
            else if (eventptr->evtype==1) {
                printf(", fromlayer5 ");
                WriteIn(", fromlayer5 ", 0);
            }
            else {
                printf(", fromlayer3 ");
                WriteIn(", fromlayer3 ", 0);
            }
                
            printf(" entity: %d\n",eventptr->eventity);
            WriteIn(" entity: %d\n",eventptr->eventity);
        }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim == nsimmax)
	        break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {

            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */    
            j = nsim % 26; 
            for (i=0; i<20; i++)  
                msg2give.data[i] = 97 + j;
            if (TRACE>2) {
                printf("          MAINLOOP: data given to student: ");
                WriteIn("          MAINLOOP: data given to student: ", 0);
                for (i=0; i<20; i++) {
                    printf("%c", msg2give.data[i]);
                    WriteIn("%c", msg2give.data[i]);
                } 
                WriteIn("\n", 0);
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A) 
                A_output(msg2give);  
            else
                B_output(msg2give);  
        } else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)  
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity ==A) {     /* deliver packet by calling */
                A_input(pkt2give);  /* appropriate entity */
            }          
            else {
                B_input(pkt2give);
            }

	        free(eventptr->pktptr);          /* free the memory for packet */
        } else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) 
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else {
            printf("INTERNAL PANIC: unknown event type \n");
            WriteIn("INTERNAL PANIC: unknown event type \n", 0);
        }
        free(eventptr);
    }

terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
    WriteIn(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
}



void init()                         /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    WriteIn("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n", 0);
    printf("Enter the number of messages to simulate: ");
    scanf("%d",&nsimmax);
    WriteIn("Enter the number of messages to simulate: %d\n", nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f",&lossprob);
    WriteIn("Enter  packet loss probability [enter 0.0 for no loss]:%f\n", lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f",&corruptprob);
    WriteIn("Enter packet corruption probability [0.0 for no corruption]:%f\n", corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f",&lambda);
    WriteIn("Enter average time between messages from sender's layer5 [ > 0.0]:%f\n", lambda);
    printf("Enter TRACE:");
    scanf("%d",&TRACE);
    WriteIn("Enter TRACE:%d\n\n", TRACE);

    srand(9999);              /* init random number generator */
    sum = 0.0;                /* test random number generator for students */
    for (i=0; i<1000; i++)
        sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
    avg = sum/1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n" ); 
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(-1);
        }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time=0.0;                    /* initialize time to 0.0 */
    generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() 
{
    double mmm = 32767;   /* largest int  - MACHINE DEPENDENT!!!!!!!!  32767 or 2147483647   */
    float x;                   /* individual students may need to change mmm */ 
    x = rand()/mmm;            /* x should be uniform in [0,1] */
    return(x);
}  

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival()
{
    double x,log(),ceil();
    struct event *evptr;
    //char *malloc(); I can't understand.
    // float ttime;
    // int tempint;

    if (TRACE>2) {
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
        WriteIn("          GENERATE NEXT ARRIVAL: creating new arrival\n", 0);
    }
    
    x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                                /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime =  time + x;
    evptr->evtype =  FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand()>0.5) )
        evptr->eventity = B;
        else
        evptr->eventity = A;
    insertevent(evptr);
} 


void insertevent(struct event *p)
{
    struct event *q,*qold;

    if (TRACE>2) {
        printf("            INSERTEVENT: time is %lf\n",time);
        printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
        WriteIn("            INSERTEVENT: time is %lf\n",time);
        WriteIn("            INSERTEVENT: future time will be %lf\n",p->evtime);  
        }
    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q==NULL) {   /* list is empty */
            evlist=p;
            p->next=NULL;
            p->prev=NULL;
    }
    else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
            qold=q; 
        if (q==NULL) {   /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q==evlist) { /* front of list */
            p->next=evlist;
            p->prev=NULL;
            p->next->prev=p;
            evlist = p;
        }
        else {     /* middle of list */
            p->next=q;
            p->prev=q->prev;
            q->prev->next=p;
            q->prev=p;
        }
    }
}

void printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  WriteIn("--------------\nEvent List Follows:\n", 0);
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    WriteIn("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
  WriteIn("--------------\n", 0);
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB) /* A or B is trying to stop timer */
{
    struct event *q,*qold;

    if (TRACE>2) {
        printf("          STOP TIMER: stopping timer at %f\n",time);
        WriteIn("          STOP TIMER: stopping timer at %f\n",time);
    }
        
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q=evlist; q!=NULL ; q = q->next) 
        if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
        /* remove this event */
        if (q->next==NULL && q->prev==NULL)
                evlist=NULL;         /* remove first and only event on list */
            else if (q->next==NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q==evlist) { /* front of list - there must be event after */
                q->next->prev=NULL;
                evlist = q->next;
            }
            else {     /* middle of list */
                q->next->prev = q->prev;
                q->prev->next =  q->next;
            }
        free(q);
        return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
    WriteIn("Warning: unable to cancel your timer. It wasn't running.\n", 0);
}


void starttimer(int AorB,float increment) /* A or B is trying to stop timer */
{

    struct event *q;
    struct event *evptr;
    // char *malloc();

    if (TRACE>2) {
        printf("          START TIMER: starting timer at %f\n",time);
        WriteIn("          START TIMER: starting timer at %f\n",time);
    }
        
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q=evlist; q!=NULL ; q = q->next)  
        if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
            printf("Warning: attempt to start a timer that is already started\n");
            WriteIn("Warning: attempt to start a timer that is already started\n", 0);
            return;
        }
    
    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime =  time + increment;
    evptr->evtype =  TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
void tolayer3(int AorB, struct pkt packet) /* A or B is trying to stop timer */
{
    struct pkt *mypktptr;
    struct event *evptr,*q;
    // char *malloc();
    float lastime, x, jimsrand();
    int i;


    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)  {
        nlost++;
        if (TRACE>0) {
            printf("          TOLAYER3: packet being lost\n");
            WriteIn("          TOLAYER3: packet being lost\n", 0);
        }    
        return;
    }  

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */ 
    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i=0; i<20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE>2)  {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
        mypktptr->acknum,  mypktptr->checksum);
        WriteIn("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
        mypktptr->acknum,  mypktptr->checksum);
        for (i=0; i<20; i++) {
            printf("%c",mypktptr->payload[i]);
            WriteIn("%c",mypktptr->payload[i]);
        }
            
        WriteIn("\n", 0);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
    evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
    /* finally, compute the arrival time of packet at the other end.
    medium can not reorder, so make sure packet arrives between 1 and 10
    time units after the latest arrival time of packets
    currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q=evlist; q!=NULL ; q = q->next) 
        if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
        lastime = q->evtime;
    evptr->evtime =  lastime + 1 + 9*jimsrand();
    


    /* simulate corruption: */
    if (jimsrand() < corruptprob)  {
        ncorrupt++;
        if ( (x = jimsrand()) < .75)
        mypktptr->payload[0]='Z';   /* corrupt payload */
        else if (x < .875)
        mypktptr->seqnum = 999999;
        else
        mypktptr->acknum = 999999;
        if (TRACE>0) {
            printf("          TOLAYER3: packet being corrupted\n");
            WriteIn("          TOLAYER3: packet being corrupted\n", 0);
        }    
        
        }  

    if (TRACE>2) {
        printf("          TOLAYER3: scheduling arrival on other side\n");
        WriteIn("          TOLAYER3: scheduling arrival on other side\n", 0);
    }  
        
    insertevent(evptr);
} 

void tolayer5(int AorB, char datasent[20])
{
    int i;  
    if (TRACE>2) {
        printf("          TOLAYER5: data received: ");
        WriteIn("          TOLAYER5: data received: ", 0);
        for (i=0; i<20; i++) {
            printf("%c",datasent[i]);
            WriteIn("%c",datasent[i]);
        }  
        WriteIn("\n", 0);
        printf("\n");
    }
}
