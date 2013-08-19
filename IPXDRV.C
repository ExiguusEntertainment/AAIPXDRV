/****************************************************************************/
/*    FILE:  SERDRV.C                                                       */
/****************************************************************************/

#include "standard.h"
#include <process.h>
#include <errno.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>

/* The following option turns on the use of the profiler program. */
//#define COMPILE_OPTION_RUN_PROFILER

#define TickerGet() clock()

static E_Boolean G_isConnected = FALSE ;

T_void IPXDriverReceive(
           T_void *p_data,
           T_byte8 size)
{
    T_word16 i ;

    DebugRoutine("IPXDriverReceive") ;

    /* Just send the data out the port. */
    IPXSend(p_data, size) ;

    DebugEnd() ;
}

T_void IPXDriverSend(
           T_void *p_data,
           T_byte8 *p_size,
           E_Boolean *p_anyData)
{
    E_Boolean status ;

    DebugRoutine("IPXDriverSend") ;

    /* Try getting a packet. */
    status = IPXGet((T_packetLong *)p_data) ;

    /* Did we get a packet? */
    if (status == FALSE)  {
        /* No packet. */
        *p_size = 0;
        *p_anyData = FALSE ;
    } else {
        /* Yes, got a packet. */
        *p_size = sizeof(T_packetLong) ;
printf("Yes, got a packet. Size set to %d\n", *p_size);
        *p_anyData = TRUE ;
//puts("got packet") ; fflush(stdout) ;
    }

    DebugEnd() ;
}

T_void IPXDriverConnect(T_void *p_data)
{
    T_word32 timeStart ;
    T_byte8 call[80] ;
    T_word16 c ;
    T_word16 d = 0 ;
    T_byte8 *p_dial ;

    p_dial = p_data ;

    DebugRoutine("IPXDriverConnect") ;

    InitNetwork() ;

    puts("IPX DRIVER: CONNECTED") ;

    /* Must of connect, no problems. */
    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_CONNECTED) ;
    G_isConnected = TRUE ;

    DebugEnd() ;
}

T_void IPXDriverDisconnect(T_void)
{
    DebugRoutine("IPXDriverDisconnect") ;

    ShutdownNetwork() ;

    DebugEnd() ;
}

T_void main(int argc, char *argv[])
{
    T_directTalkHandle handle ;
    T_byte8 arg1[80] ;
    T_sword16 status ;
    char *OurArgV[50] ;
    T_word16 i ;
    T_localAddr *p_addr ;

    DebugRoutine("main (IPX Driver v1.0)") ;

    printf("USA IPX Driver v1.02 (%s)\n", __DATE__) ;
    puts("----------------------------------") ;

    InitNetwork() ;

    /* Install the process communications (real to dos32). */
    handle = DirectTalkInit(
                 IPXDriverReceive,
                 IPXDriverSend,
                 IPXDriverConnect,
                 IPXDriverDisconnect,
                 DIRECT_TALK_HANDLE_BAD) ;

    /* Transfer the local address. */
    p_addr = GetLocalAddress() ;
    DirectTalkSetUniqueAddress((T_directTalkUniqueAddress *)(p_addr->node)) ;
//    printf("Unique address: ") ;
//    DirectTalkPrintAddress(stdout, (T_directTalkUniqueAddress *)(p_addr->node)) ;
    puts(".") ;

    /* Declare this to be an IPX driver. */
    DirectTalkSetServiceType(DIRECT_TALK_IPX) ;

    /* Start in broadcast message mode. */
    DirectTalkSetDestinationAll() ;

    printf("DirectTalk Handle 0x%08lX\n", handle) ;  fflush(stdout) ;
#if 1
    sprintf(arg1, "%08lX", handle) ;
    OurArgV[0] = argv[1] ;
//printf("arg 0: %s\n", argv[1]) ;
    for (i=1; i<argc; i++)  {
        OurArgV[i-1] = argv[i] ;
//printf("arg %d: %s\n", i, argv[i]) ;
    }
    OurArgV[i-1] = arg1 ;
//printf("arg %d: %s\n", i, arg1) ;
    OurArgV[i] = NULL ;

    status = spawnvp(P_WAIT, argv[1], OurArgV) ;

    if (status == -1)  {
        switch(errno)  {
            case ENOENT:
                puts("Driver Failure:  EXECUTABLE NOT FOUND") ;  fflush(stdout) ;
//                exit(1) ;
                break ;
            case ENOMEM:
                puts("Driver Failure:  NOT ENOUGH MEMORY") ;
//                exit(1) ;
                break ;
            defualt:
                printf("Driver Failure:  UNKNOWN ERROR %d\n", errno) ;
//                exit(1) ;
                break ;
        }
    }
#else
    SerialDriverConnect("8820125") ;
    puts("Done with connect, now disconnect") ;
    SerialDriverDisconnect() ;
#endif
    DirectTalkFinish(handle) ;
    IPXDriverDisconnect() ;

    puts("Done.") ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SERDRV.C                                                */
/****************************************************************************/
