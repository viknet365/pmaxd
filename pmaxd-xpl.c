#include <signal.h>

#define PMAXD_VERSION "1.0"

static time_t lastTimeSent = 0;
static xPL_MessagePtr pmaxdMessage = NULL;


void pmaxdMessageHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue) {

	char *xPL_command = xPL_getMessageNamedValue(theMessage, "command");
	char *pmstatus = NULL;

	DEBUG(LOG_DEBUG,"Received a pmaxd Message from %s-%s.%s of type %d for %s.%s\n", 
	xPL_getSourceVendor(theMessage), xPL_getSourceDeviceID(theMessage), xPL_getSourceInstanceID(theMessage),
	xPL_getMessageType(theMessage), xPL_getSchemaClass(theMessage), xPL_getSchemaType(theMessage));
	DEBUG(LOG_DEBUG,"xPL message received: %s",xPL_formatMessage(theMessage));    
	
	if (strcmp("basic",xPL_getSchemaType(theMessage))==0 && xPL_command != NULL )  {
	
		DEBUG(LOG_INFO,"xPL command: %s ", xPL_command);
		
		// force sending of a new pmstatus to ensure sync with alarm system
		gatestat.pmstatus = NULL;
	
		if (strcmp("arm-away", xPL_command)==0 ) {
			DEBUG(LOG_INFO,"Arming away.........");
			sendBuffer(&PowerlinkCommand[Pmax_ARMAWAY]);
			pmstatus = XplStatusPMArmingAway;
		} 
		else if (strcmp("arm-home", xPL_command)==0 ) {
			DEBUG(LOG_INFO,"Arming home........");
			sendBuffer(&PowerlinkCommand[Pmax_ARMHOME]);
			pmstatus = XplStatusPMArmingHome;
		} 
		else if (strcmp("disarm", xPL_command)==0  )  {
			DEBUG(LOG_INFO,"Disarming.....");
			sendBuffer(&PowerlinkCommand[Pmax_DISARM]);
			pmstatus = XplStatusPMDisarmed;
		}
		else {
			// handle different message format -> ON/OFF + type field
			char *xPL_type = xPL_getMessageNamedValue(theMessage, "type");
			
			if (xPL_type != NULL) {
				if (strcmp("OFF", xPL_command)==0 || strcmp("off", xPL_command)==0 || strcmp("Off", xPL_command)==0) {
					DEBUG(LOG_INFO,"Disarming.....");
					sendBuffer(&PowerlinkCommand[Pmax_DISARM]);
					pmstatus = XplStatusPMDisarmed;
				}
				else if (strcmp("ON", xPL_command)==0 || strcmp("on", xPL_command)==0 || strcmp("On", xPL_command)==0) {
					if (strcmp("arm-away", xPL_type)==0 ) {
						DEBUG(LOG_INFO,"Arming away.........");
						sendBuffer(&PowerlinkCommand[Pmax_ARMAWAY]);
						pmstatus = XplStatusPMArmingAway;
					} 
					else if (strcmp("arm-home", xPL_type)==0 ) {
						DEBUG(LOG_INFO,"Arming home........");
						sendBuffer(&PowerlinkCommand[Pmax_ARMHOME]);
						pmstatus = XplStatusPMArmingHome;
					} 
				}
			}	
		}
		
		// if something happened, send a status message
		if (pmstatus != NULL) {
			pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS);
			xPL_setSchema(pmaxdTrigMessage, "security", "gateway");
			xPL_setMessageNamedValue(pmaxdTrigMessage, "device", "pmaxplus"); 
			xPL_setMessageNamedValue(pmaxdTrigMessage, "status", pmstatus); 
			DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
			xPL_sendMessage(pmaxdTrigMessage);
		}
		
		return;
	}

	DEBUG(LOG_INFO,"not a security basic");
	if (strcmp("request",xPL_getSchemaType(theMessage))==0 && xPL_getMessageNamedValue(theMessage, "request")!=NULL )  { 
		DEBUG(LOG_INFO,"a security request");
		if (strcmp("gatestat",xPL_getMessageNamedValue(theMessage, "request"))==0  )  {
			DEBUG(LOG_INFO,"requesting getstat.....");
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
		
		if (strcmp("zonelist",xPL_getMessageNamedValue(theMessage, "request"))==0  )  {
			DEBUG(LOG_INFO,"requesting zonelist.....");

			pmaxdMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS);
			xPL_setSchema(pmaxdMessage, "security", "zonelist");              // setschema to security.basic
			
			char zonelist[200];
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

	/* Create  a service for us */
	pmaxdService = xPL_createService("visonic", "pmaxplus", "default");
	xPL_setServiceVersion(pmaxdService, PMAXD_VERSION);

	/* Add a responder for time setting */
	xPL_addServiceListener(pmaxdService, pmaxdMessageHandler, xPL_MESSAGE_COMMAND, "security", NULL, NULL);

	/* Install signal traps for proper shutdown */
	signal(SIGTERM, shutdownHandler);
	signal(SIGINT, shutdownHandler);

	/* Enable the service */
	xPL_setServiceEnabled(pmaxdService, TRUE);
}
