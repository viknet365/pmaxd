#include <signal.h>
#include <xPL.h>

#define PMAXD_VERSION "1.0"

static time_t lastTimeSent = 0;
static xPL_ServicePtr pmaxdService = NULL;
static xPL_MessagePtr pmaxdMessage = NULL;



void pmaxdMessageHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue) {
  DEBUG(LOG_DEBUG,"Received a pmaxd Message from %s-%s.%s of type %d for %s.%s\n", 
	xPL_getSourceVendor(theMessage), xPL_getSourceDeviceID(theMessage), xPL_getSourceInstanceID(theMessage),
	xPL_getMessageType(theMessage), xPL_getSchemaClass(theMessage), xPL_getSchemaType(theMessage));
	DEBUG(LOG_DEBUG,"xpl message received: %s",xPL_formatMessage(theMessage));    
  DEBUG(LOG_INFO,"command: %s ", xPL_getMessageNamedValue(theMessage, "command"));
        
  if (strcmp("basic",xPL_getSchemaType(theMessage))==0 && xPL_getMessageNamedValue(theMessage, "command")!=NULL )  { 
    if (strcmp("arm-away",xPL_getMessageNamedValue(theMessage, "command"))==0 ) {
      DEBUG(LOG_INFO,"arming away.........");
      sendBuffer(&PowerlinkCommand[Pmax_ARMAWAY]);
    } 
    if (strcmp("arm-home",xPL_getMessageNamedValue(theMessage, "command"))==0 ) {
      DEBUG(LOG_INFO,"arming home........");
      sendBuffer(&PowerlinkCommand[Pmax_ARMHOME]);
    } 
    if (strcmp("disarm",xPL_getMessageNamedValue(theMessage, "command"))==0  )  {
      DEBUG(LOG_INFO,"disarming.....");
      sendBuffer(&PowerlinkCommand[Pmax_DISARM]);
    }
  }
  DEBUG(LOG_INFO,"not a security basic");
  if (strcmp("request",xPL_getSchemaType(theMessage))==0 && xPL_getMessageNamedValue(theMessage, "request")!=NULL )  { 
    DEBUG(LOG_INFO,"a security request");
    if (strcmp("gatestat",xPL_getMessageNamedValue(theMessage, "request"))==0  )  {
      DEBUG(LOG_INFO,"requesting getstat.....");
    
    
      
      sendBuffer(&PowerlinkCommand[Pmax_REQSTATUS]);
      int i;
      for(i=0;i<50;i++) {      
      serialHandler();
      }
      
      if (lastValidSerialIO<(time(NULL)-10))
      { DEBUG(LOG_INFO,"lst serial command too old");}
      else  {
      DEBUG(LOG_INFO,"lst serial recent enough");
      
      /* Create a message to send */
      pmaxdMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS); 
    
      xPL_setSchema(pmaxdMessage, "security", "gatestat");              // setschema to security.basic
  
      xPL_setMessageNamedValue(pmaxdMessage, "status", gatestat.status);
     /* if (pmaxSystem.xplalarmstatus==AlarmArmed)    
        xPL_setMessageNamedValue(pmaxdMessage, "status", "armed");
      else   
        xPL_setMessageNamedValue(pmaxdMessage, "status", "disarmed"); */
      
      xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", gatestat.pmstatus);
      /*if (pmaxSystem.xplpmaxstatus==PmaxArmedAway)
        xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", "armed-away");
      if (pmaxSystem.xplpmaxstatus==PmaxArmedHome)  
        xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", "armed-home");
      if (pmaxSystem.xplpmaxstatus==PmaxDisarmed)  
        xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", "disarmed"); */
      
      /*if (pmaxSystem.readytoarm)        
        xPL_setMessageNamedValue(pmaxdMessage, "readytoarm", "true");
      else
        xPL_setMessageNamedValue(pmaxdMessage, "readytoarm", "false");   
        */
/*           
      
      if (pmaxSystem.status==0 || pmaxSystem.status==1 || pmaxSystem.status==2) {
        xPL_setMessageNamedValue(pmaxdMessage, "status", "disarmed");
        xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", "disarmed");
      }      
      if (pmaxSystem.status==4 ||  pmaxSystem.status==10 )  {
        xPL_setMessageNamedValue(pmaxdMessage, "status", "armed");
        xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", "armed-home");
      }     
      if (pmaxSystem.status==3 || pmaxSystem.status==5 || pmaxSystem.status==11)  {
        xPL_setMessageNamedValue(pmaxdMessage, "status", "armed");
        xPL_setMessageNamedValue(pmaxdMessage, "pmaxstatus", "armed-away");
      }      
      if ( (pmaxSystem.flags & 0x01)==1 )  {
        xPL_setMessageNamedValue(pmaxdMessage, "readytoarm", "true");
      }
      else  {
        xPL_setMessageNamedValue(pmaxdMessage, "readytoarm", "false");
      } */
      
      /* Broadcast the message */
      DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdMessage));
      xPL_sendMessage(pmaxdMessage);
      }      
    }
    if (strcmp("zonelist",xPL_getMessageNamedValue(theMessage, "request"))==0  )  {
      DEBUG(LOG_INFO,"requesting zonelist.....");
  
      pmaxdMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS);
      xPL_setSchema(pmaxdMessage, "security", "zonelist");              // setschema to security.basic
      
      char zonelist[256];
      zonelist[0]=0;
      char id[10];
      int i;
      for (i=1;i<=30;i++) {
        if (gatestat.zone[i].enrolled)  {
        //sprintf(id,"%d,",i);
        strcat(zonelist,gatestat.zone[i].info.id);
        strcat(zonelist,","); 
        }
      }
      zonelist[strlen(zonelist)-1]=0;
      DEBUG(LOG_INFO,"zonelist: %s",zonelist);
      xPL_setMessageNamedValue(pmaxdMessage, "zone-list", zonelist);
      DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdMessage)); 
      xPL_sendMessage(pmaxdMessage);   
    }
     
     if (strcmp("zoneinfo",xPL_getMessageNamedValue(theMessage, "request"))==0  )  {
      int zone;
      zone=0;
      char * zonebuffer;
      zonebuffer= xPL_getMessageNamedValue(theMessage, "zone");
  //    sscanf(zonebuffer,"%d",&zone);
     int i; 
      for (i=1;i<=30;i++)
  {
//      pmaxSystem.sensor[i].type=perimeter;
      if (strcmp(gatestat.zone[i].info.id,zonebuffer)==0) break;         
  }
     zone=i; 
      
      
     DEBUG(LOG_INFO,"requesting zoneinfo for zone %s,%d.....",zonebuffer,zone);
          
      pmaxdMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS);
      xPL_setSchema(pmaxdMessage, "security", "zoneinfo");              // setschema to security.basic
      xPL_setMessageNamedValue(pmaxdMessage, "zone", zonebuffer);
      xPL_setMessageNamedValue(pmaxdMessage, "zone-type", gatestat.zone[zone].info.zonetype); 
     // if (pmaxSystem.sensor[zone].type==interior  )  xPL_setMessageNamedValue(pmaxdMessage, "zone-type", "interior");
     // if (pmaxSystem.sensor[zone].type==perimeter  )  xPL_setMessageNamedValue(pmaxdMessage, "zone-type", "perimeter");      
      DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdMessage));
      xPL_sendMessage(pmaxdMessage);   
    }
    if (strcmp("zonestat",xPL_getMessageNamedValue(theMessage, "request"))==0  )  {
      int zone;
      zone=0;
      char * zonebuffer;
      zonebuffer=xPL_getMessageNamedValue(theMessage, "zone");
      //sscanf(zonebuffer,"%d",&zone);
       int i; 
      for (i=1;i<=30;i++)
  {
      if (strcmp(gatestat.zone[i].info.id,zonebuffer)==0) break;         
  }
     zone=i; 
      DEBUG(LOG_INFO,"requesting zonestat for zone %s,%d.....",zonebuffer,zone);
          
      pmaxdMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS);
      xPL_setSchema(pmaxdMessage, "security", "zonestat");              // setschema to security.basic
      xPL_setMessageNamedValue(pmaxdMessage, "zone", zonebuffer);
      
      xPL_setMessageNamedValue(pmaxdMessage, "alert", gatestat.zone[zone].stat.alert); 
      //if (pmaxSystem.sensor[zone].state==SensorClose  )  xPL_setMessageNamedValue(pmaxdMessage, "alert", "false");
      //if (pmaxSystem.sensor[zone].state==SensorOpen  )  xPL_setMessageNamedValue(pmaxdMessage, "alert", "true");

      xPL_setMessageNamedValue(pmaxdMessage, "armed", gatestat.zone[zone].stat.armed);   
      //if (pmaxSystem.sensor[zone].armed  )  xPL_setMessageNamedValue(pmaxdMessage, "armed", "true");
      //else  xPL_setMessageNamedValue(pmaxdMessage, "armed", "false"); 
      
   
      xPL_setMessageNamedValue(pmaxdMessage, "alarm", "false");
      
      xPL_setMessageNamedValue(pmaxdMessage, "state", gatestat.zone[zone].stat.state);
      
      xPL_setMessageNamedValue(pmaxdMessage, "tamper", gatestat.zone[zone].stat.tamper);
      //if (pmaxSystem.sensor[zone].tampered  )  xPL_setMessageNamedValue(pmaxdMessage, "tamper", "true");
      //else  xPL_setMessageNamedValue(pmaxdMessage, "tamper", "false");
      
      xPL_setMessageNamedValue(pmaxdMessage, "low-battery", gatestat.zone[zone].stat.lowbattery); 
      //if (pmaxSystem.sensor[zone].lowbattery  )  xPL_setMessageNamedValue(pmaxdMessage, "low-battery", "true");
      //else  xPL_setMessageNamedValue(pmaxdMessage, "low-battery", "false");  
      DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdMessage));
      xPL_sendMessage(pmaxdMessage);
      
         
    }
  }
  DEBUG(LOG_INFO,"end message handling");
}

void shutdownHandler(int onSignal) {
  xPL_setServiceEnabled(pmaxdService, FALSE);
  xPL_releaseService(pmaxdService);
  xPL_shutdown();
  exit(0);
}

void initXpl() {
 
 if (!xPL_initialize(xPL_getParsedConnectionType())) {
    fprintf(stderr, "Unable to start xPL");
    exit(1);
  } 

  /* Initialze clock service */

  /* Create  a service for us */
  pmaxdService = xPL_createService("visonic", "powermaxplus", "default");
  xPL_setServiceVersion(pmaxdService, PMAXD_VERSION);
  
  /* Add a responder for time setting */
  xPL_addServiceListener(pmaxdService, pmaxdMessageHandler, xPL_MESSAGE_COMMAND, "security", NULL, NULL);

  

  /* Install signal traps for proper shutdown */
  signal(SIGTERM, shutdownHandler);
  signal(SIGINT, shutdownHandler);

  /* Enable the service */
    xPL_setServiceEnabled(pmaxdService, TRUE);

}

// int main(int argc, String argv[]) {
  /* Parse command line parms */
 // if (!xPL_parseCommonArgs(&argc, argv, FALSE)) exit(1);

  /* Start xPL up */
  

  /** Main Loop of Clock Action **/

//  for (;;) {
    /* Let XPL run for a while, returning after it hasn't seen any */
    /* activity in 100ms or so                                     */
    

    /* Process clock tick update checking */
    // sendClockTick();
//  }
//}
