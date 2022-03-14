




/**************************************************************************
***************************************************************************

   tpac.c   -  main process




  
***************************************************************************
Notes:

**************************************************************************/

#include <sys/socket.h>

// #include <idblib.h>
//#include "feed_common.h"
#include "tpac.h"
//#include "dvi_def.h"

//extern char **environ;
//extern char OutputBuffer[MAX_DATA_SIZE];

//extern int errno;
//extern char *sys_errlist[];

//extern STATUS_INFO     idb_CurrentStatus;
//extern struct conf tpac_config;
//extern struct 	ErrDump idb_ErrDump;
//extern char *idb_DebugLevels[];


/* Exit Handler */
/* NB. on_exit() is not used, as it's not portable */

void ProcessExit(ExitCode, signal)
int ExitCode, signal;
{

    tpacLog(ERR_INFO, "==============================================");
	tpacLog(ERR_SPECIAL, "*** %s TERMINATING ***", tpac_config.Long_name);
    tpacLog(ERR_SPECIAL, "*** Exiting on code %d", ExitCode);

    
    tpacLog(EXITCODE, "");

    exit(ExitCode);

}

static	void SignalHandler (sig)
int sig;
{
	int terminate = 0;

    switch (sig)
	{
		case SIGALRM:
		case SIGHUP:
		case SIGPIPE:
		case SIGURG:
		case SIGTSTP:
		case SIGCONT:
		case SIGCHLD:
		case SIGIO:
		case SIGXCPU:
		case SIGXFSZ:
		case SIGVTALRM:
		case SIGPROF:
		case SIGWINCH:
		case SIGUSR1:
		case SIGUSR2:
		break;
		case SIGINT:
		case SIGQUIT:
		case SIGILL:
		case SIGTRAP:
		case SIGABRT:
		case SIGEMT:
		case SIGFPE:
		case SIGKILL:
		case SIGBUS:
		case SIGSEGV:
		case SIGSYS:
		case SIGTERM:
		case SIGSTOP:
		case SIGTTIN:
		case SIGTTOU:
		default:
			tpacLog(ERR_SPECIAL,"!!! Server going down on signal %d...",
						sig);
			terminate = 1;
			break;

	}

    if (terminate)
    {
		ProcessExit(999, sig);
    }

}



/*
 * Main routine
 * 
 * (C) Copyright Market Arts Software 1993
 */

main(argc, argv)
    int argc;
    char *argv[];
{
    int status, i, ii;
	
    /* Initialize the feed structures */

    NumPages = NUM_PAGES;
    TickSeconds = 2;
    MsgBufferLength = DVI_MAX_FRAME_SIZE;
  	strcpy (SocketName, "TULLET");
  	strcpy (ProcessName, "tullet_lmg");
  	strcpy (BaudRate, "19200");

		/* Set up our signals */

    for (i = 1; i < 32; i++)
	{
		if ((i == SIGKILL) || (i == SIGSTOP))
			continue;
		if ((i == SIGURG) || (i == SIGWINCH) ||
			 (i == SIGTSTP) || (i == SIGCONT))
			signal(i, SIG_DFL);
		else
			signal(i, SignalHandler);
	}

	/* Initialize IDB Library */

	//idbInitLib(IDB_VERSION, IDB_TaT_TYPE);
	//idbExitFunc = ProcessExit;

	
	//tpacLog(ERR_SPECIAL,"*** Trace Debug Level set to %d",
			tpac_config.DebugLevel);

	ii = 0;
	while(idb_DebugLevels[ii] != (char *)0)
	{
		tpacLog(ERR_INFO,"%02d %-35s %s",
				ii,
				idb_DebugLevels[ii],
				DebugLevelSet(ii) ? "ON" : "OFF");
		ii++;
	}

    

	tpacLog(ERR_SPECIAL, "*** Configuration complete");
	tpacLog(ERR_SPECIAL, "*** HOT STANDBY MODE %s",(tpac_config.LoadBalance == 1) ? "ENABLED" : "DISABLED");

    

    /* Initialize the server process */
    strcpy(LogBuffer, "/tmp/");
    strcat(LogBuffer, SocketName);
    status = tci_init_server(LogBuffer, AF_UNIX, &ListenSocket);

    if (status < 0)
	{
		tpacLog(ERR_FATAL, "!!! tci_init_server for %s failed...", LogBuffer);
		ProcessExit(310, 15);
	}

    /* Add the listen callback to receive a connection request */
    /* from the line manager process which will be forked next */

    status = tci_addinput(ListenSocket, listen_callback, 0, 0);
    if (status < 0)
    {
		sprintf(LogBuffer, "!!! Server add input failed %d", status);
		tpacLog(ERR_FATAL, LogBuffer);
		ProcessExit(320, 15);
    }
	else
		tpacLog(ERR_INFO,"SSL:Listen callback added at %d", ListenSocket);

    
    
    /* Fork the line handler process */
    ForkLmgr();

	

   

    /* Process events */

    status = feed_mainloop();

    if (status < 0)
    {
		sprintf(LogBuffer, "!!! Server main loop failed %d", status);
		tpacLog(ERR_FATAL, LogBuffer);
		ProcessExit(330, 15);
    }

}				/* end of main() routine */
