//===========================================================================

#define NUMPACKETS      10			// max outstanding packets before loss


// setupdata_t is used as doomdata_t during setup
typedef struct
{
	short	gameid;					// so multiple games can setup at once
	short	drone;
	short	nodesfound;
	short	nodeswanted;
} T_setup ;



typedef struct IPXPacketStructure
{
	T_word16 packetCheckSum;         /* high-low */
	T_word16 packetLength;           /* high-low */
	T_byte8 packetTransportControl;
	T_byte8 packetType;

	T_byte8 dNetwork[4];            /* high-low */
	T_byte8 dNode[6];               /* high-low */
	T_byte8 dSocket[2];             /* high-low */

	T_byte8 sNetwork[4];            /* high-low */
	T_byte8 sNode[6];               /* high-low */
	T_byte8 sSocket[2];             /* high-low */
} T_IPXPacket;


typedef struct
{
	T_byte8 network[4];             /* high-low */
	T_byte8 node[6];                /* high-low */
} T_localAddr ;

typedef struct
{
	T_byte8 node[6];                /* high-low */
} T_nodeAddr ;

typedef struct ECBStructure
{
	T_word16 Link[2];                /* offset-segment */
	T_word16 ESRAddress[2];          /* offset-segment */
	T_byte8 InUseFlag;
	T_byte8 CompletionCode;
	T_word16 ECBSocket;              /* high-low */
	T_byte8 IPXWorkspace[4];        /* N/A */
	T_byte8 DriverWorkspace[12];    /* N/A */
	T_byte8 ImmediateAddress[6];    /* high-low */
	T_word16 FragmentCount;          /* low-high */

	T_word16 fAddress[2];            /* offset-segment */
	T_word16 fSize;                  /* low-high */
} T_ECB;


// time is used by the communication driver to sequence packets returned
// to DOOM when more than one is waiting

typedef struct
{
	T_ECB             ecb;
	T_IPXPacket       ipx;

	T_word32 time;
	T_byte8 data[1512];
} T_packet;



T_word16 OpenSocket(T_word16 socketNumber) ;
T_void CloseSocket(T_word16 socketNumber) ;
T_void ListenForPacket(T_ECB *ecb) ;
T_localAddr *GetLocalAddress(T_void) ;

T_void IPXSend(T_void *p_data, T_word16 size) ;
E_Boolean IPXGet(T_packetLong *p_data) ;
T_void InitNetwork(T_void) ;
T_void ShutdownNetwork(T_void) ;

