#include "standard.h"

#define	MAXNETNODES		     8           /* Maximum players at any time. */

static T_localAddr G_localAddr ;
//static unsigned short G_enterIPX[2];
static unsigned int G_socketID = 0x869C ;
static T_packet G_packetArray[NUMPACKETS];
static T_nodeAddr G_nodeAddressArray[MAXNETNODES+1];

T_word32 localtime=0;		// for time stamp in packets
T_word32 remotetime;
T_nodeAddr		remoteadr;			// set by each GetPacket

char *hex = "0123456789abcdef";

#define swap16(v)  (((v&0xFF)<<8) | ((v&0xFF00)>>8))

void PrintAddress (T_nodeAddr *adr, char *str)
{
	int	i;

	for (i=0 ; i<6 ; i++)
	{
		*str++ = hex[adr->node[i]>>4];
		*str++ = hex[adr->node[i]&15];
	}
	*str = 0;
}

T_void Error(char *p_str, ...)
{
    printf(p_str, ...) ;
}

T_word16 OpenSocket(T_word16 socketNumber)
{
    union REGS regs ;

	regs.x.bx = 0;                      /* Open socket command. */
	regs.h.al = 0;  /* longevity */
	regs.x.dx = swap16(socketNumber);
	int86(0x7A, &regs, &regs);
	if (regs.h.al)
		Error ("OpenSocket: 0x%x",regs.h.al);
    printf("Opened socket 0x%04X\n", swap16(regs.x.dx)) ;

	return swap16(regs.x.dx);
}


T_void CloseSocket(T_word16 socketNumber)
{
    union REGS regs ;

	regs.x.bx = 1;                      /* Close socket command. */
	regs.x.dx = swap16(socketNumber);
	int86(0x7A,&regs,&regs);
}

T_void ListenForPacket(T_ECB *ecb)
{
    union REGS regs ;
    struct SREGS sregs ;

    segread(&sregs) ;
	regs.x.si = FP_OFF(ecb);
	sregs.es = FP_SEG(ecb);
	regs.x.bx = 4;                      /* Listen command */
    regs.x.ax = 0 ;

	int86x(0x7a, &regs, &regs, &sregs);
	if (regs.h.al)
		Error ("ListenForPacket: 0x%x",regs.h.al);
}

T_localAddr *GetLocalAddress(T_void)
{
    union REGS regs ;
    struct SREGS sregs ;
    char string[80] ;


    segread(&sregs) ;
	regs.x.si = FP_OFF(&G_localAddr);   /* Place to store address. */
	sregs.es = FP_SEG(&G_localAddr);
	regs.x.bx = 9;           /* Service get local address */

	int86x(0x7a, &regs, &regs, &sregs) ;
	if (regs.h.al)
		Error("Get inet addr: 0x%x",regs.h.al);

    PrintAddress((T_nodeAddr *)G_localAddr.node, string) ;
    printf("Address: %s\n", string) ;

    return &G_localAddr ;
}

//E_Boolean IPXGet(T_packetLong *p_data)
//{
//    return FALSE ;
//}

T_void InitNetwork(T_void)
{
    union REGS regs ;
    struct SREGS sregs ;

	T_sword16 i, j;

    /* Get the IPX functional address. */
    segread(&sregs) ;
	regs.x.ax = 0x7a00;
	int86x (0x2f, &regs, &regs, &sregs);
	if (regs.h.al != 0xff)
		Error("IPX not detected\n");

//	G_enterIPX[0] = regs.x.di;
//	G_enterIPX[1] = sregs.es;

    /* Allocate a socket for sending and receiving. */
	G_socketID = OpenSocket(G_socketID);

	GetLocalAddress();

    /* Set up the ECBs */
	memset(
        G_packetArray,
        0,
        sizeof(G_packetArray));

	for (i=1; i<NUMPACKETS; i++)
	{
		G_packetArray[i].ecb.InUseFlag = 0x1d;
		G_packetArray[i].ecb.ECBSocket = swap16(G_socketID) ;
		G_packetArray[i].ecb.FragmentCount = 1;
		G_packetArray[i].ecb.fAddress[0] = FP_OFF(&G_packetArray[i].ipx);
		G_packetArray[i].ecb.fAddress[1] = FP_SEG(&G_packetArray[i].ipx);
		G_packetArray[i].ecb.fSize = sizeof(T_packet)-sizeof(T_ECB);

		ListenForPacket (&G_packetArray[i].ecb);
	}

    /* Set up the sending ECB. */
	memset (&G_packetArray[0],0,sizeof(G_packetArray[0]));

	G_packetArray[0].ecb.ECBSocket = swap16(G_socketID) ;
	G_packetArray[0].ecb.FragmentCount = 1;
	G_packetArray[0].ecb.fAddress[0] = FP_OFF(&G_packetArray[0].ipx);
	G_packetArray[0].ecb.fAddress[1] = FP_SEG(&G_packetArray[0].ipx);
    G_packetArray[0].ecb.fSize = sizeof(T_packet) - sizeof(T_ECB) ;
    for (i=0; i<6; i++)
        G_packetArray[0].ecb.ImmediateAddress[i] = 0xFF ;

	for (i=0 ; i<4 ; i++)
		G_packetArray[0].ipx.dNetwork[i] = G_localAddr.network[i];

	for (i=0 ; i<6 ; i++)
		G_packetArray[0].ipx.dNode[i] = 0xFF ;

    G_packetArray[0].ipx.packetCheckSum = 0xFFFF ;

	G_packetArray[0].ipx.dSocket[0] = G_socketID>>8;
	G_packetArray[0].ipx.dSocket[1] = G_socketID&255;

    /* We are at local node 0. */
	for (i=0 ; i<6 ; i++)
		G_nodeAddressArray[0].node[i] = G_localAddr.node[i];

    for (i=0; i<6; i++)
        G_packetArray[0].ipx.sNetwork[i] = G_localAddr.network[i] ;

	for (j=0 ; j<6 ; j++)
		G_packetArray[0].ipx.sNode[j] = G_localAddr.node[j] ;

    G_packetArray[0].ipx.sSocket[0] = G_socketID>>8 ;
    G_packetArray[0].ipx.sSocket[1] = G_socketID&255 ;

    /* Broad cast node. */
	for (i=0 ; i<6 ; i++)
		G_nodeAddressArray[MAXNETNODES].node[i] = 0xff;

}

T_void ShutdownNetwork(T_void)
{
	CloseSocket(G_socketID);
}

#define MAXLONG 0x7FFFFFFFL

unsigned short ShortSwap (unsigned short i)
{
	return ((i&255)<<8) + ((i>>8)&255);
}

E_Boolean IPXGet(T_packetLong *p_packet)
{
	T_sword16       packetnum;
	T_sword16       i, j;
	T_word32		besttic;
	T_packet		*packet;
    T_word16        datalength ;

    /* if multiple packets are waiting, return them in order by time */
	besttic = MAXLONG;
	packetnum = -1;
	for ( i = 1 ; i < NUMPACKETS ; i++)
	{
		if (G_packetArray[i].ecb.InUseFlag)
		{
			continue;
		}

		if (G_packetArray[i].time < besttic)
		{
			besttic = G_packetArray[i].time;
			packetnum = i;
		}
	}

	if (besttic == MAXLONG)
		return FALSE ;                           // no G_packetArray

//puts("Got packet") ;
	packet = &G_packetArray[packetnum];

	remotetime = besttic;

    /* Got a good packet. */
	if (packet->ecb.CompletionCode)
		Error ("GetPacket: ecb.ComletionCode = 0x%x",packet->ecb.CompletionCode);

    /* Set remote address to the sender of the packet. */
	memcpy(&remoteadr, packet->ipx.sNode, sizeof(remoteadr));
/*
printf("recv: ") ;
for (i=0; i<6; i++)
  printf("<%02X>", remoteadr.node[i]) ;
puts("") ;
*/

// copy out the data
//	doomcom.datalength = ShortSwap(packet->ipx.PacketLength) - 38;
    datalength = ShortSwap(packet->ipx.packetLength) ;
//printf("length = %d\n", datalength) ;
    if (datalength > LONG_PACKET_LENGTH)
        datalength = LONG_PACKET_LENGTH ;
	memcpy (p_packet, &packet->data, datalength);

// repost the ECB
	ListenForPacket (&packet->ecb);

    /* Check to see if this is a packet we sent. */
    for (i=0; i<6; i++)
        if (packet->ipx.sNode[i] != G_localAddr.node[i])
            return TRUE ;

	return FALSE;
}

T_void IPXSend(T_void *p_data, T_word16 size)
{
    union REGS regs ;
    struct SREGS sregs ;
	T_sword16 j ;

    /* find a free packet buffer to use */
	while (G_packetArray[0].ecb.InUseFlag)
	{
	}

    /* set the time */
	G_packetArray[0].time = localtime++;

    /* set the address */
/*
	for (j=0 ; j<6 ; j++)
		G_packetArray[0].ipx.dNode[j] = 0 ;
	for (j=0 ; j<6 ; j++)
		G_packetArray[0].ipx.sNode[j] = G_localAddr.node[j] ;
    for (j=0; j<4; j++)
        G_packetArray[0].ipx.sNetwork[j] = G_localAddr.network[j] ;
*/
#if 0
    for (j=0; j<6; j++)
        G_packetArray[0].ecb.ImmediateAddress[j] =
//		    G_nodeAddressArray[destination].node[j];
            0xFF ;
#endif
    memcpy(
        G_packetArray[0].ecb.ImmediateAddress,
        DirectTalkGetDestination(),
        6) ;
    memcpy(
        G_packetArray[0].ipx.dNode,
        DirectTalkGetDestination(),
        6) ;
/*
printf("ecb: ") ;
for (j=0; j<6; j++)
  printf("[%02X]", G_packetArray[0].ecb.ImmediateAddress[j]) ;
printf("\n") ;

printf("ipx: ") ;
for (j=0; j<6; j++)
  printf("[%02X]", G_packetArray[0].ipx.dNode[j]) ;
printf("\n") ;  fflush(stdout) ;
*/

    /* set the length (ipx + time + datalength) */
	G_packetArray[0].ecb.fSize = sizeof(T_IPXPacket) + 4 + size + 4;

    /* put the data into an ipx packet */
	memcpy(&G_packetArray[0].data, p_data, size);

    /* send the packet */
    segread(&sregs) ;
	regs.x.si = FP_OFF(&G_packetArray[0]);
	sregs.es = FP_SEG(&G_packetArray[0]);
	regs.x.bx = 3;                          /* Send packet command */

	int86x (0x7a, &regs, &regs, &sregs);

	if (regs.h.al)
		Error ("SendPacket: 0x%x",regs.h.al);
}
