/****************************************************************************/
/*    FILE:  SERDRV.C                                                       */
/****************************************************************************/

#include "standard.h"
#include <process.h>
#include <errno.h>
#include <time.h>
#include <conio.h>

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

#if 0
    if (CommCheckClientAndServerExist() == FALSE)  {
        printf("Breaking line ... ") ;  fflush(stdout) ;
        CommSetActivePortN(0) ;
        delay(2000) ;
        CommSendData("+++", 3) ;
        delay(2000) ;
		sprintf(call, "ATZ\r", p_dial) ;
        puts("OK") ;

        if (stricmp(p_dial, "ANSWER") == 0)  {
            puts("Waiting for caller.") ;

            /* Read in the OK from the file. */
            while (CommGetReadBufferLength())  {
                c = CommReadByte() ;
                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
            }

            /* Wait for CONNECT response. */
            c = 0 ;
            do {
                if (CommGetReadBufferLength() > 0)  {
                    c = CommReadByte() ;
                }
                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
            } while (c != 'C') ;
        } else {
            puts("Connecting to server.") ;

            timeStart = TickerGet()+ (45 * CLK_TCK) ;
            while (CommGetReadBufferLength())  {
                c = CommReadByte() ;

                if (TickerGet() > timeStart)  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_TIMED_OUT) ;
                    DebugEnd() ;
                    return ;
                }

                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
            }

            /* Dial the number. */
            DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_DIALING) ;
            CommSendData(call, strlen(call)) ;

            c = 0 ;
            do {
                if (CommGetReadBufferLength() > 0)
                    c = CommReadByte() ;
                if (TickerGet() > timeStart)  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_TIMED_OUT) ;
                    DebugEnd() ;
                    return ;
                }
                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
            } while (c != 'O') ;

            do {
                if (CommGetReadBufferLength() > 0)  {
                    c = CommReadByte() ;
                }
                if (TickerGet() > timeStart)  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_TIMED_OUT) ;
                    DebugEnd() ;
                    return ;
                }
                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
		    } while ((c != '\n') && (c != '\r')) ;

            puts("Modem initialized.  Dialing now.") ;
		    sprintf(call, "ATDT%s\r", p_dial) ;
            CommSendData(call, strlen(call)) ;

            do {
                if (CommGetReadBufferLength() > 0)  {
                    c = CommReadByte() ;
                }
                if (TickerGet() > timeStart)  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_TIMED_OUT) ;
                    DebugEnd() ;
                    return ;
                }
                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
            } while ((c != 'C') && (c != 'B')) ;

            d = c ;
            do {
                if (CommGetReadBufferLength() > 0)  {
                    c = CommReadByte() ;
                }
                if (TickerGet() > timeStart)  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_TIMED_OUT) ;
                    DebugEnd() ;
                    return ;
                }
                if (kbhit())  {
                    DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_ABORTED) ;
                    DebugEnd() ;
                    return ;
                }
		    } while ((c != '\n') && (c != '\r')) ;

            /* Check to see if we were given a busy signal. */
            if (d == 'B')  {
                puts("IPX DRIVER: BUSY") ;
                DirectTalkSetLineStatus(DIRECT_TALK_LINE_STATUS_BUSY) ;
                DebugEnd() ;
                return ;
            }
        }
    }

#endif
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

#if 0
T_void TestA(T_void)
{
    T_word32 i ;

    T_byte8 test[80] = "test" ;
    T_byte8 buffer[180] ;

    puts("I'm tesa") ;
    for (i=0; (!kbhit()); i++)  {
        if ((i & 0x7FFF)==0)   {
            IPXSend("tesa", 5) ;
            puts("Send") ;
        }

        if (IPXGet(buffer) == TRUE)
            printf("ipx got: %4.4s\n", buffer) ;
    }
}

T_void TestB(T_void)
{
    T_word32 i ;

    T_byte8 test[80] = "test" ;
    T_byte8 buffer[180] ;

    puts("I'm tesb") ;
    for (i=0; (!kbhit()); i++)  {
        if ((i & 0x7FFF)==0)   {
            IPXSend("tesb", 5) ;
            puts("Send") ;
        }

        if (IPXGet(buffer) == TRUE)
            printf("ipx got: %4.4s\n", buffer) ;
    }
}
#endif

T_void main(int argc, char *argv[])
{
    T_directTalkHandle handle ;
    T_byte8 arg1[80] ;
    T_sword16 status ;
    char *OurArgV[50] ;
    T_word16 i ;
    T_localAddr *p_addr ;

    DebugRoutine("main (IPX Driver v1.0)") ;

    printf("USA IPX Driver v1.01 (%s)\n", __DATE__) ;
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
