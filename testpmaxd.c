#include <stdio.h>
//#include <stdlib.h>
#include <libconfig.h> 
#include <xPL.h>
#include <stdarg.h>
//#include <errno.h>
#include <syslog.h>

#include <sys/reboot.h>
#include "debug.h" 

static xPL_ServicePtr webgateway = NULL;

int    pmaxstatus=0;

config_t cfg, *cf;
int foregroundOption = 0;
int verboseLevel = 0;


void initLog(int verboseLevel) {
  // avoid logging to syslog with a verboselevel more than info (otherwise message answer are too slow)
  // be carefull, syslogd on openwrt can take up to 600ms
  if (foregroundOption==0 & verboseLevel>5) verboseLevel=5;      
  if (foregroundOption==0)  {
    openlog ("pmaxd", LOG_PERROR  | LOG_PID | LOG_NDELAY,LOG_DAEMON);
    LOG = &vsyslog;
    LOG_setmask = &setlogmask;   
  }
  else  {
    LOG = &log_console;
    LOG_setmask = &log_console_setlogmask;
  }   
  LOG_setmask(LOG_UPTO(verboseLevel));
  DEBUG (LOG_NOTICE, "Logging initialized");
  DEBUG (LOG_NOTICE, "Verbose level: %d", verboseLevel);     
}




void parseCommandLineArgs(int argc, char **argv) {     
  char c;
 if (!xPL_parseCommonArgs(&argc, argv, FALSE)) exit(1);

  while ((c = getopt (argc, argv, "fvh")) != -1)
    switch (c)  {
      case 'f':
        foregroundOption = 1;
        break;        
      case 'v':
        verboseLevel++;
        break;             
      case 'h':
        fprintf (stderr, "usage:\n\t-f start in foreground \n\t-v enable verbose you cannot use more than vvvvv in daemon mode");
        fprintf (stderr, "\n\t-interface lo/eth0/... the network interface where xpl is listening");
        fprintf (stderr, "\n\t-xpldebug anable xpl debugging");
  
        exit(0);
      case '?':
        if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        exit(0);        
//      default:
//        return 1;
    }
  }     
  
  
  

void webgatewayMessageHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue) {
 // printf( "Received a pmax Message from %s-%s.%s of type %d for %s.%s\n", 
	//  xPL_getSourceVendor(theMessage), xPL_getSourceDeviceID(theMessage), xPL_getSourceInstanceID(theMessage),
	//  xPL_getMessageType(theMessage), xPL_getSchemaClass(theMessage), xPL_getSchemaType(theMessage));
	//  printf("%s",xPL_formatMessage(theMessage));
 //   	printf("xpl handling\n");
 //     printf("%s<BR>\n",xPL_formatMessage(theMessage));
       
    if (strcmp("gatestat",xPL_getSchemaType(theMessage))==0 )
    {
//      printf("xpl gatestat handling\n"); 
      if ( xPL_getMessageNamedValue(theMessage, "status")!=NULL && xPL_getMessageNamedValue(theMessage, "pmaxstatus")!=NULL)
         {
           pmaxstatus=1;
//     	     printf("status OK handling\n");
         }
    }
}


int main(int argc, char **argv)
{
const config_setting_t *restartscript;
cf = &cfg;
config_init(cf);
 
 
  parseCommandLineArgs(argc, argv);
 
 
  
  if (!config_read_file(cf, "/etc/pmaxd.conf")) {
            fprintf(stderr, "%s:%d - %s\n",
                config_error_file(cf),
                config_error_line(cf),
                config_error_text(cf));
            config_destroy(cf);
            return(EXIT_FAILURE);
        }

restartscript = config_lookup(cf, "restartscript");


if (config_setting_get_string(restartscript)) {
  printf("shell script for restarting is : %s \n",config_setting_get_string(restartscript));     
}
     
  int nblost=0;
  int i=0;
	pid_t pid=0, sid=0;
		
	/* Fork off the parent process */
 if  (foregroundOption==0) pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
  we can exit the parent process. */
  if (pid > 0) {   
    exit(EXIT_SUCCESS);
  }
  /* Change the file mode mask */
  umask(0);
  
   initLog(verboseLevel);
                                       
  /* Create a new SID for the child process */
   if  (foregroundOption==0) sid = setsid();
  if (sid < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    /* Log the failure */
    exit(EXIT_FAILURE);
  }
  
  /* Close out the standard file descriptors */

   if  (foregroundOption==0) {
    DEBUG (LOG_INFO, "closing std file descriptor");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }
  
  //sleep(300);
 
	xPL_initialize(xPL_getParsedConnectionType());
  webgateway = xPL_createService("viknet", "webgateway", "default");  
  xPL_setServiceVersion(webgateway, "1.0");
 
  /* Add a responder for time setting */
  xPL_addServiceListener(webgateway, webgatewayMessageHandler, xPL_MESSAGE_ANY, "security", NULL, NULL);
  xPL_setServiceEnabled(webgateway, TRUE);
  
  DEBUG (LOG_INFO, "starting main loop");
 
  xPL_MessagePtr xplMessage = NULL;
  sleep(300);
  for (;;)  // infinite loop
  {
   
    if ( pmaxstatus!=0 )
    {
     DEBUG (LOG_INFO, "starting to sleep 60 second");
    sleep(10);   
    }                     
    else
    {
     DEBUG (LOG_INFO, "starting to sleep 1 second");
    sleep(1);
    }
    pmaxstatus=0;
  
    DEBUG (LOG_INFO, "creating xpl message");
    /* Create a message to send */
    xplMessage = xPL_createBroadcastMessage(webgateway, xPL_MESSAGE_COMMAND);
    xPL_setSchema(xplMessage, "security", "request");
    xPL_setMessageNamedValue(xplMessage, "request", "gatestat");	
    
    DEBUG (LOG_INFO, "sending xpl message");
    xPL_sendMessage(xplMessage);
	
	  for (i=1;i<1000;i++)
    {
      usleep(1000);
      if (pmaxstatus != 0) {
        nblost=0;
        break;
      } 
      
      xPL_processMessages(0);
    }
	  if  (pmaxstatus==0) nblost++;
    if (nblost>3) 
    {
      system(config_setting_get_string(restartscript) );
    }
    
	}	
 return 0;  
}       
