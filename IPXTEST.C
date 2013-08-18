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
    T_sword16 status ;

    DebugRoutine("IPXDriverSend") ;

    /* Try getting a packet. */
    status = IPXGet((T_packetLong *)p_data) ;

    /* Did we get a packet? */
    if (status == -1)  {
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
printf("%04X\r", i) ;
        if ((i & 0x7FFF)==0)   {
            IPXSend("tesb", 5) ;
            puts("Send") ;
        }

        if (IPXGet(buffer) == TRUE)
            printf("ipx got: %4.4s\n", buffer) ;
    }
}

T_void TestC(T_void)
{
    int out = 0 ;
    char buffer[200] ;

    while (out==0)  {
        if (kbhit())  {
            printf("\nEnter> ") ;
            gets(buffer) ;

            if (stricmp(buffer, "quit") == 0)
                out = 1 ;
            else
                IPXSend(buffer, 1+strlen(buffer)) ;
        }

        if (IPXGet(buffer) == TRUE)
            printf(">> %s\n", buffer) ;
    }
}

T_void main(int argc)
{
    T_directTalkHandle handle ;
    T_byte8 arg1[80] ;
    T_sword16 status ;
    char *OurArgV[20] ;

    DebugRoutine("main (IPX Driver v1.0)") ;

    printf("Lysle's IPX Test v1.0 (%s)\n", __DATE__) ;
    puts("-------------------------------------") ;

    InitNetwork() ;

    /* Install the process communications (real to dos32). */
    handle = DirectTalkInit(
                 IPXDriverReceive,
                 IPXDriverSend,
                 IPXDriverConnect,
                 IPXDriverDisconnect,
                 DIRECT_TALK_HANDLE_BAD) ;

    printf("DirectTalk Handle 0x%08lX\n", handle) ;  fflush(stdout) ;
#if 0
#if 1
    sprintf(arg1, "%08lX", handle) ;
#ifdef COMPILE_OPTION_RUN_PROFILER
    OurArgV[0] = "WSAMPRSI.EXE" ;
    OurArgV[1] = "CONNECT.EXE" ;
    OurArgV[2] = arg1 ;
    OurArgV[3] = NULL ;
    status = spawnv(P_WAIT, "WSAMPRSI.EXE", OurArgV) ;
#else
    OurArgV[0] = "CONNECT.EXE" ;
    OurArgV[1] = arg1 ;
    OurArgV[2] = NULL ;
    status = spawnv(P_WAIT, "CONNECT.EXE", OurArgV) ;
#endif
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
    IPXDriverConnect("8820125") ;
    puts("Done with connect, now disconnect") ;
    IPXDriverDisconnect() ;
#endif
#endif
    if (argc > 1)
        TestA() ;
    else if (argc == 0)
        TestB() ;
    else
        TestC() ;
    DirectTalkFinish(handle) ;

    ShutdownNetwork() ;

    puts("Done.") ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SERDRV.C                                                */
/****************************************************************************/
