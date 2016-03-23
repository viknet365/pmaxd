#define  MAX_BUFFER_SIZE 256
#define  PACKET_TIMEOUT 15000
#define  MAX_IDLE_TIME 70000000
#define  IDLE_TIME 10000

#define	MAX_NAME_LENGTH 32

#define Pmax_ACK  0
#define Pmax_DISARM  3
#define Pmax_ARMHOME  4
#define Pmax_ARMAWAY  5
#define Pmax_ENROLLREPLY  10
#define Pmax_ENROLLREQUEST  9
#define Pmax_GETEVENTLOG  2

#define Pmax_NBCOMMAND 13
#define Pmax_REQSTATUS 7

char XplTrue[] = "true" ;
char XplFalse[]= "false";

char XplStateBypassed[]="bypassed";
char XplStateEnabled[] ="enabled" ;

char XplStatusDisarmed[]="disarmed";
char XplStatusArmed[]   ="armed"   ;

char XplStatusPMDisarmed[]="disarmed";
char XplStatusPMArmedAway[]="armed-away";
char XplStatusPMArmedHome[]="armed-home";
char XplStatusPMArmingHome[]="arming-home";
char XplStatusPMArmingAway[]="arming-away";

char XplTypeInterior[] ="interior";
char XplTypePerimeter[]="perimeter";
char XplType24hour[]   ="24hour";

char XplAlarmTypeBurglary[]="burglary";
char XplAlarmTypeFire[]    ="fire";
char XplAlarmTypeFlood[]   ="flood";
char XplAlarmTypeGas[]     ="gas";
char XplAlarmTypeOther[]   ="other";  

 
struct PlinkBuffer {
	unsigned char buffer[MAX_BUFFER_SIZE];
	unsigned char size;
	unsigned char description[27];
	void (*action)(struct PlinkBuffer *);
};

typedef enum {interior, perimeter, H24} PmaxSensorType;
typedef enum {burglary, fire, flood, gas, other} PmaxAlarmType;
typedef enum {SensorClose, SensorOpen, SensorViolated , SensorFire} PmaxSensorState;
typedef enum {PmaxArmedAway, PmaxArmedHome, PmaxDisarmed, PmaxArmingAway, PmaxArmingHome } PmaxStatus;
typedef enum {AlarmArmed, AlarmDisarmed } AlarmStatus;


/*struct PmaxSensor {
PmaxSensorType type;         //OK (perimeter set at boot, interior when motion is detected)
PmaxAlarmType alarmtype;
PmaxSensorState state;       //OK   (open/close only)
bool armed;                  //OK
bool alarmed;
bool lowbattery;            //OK
bool enrolled;              //OK
bool tampered;              //OK
};*/

struct Info {

	//char *alarmtype;
	//char *state;       			//OK   (open/close only)
	//char *armed;                 //OK
	//char *alarmed;
	//char *enrolled;              //OK
	char id[MAX_NAME_LENGTH];
	char *zonetype;
	char *alarmtype;               //OK
};

struct Stat {
	char *PmaxSensorType;        //OK (perimeter set at boot, interior when motion is detected)
	char *lowbattery;            //OK
	char *tamper;                //OK
	char *alert;                 //OK
	char *state;                 //OK
	char *armed;                 //OK
};

struct Zone {
	//char name[20];
	struct Info info;
	struct Stat stat;
	bool enrolled; 
};

struct GateStat {
	char *acfail;
	char *lowbattery;
	char *status;
	char *pmstatus;
	struct Zone zone[256];  
};

config_t cfg, *cf;
const char *command = NULL;

/*struct SystemStatus {
  unsigned char status;
  unsigned char flags;
  struct PmaxSensor sensor[31];
  struct Zone zone[31];  
  bool readytoarm;
  PmaxStatus xplpmaxstatus;
  AlarmStatus xplalarmstatus;
}; */

//struct SystemStatus pmaxSystem;
struct GateStat gatestat;

char PmaxLog[0xFF][MAX_BUFFER_SIZE];

char PmaxZoneUser[87][22];
char PmaxZoneEventTypes[21][22];
char PmaxLogEvents[111][43];
char SystemStateFlags[8][46];
char PmaxSystemStatus[14][12];

struct PlinkBuffer PowerlinkCommand[];

void sendBuffer(struct PlinkBuffer * Buff);  

void PmaxInit() {
	int i,usercode,count;
	const config_setting_t *zoneName;

	if (config_lookup_int(cf, "usercode", &usercode))
	for (i=2;i<=6;i++)
	{
		PowerlinkCommand[i].buffer[4]=usercode>>8;
		PowerlinkCommand[i].buffer[5]=usercode & 0x00FF ;
	}
	
	for (i=0;i<=255;i++)
	{
		// pmaxSystem.sensor[i].type=perimeter;
		gatestat.zone[i].info.zonetype=XplTypePerimeter;
		gatestat.zone[i].info.alarmtype=XplAlarmTypeBurglary;
		sprintf(gatestat.zone[i].info.id,"%d",i);      
	}

	zoneName = config_lookup(cf, "zonename");
	count = config_setting_length(zoneName);

	DEBUG(LOG_INFO, "%d zone(s) in config:", count);
	for (i = 0; i < count; i++) {  
		if (config_setting_get_string_elem(zoneName, i)!=0x00)       
		strncpy(gatestat.zone[i+1].info.id, config_setting_get_string_elem(zoneName, i), MAX_NAME_LENGTH - 1);
		DEBUG(LOG_INFO, "%d - %s", i + 1, gatestat.zone[i+1].info.id);
	}
	
	int result = config_lookup_string(cf, "extprog", &command);

	sendBuffer(&PowerlinkCommand[Pmax_REQSTATUS]);
}


void PmaxEnroll(struct PlinkBuffer  * Buff)
{
	sendBuffer(&PowerlinkCommand[Pmax_ENROLLREPLY]);
	DEBUG(LOG_INFO,"Enrolling.....");
}

void PmaxAck(struct PlinkBuffer  * Buff)
{

}

void PmaxAccessDenied(struct PlinkBuffer  * Buff)
{
	DEBUG(LOG_INFO,"Access denied");
}

void PmaxEventLog(struct PlinkBuffer  * Buff)
{
	char logline[MAX_BUFFER_SIZE];
	char tpzone[MAX_BUFFER_SIZE];
	logline[0]=0;

	sendBuffer(&PowerlinkCommand[Pmax_ACK]);
	if (Buff->buffer[9]>0 & Buff->buffer[9]<31 )
		strcpy(tpzone,gatestat.zone[Buff->buffer[9]].info.id);
	else
		strcpy(tpzone,PmaxZoneUser[Buff->buffer[9]]);   


	sprintf(logline,"event number:%d/%d at %d:%d:%d %d/%d/%d %s:%s",
		Buff->buffer[2],Buff->buffer[1],  // event number amoung number
		Buff->buffer[5],Buff->buffer[4],Buff->buffer[3],  //hh:mm:ss
		Buff->buffer[6],Buff->buffer[7],2000+Buff->buffer[8], //day/month/year
		tpzone,       //zone
		PmaxLogEvents[Buff->buffer[10]]
		);  
	strcpy (PmaxLog[Buff->buffer[2]],logline);
	DEBUG(LOG_NOTICE,"Event log:%s",logline);
}
 
void PmaxStatusUpdate(struct PlinkBuffer  * Buff)
{
	sendBuffer(&PowerlinkCommand[Pmax_ACK]);
	DEBUG(LOG_INFO,"pmax status update")   ;
}

void PmaxStatusChange(struct PlinkBuffer  * Buff)
{
	sendBuffer(&PowerlinkCommand[Pmax_ACK]);
	DEBUG(LOG_INFO,"PmaxStatusChange %s %s",PmaxZoneUser[Buff->buffer[3]],PmaxLogEvents[Buff->buffer[4]]); 
}

void PmaxStatusUpdateZoneTamper(struct PlinkBuffer  * Buff)
{
	sendBuffer(&PowerlinkCommand[Pmax_ACK]);
	DEBUG(LOG_INFO,"Status Update : Zone active/tampered");
	int i=0;
	char * ZoneBuffer;
	ZoneBuffer=Buff->buffer+3;
	for (i=1;i<=30;i++)
	{
		int byte=(i-1)/8;
		int offset=(i-1)%8;
		if (ZoneBuffer[byte] & 1<<offset) DEBUG(LOG_INFO,"Zone %d is active",i );

	}
	ZoneBuffer=Buff->buffer+7;  
	for (i=1;i<=30;i++)
	{
		int byte=(i-1)/8;
		int offset=(i-1)%8;
		if (ZoneBuffer[byte] & 1<<offset)
		{
			DEBUG(LOG_INFO,"Zone %d is tampered",i );
			// pmaxSystem.sensor[i].tampered=ZoneBuffer[byte] & 1<<offset;
			gatestat.zone[i].stat.tamper=XplTrue;
		}
		else
		{
			gatestat.zone[i].stat.tamper=XplFalse;    
		}      
	} 
}
 
void PmaxStatusUpdateZoneBypassed(struct PlinkBuffer  * Buff)
{  
	sendBuffer(&PowerlinkCommand[Pmax_ACK]);
	DEBUG(LOG_INFO,"Status Update : Zone Enrolled/Bypassed");
	int i=0;
	char * ZoneBuffer;
	ZoneBuffer=Buff->buffer+3;
	for (i=1;i<=30;i++)
	{
		int byte=(i-1)/8;
		int offset=(i-1)%8;
		if (ZoneBuffer[byte] & 1<<offset)
		DEBUG(LOG_INFO,"Zone %d is enrolled",i );
		//   pmaxSystem.sensor[i].enrolled=ZoneBuffer[byte] & 1<<offset;
		gatestat.zone[i].enrolled=ZoneBuffer[byte] & 1<<offset;
		
	}
	ZoneBuffer=Buff->buffer+7;  
	for (i=1;i<=30;i++)
	{
		int byte=(i-1)/8;
		int offset=(i-1)%8;
		if (ZoneBuffer[byte] & 1<<offset) {
			DEBUG(LOG_INFO,"Zone %d is bypassed",i );
			//   pmaxSystem.sensor[i].bypassed=ZoneBuffer[byte] & 1<<offset;
			gatestat.zone[i].stat.state=XplStateBypassed;
		}
		else {
			gatestat.zone[i].stat.state=XplStateEnabled;
		}       
	}
}

 
void PmaxStatusUpdatePanel(struct PlinkBuffer  * Buff)    
{
	char tpbuff[MAX_BUFFER_SIZE];
	char tpbuff1[MAX_BUFFER_SIZE];
	int pmaxSystemstatus = Buff->buffer[3];
	static short prev_status = 0;
	char *prev_gate_status = gatestat.status;
	char *prev_gate_pmstatus = gatestat.pmstatus;

	sendBuffer(&PowerlinkCommand[Pmax_ACK]);

	tpbuff[0]=0;

	// system status changed?
	if (prev_status == (short) *(Buff->buffer + 4)) {
		int f;
	
		sprintf(tpbuff, "System status: heart beat");
		DEBUG(LOG_INFO,"%s", tpbuff);

		// timestamping heartbeat file
		f = creat("/tmp/pmaxd.heartbeat", S_IRWXU);
		close(f);
		
		return;
	}
	prev_status = (short) *(Buff->buffer + 4);

	// system status
	sprintf(tpbuff,"System status: 0x%02x %s - Flags : 0x%02x", Buff->buffer[3], PmaxSystemStatus[pmaxSystemstatus], Buff->buffer[4]);    
	int i=0;
	for (i=0;i<8;i++)
	if (Buff->buffer[4] & 1<<i) {
		sprintf(tpbuff1," - %s", SystemStateFlags[i]);
		strcat(tpbuff,tpbuff1);
	}      
	//  pmaxSystem.status=Buff->buffer[3];
	//  pmaxSystem.flags=Buff->buffer[4];

	// if disarmed or exit delay or exit delay
	if (pmaxSystemstatus==0 ) {
		// pmaxSystem.xplalarmstatus=AlarmDisarmed;
		// pmaxSystem.xplpmaxstatus=PmaxDisarmed;
		gatestat.pmstatus=XplStatusPMDisarmed;
		gatestat.status=XplStatusDisarmed;
		for (i=1;i<=30;i++) {
			if (gatestat.zone[i].enrolled) {
				//pmaxSystem.sensor[i].armed=false;
				gatestat.zone[i].stat.armed=XplFalse;
			}
		}   
	}

	// if disarmed or exit delay or exit delay
	if ( pmaxSystemstatus==1 ) {
		// pmaxSystem.xplalarmstatus=AlarmDisarmed;
		// pmaxSystem.xplpmaxstatus=PmaxDisarmed;
		gatestat.pmstatus=XplStatusPMArmingHome;
		gatestat.status=XplStatusDisarmed;
	}

	// if disarmed or exit delay or exit delay
	if ( pmaxSystemstatus==2) {
		// pmaxSystem.xplalarmstatus=AlarmDisarmed;
		// pmaxSystem.xplpmaxstatus=PmaxDisarmed;
		gatestat.pmstatus=XplStatusPMArmingAway;
		gatestat.status=XplStatusDisarmed;
	}

	// if armed home or armed home bypass      
	if (pmaxSystemstatus==4 ||  pmaxSystemstatus==10 )  {
		//pmaxSystem.xplalarmstatus=AlarmArmed;
		//pmaxSystem.xplpmaxstatus=PmaxArmedHome;
		gatestat.pmstatus=XplStatusPMArmedHome;    
		gatestat.status=XplStatusArmed;
		for (i=1;i<=30;i++) {
			//if (pmaxSystem.sensor[i].enrolled && pmaxSystem.sensor[i].type==perimeter) pmaxSystem.sensor[i].armed=true; 
			if (gatestat.zone[i].enrolled && gatestat.zone[i].info.zonetype==XplTypePerimeter) gatestat.zone[i].stat.armed=XplTrue; 
		}
	}

	// if entry delay or armed away or armed away bypass    
	if (pmaxSystemstatus==3 || pmaxSystemstatus==5 || pmaxSystemstatus==11)  {
		//   pmaxSystem.xplalarmstatus=AlarmArmed;
		//   pmaxSystem.xplpmaxstatus=PmaxArmedAway;
		gatestat.pmstatus=XplStatusPMArmedAway; 
		gatestat.status=XplStatusArmed;
		for (i=1;i<=30;i++) {
			if (gatestat.zone[i].enrolled)    {
				//       pmaxSystem.sensor[i].armed=true;
				gatestat.zone[i].stat.armed=XplFalse;
			}
		}
	}
	
	// if status has changed, sending an xPL-trig message
	if (prev_gate_status != gatestat.status) {
		pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_TRIGGER);
		xPL_setSchema(pmaxdTrigMessage, "security", "gateway");
		xPL_setMessageNamedValue(pmaxdTrigMessage, "device", "pmaxplus"); 
		xPL_setMessageNamedValue(pmaxdTrigMessage, "event", gatestat.status); 
		DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
		xPL_sendMessage(pmaxdTrigMessage);
	}
	
	// if pmstatus has changed, sending an xPL-stat message
	if (prev_gate_pmstatus != gatestat.pmstatus) {
		pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_STATUS);
		xPL_setSchema(pmaxdTrigMessage, "security", "gateway");
		xPL_setMessageNamedValue(pmaxdTrigMessage, "device", "pmaxplus"); 
		xPL_setMessageNamedValue(pmaxdTrigMessage, "status", gatestat.pmstatus); 
		DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
		xPL_sendMessage(pmaxdTrigMessage);
	}
	
	// pmaxSystem.readytoarm=((pmaxSystem.flags & 0x01)==1);

	// if system state flag says it is a zone event (bit 5 of system flag)  
	if (Buff->buffer[4] & 1<<5) {
		
		/* Create a message and send it - Only for motion detection (change of status is processed in PmaxStatusUpdateZoneBat) */
		if (Buff->buffer[6] == 5) {
			pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_TRIGGER);
			xPL_setSchema(pmaxdTrigMessage, "security", "zone");
			sprintf(tpbuff1,"zone.%d", Buff->buffer[5]);
			xPL_setMessageNamedValue(pmaxdTrigMessage, "device", tpbuff1); 
			xPL_setMessageNamedValue(pmaxdTrigMessage, "status", PmaxZoneEventTypes[3]); 
			DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
			xPL_sendMessage(pmaxdTrigMessage);
			pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_TRIGGER);
			xPL_setSchema(pmaxdTrigMessage, "security", "zone");
			sprintf(tpbuff1,"zone.%d", Buff->buffer[5]);
			xPL_setMessageNamedValue(pmaxdTrigMessage, "device", tpbuff1); 
			xPL_setMessageNamedValue(pmaxdTrigMessage, "status", PmaxZoneEventTypes[4]); 
			DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
			xPL_sendMessage(pmaxdTrigMessage);
		}
		
		sprintf(tpbuff1," - Zone %d (%s): %s", Buff->buffer[5], gatestat.zone[Buff->buffer[5]].info.id, PmaxZoneEventTypes[Buff->buffer[6]]);

		if  ( 0<Buff->buffer[5] && Buff->buffer[5]<30 && Buff->buffer[6]==5 )
		{
			DEBUG(LOG_INFO, "Setting Zone %d to interior",Buff->buffer[5] );
			gatestat.zone[Buff->buffer[5]].info.zonetype=XplTypeInterior;
			DEBUG(LOG_INFO, "Zone %d type: %s",Buff->buffer[5],gatestat.zone[Buff->buffer[5]].info.zonetype );
			//   pmaxSystem.sensor[Buff->buffer[5]].type=interior;
		}
		
		if  ( 0<Buff->buffer[5] && Buff->buffer[5]<30 && Buff->buffer[6]==0x0F )
		{
			gatestat.zone[Buff->buffer[5]].info.zonetype=XplType24hour;
			gatestat.zone[Buff->buffer[5]].info.alarmtype=XplAlarmTypeFire;  
			//   pmaxSystem.sensor[Buff->buffer[5]].type=interior;
		} 
		strcat(tpbuff,tpbuff1);
	}
	
	// if Alarm-event, send xpl message & trigger external program
	if (Buff->buffer[4] & 1<<7) {

		pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_TRIGGER);
		xPL_setSchema(pmaxdTrigMessage, "security", "zone");
		sprintf(tpbuff1, "zone.%d", i);
		xPL_setMessageNamedValue(pmaxdTrigMessage, "device", tpbuff1); 
		xPL_setMessageNamedValue(pmaxdTrigMessage, "event", "alarm");
		xPL_setMessageNamedValue(pmaxdTrigMessage, "zone", gatestat.zone[i].info.id);
		DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
		xPL_sendMessage(pmaxdTrigMessage);
	
		if (command != NULL) {
		
			DEBUG(LOG_INFO,"Executing %s arg:%s", command, tpbuff);
			
			signal(SIGCHLD, SIG_IGN); // To avoid having to wait for child process
			pid_t pid = fork();
			if (pid == 0) {
				execlp(command, command, tpbuff, (char *) NULL);
				exit(0);
			}
		}
	}
	
	DEBUG(LOG_INFO,"%s", tpbuff);
}
 
 
void PmaxStatusUpdateZoneBat(struct PlinkBuffer  * Buff)
{
	sendBuffer(&PowerlinkCommand[Pmax_ACK]);
	DEBUG(LOG_INFO,"Status Update : Zone state/Battery");
	int i=0;
	char * ZoneBuffer;
	char tpbuff1[MAX_BUFFER_SIZE];
	
	ZoneBuffer=Buff->buffer+3;
	for (i=1;i<=30;i++)
	{
		int byte=(i-1)/8;
		int offset=(i-1)%8;
		if (ZoneBuffer[byte] & 1<<offset) {

			if (gatestat.zone[i].stat.alert != XplTrue) {
				DEBUG(LOG_INFO,"Zone %d is changing status to open", i);

				//    pmaxSystem.sensor[i].state=SensorOpen;
				gatestat.zone[i].stat.alert = XplTrue;

				/* Create a message and send it */
				pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_TRIGGER);
				xPL_setSchema(pmaxdTrigMessage, "security", "zone");
				sprintf(tpbuff1, "zone.%d", i);
				xPL_setMessageNamedValue(pmaxdTrigMessage, "device", tpbuff1); 
				xPL_setMessageNamedValue(pmaxdTrigMessage, "status", PmaxZoneEventTypes[3]);
				xPL_setMessageNamedValue(pmaxdTrigMessage, "zone", gatestat.zone[i].info.id);
				DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
				xPL_sendMessage(pmaxdTrigMessage);
			}
		} else {
			if (gatestat.zone[i].stat.alert != XplFalse) {
				DEBUG(LOG_INFO,"Zone %d is changing status to closed", i);

				//    pmaxSystem.sensor[i].state=SensorClose;
				gatestat.zone[i].stat.alert = XplFalse;

				/* Create a message and send it */
				pmaxdTrigMessage = xPL_createBroadcastMessage(pmaxdService, xPL_MESSAGE_TRIGGER);
				xPL_setSchema(pmaxdTrigMessage, "security", "zone");
				sprintf(tpbuff1, "zone.%d", i);
				xPL_setMessageNamedValue(pmaxdTrigMessage, "device", tpbuff1); 
				xPL_setMessageNamedValue(pmaxdTrigMessage, "status", PmaxZoneEventTypes[4]);
				xPL_setMessageNamedValue(pmaxdTrigMessage, "zone", gatestat.zone[i].info.id);
				DEBUG(LOG_DEBUG,"xpl message sent: %s",xPL_formatMessage(pmaxdTrigMessage));
				xPL_sendMessage(pmaxdTrigMessage);
			}
		}     
	}
	ZoneBuffer=Buff->buffer+7;  
	for (i=1;i<=30;i++)
	{
		int byte=(i-1)/8;
		int offset=(i-1)%8;
		if (ZoneBuffer[byte] & 1<<offset) {
			DEBUG(LOG_INFO,"Zone %d battery is low", i);
			gatestat.zone[i].stat.lowbattery=XplTrue;
		}
		else  {
			gatestat.zone[i].stat.lowbattery=XplFalse;
		}           
		//  pmaxSystem.sensor[i].lowbattery=ZoneBuffer[byte] & 1<<offset;       
	}
} 
    
 
 



struct PlinkBuffer PowerlinkCommand[] =
{
 {{0x02,0x43                                                  },2  ,"Acknowledgement"      ,NULL},
 {{0x46,0xF8,0x00,0x00,0x59,0x11,0x30,0x12,0x20,0xFF,0xFF     },11 ,"Set date and time"    ,NULL},
 {{0xA0,0x00,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Get event log"        ,NULL},
 {{0xA1,0x00,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Disarm"               ,NULL},
 {{0xA1,0x00,0x00,0x04,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Arm-home"             ,NULL},
 {{0xA1,0x00,0x00,0x05,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Arm-away"             ,NULL},
 {{0xA1,0x00,0x00,0x14,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Arm home instantly"   ,NULL},
 {{0xA2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Request status update",NULL},
 {{0xAA,0x12,0x34,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x43},12 ,"Enable bypass"        ,NULL},
 {{0xAA,0x12,0x34,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Disable bypass"       ,NULL},
 {{0xAB,0x0A,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Enroll reply"         ,NULL}
};


struct PlinkBuffer PmaxCommand[Pmax_NBCOMMAND] =
{
 {{0x08,0x43                                                  },2  ,"Access denied"              ,&PmaxAccessDenied},
 {{0xA0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Event log"                  ,&PmaxEventLog},
 {{0xA5,0xFF,0x02,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone Battery" ,&PmaxStatusUpdateZoneBat},
 {{0xA5,0xFF,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone tamper"  ,&PmaxStatusUpdateZoneTamper},
 {{0xA5,0xFF,0x04,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Panel"        ,&PmaxStatusUpdatePanel},
 {{0xA5,0xFF,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone Bypassed",&PmaxStatusUpdateZoneBypassed},
 {{0xA7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Panel status change"        ,&PmaxStatusChange},
 {{0xAB,0x0A,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Enroll request"             ,&PmaxEnroll},
 {{0x02,0x43                                                  },2  ,"Acknowledgement"            ,&PmaxAck}
};


char PmaxSystemStatus[14][12] = {
"Disarmed"     ,
"Exit Delay" ,
"Exit Delay" ,
"Entry Delay",
"Armed Home" ,
"Armed Away" ,
"User Test"  ,
"Downloading",
"Programming",
"Installer"  ,
"Home Bypass",
"Away Bypass",
"Ready"      ,
"Not Ready"
};


char SystemStateFlags[8][46] = {
"Ready",
"Alert-in-Memory",
"Trouble",
"Bypass-On",
"Last-10-sec-delay",
"Zone-event",
"Arm/disarm-event",
"Alarm-event"
};


char  PmaxLogEvents[111][43] = {            
"None"                                      ,
"Interior Alarm"                            ,
"Perimeter Alarm"                           ,
"Delay Alarm"                               ,
"24h Silent Alarm"                          ,
"24h Audible Alarm"                         ,
"Tamper"                                    ,
"Control Panel Tamper"                      ,
"Tamper Alarm"                              ,
"Tamper Alarm"                              ,
"Communication Loss"                        ,
"Panic From Keyfob"                         ,
"Panic From Control Panel"                  ,
"Duress"                                    ,
"Confirm Alarm"                             ,
"General Trouble"                           ,
"General Trouble Restore"                   ,
"Interior Restore"                          ,
"Perimeter Restore"                         ,
"Delay Restore"                             ,
"24h Silent Restore"                        ,
"24h Audible Restore"                       ,
"Tamper Restore"                            ,
"Control Panel Tamper Restore"              ,
"Tamper Restore"                            ,
"Tamper Restore"                            ,
"Communication Restore"                     ,
"Cancel Alarm"                              ,
"General Restore"                           ,
"Trouble Restore"                           ,
"Not used"                                  ,
"Recent Close"                              ,
"Fire"                                      ,
"Fire Restore"                              ,
"No Active"                                 ,
"Emergency"                                 ,
"No used"                                   ,
"Disarm Latchkey"                           ,
"Panic Restore"                             ,
"Supervision (Inactive)"                    ,
"Supervision Restore (Active)"              ,
"Low Battery"                               ,
"Low Battery Restore"                       ,
"AC Fail"                                   ,
"AC Restore"                                ,
"Control Panel Low Battery"                 ,
"Control Panel Low Battery Restore"         ,
"RF Jamming"                                ,
"RF Jamming Restore"                        ,
"Communications Failure"                    ,
"Communications Restore"                    ,
"Telephone Line Failure"                    ,
"Telephone Line Restore"                    ,
"Auto Test"                                 ,
"Fuse Failure"                              ,
"Fuse Restore"                              ,
"Keyfob Low Battery"                        ,
"Keyfob Low Battery Restore"                ,
"Engineer Reset"                            ,
"Battery Disconnect"                        ,
"1-Way Keypad Low Battery"                  ,
"1-Way Keypad Low Battery Restore"          ,
"1-Way Keypad Inactive"                     ,
"1-Way Keypad Restore Active"               ,
"Low Battery"                               ,
"Clean Me"                                  ,
"Fire Trouble"                              ,
"Low Battery"                               ,
"Battery Restore"                           ,
"AC Fail"                                   ,
"AC Restore"                                ,
"Supervision (Inactive)"                    ,
"Supervision Restore (Active)"              ,
"Gas Alert"                                 ,
"Gas Alert Restore"                         ,
"Gas Trouble"                               ,
"Gas Trouble Restore"                       ,
"Flood Alert"                               ,
"Flood Alert Restore"                       ,
"X-10 Trouble"                              ,
"X-10 Trouble Restore"                      ,
"Arm Home"                                  ,
"Arm Away"                                  ,
"Quick Arm Home"                            ,
"Quick Arm Away"                            ,
"Disarm"                                    ,
"Fail To Auto-Arm"                          ,
"Enter To Test Mode"                        ,
"Exit From Test Mode"                       ,
"Force Arm"                                 ,
"Auto Arm"                                  ,
"Instant Arm"                               ,
"Bypass"                                    ,
"Fail To Arm"                               ,
"Door Open"                                 ,
"Communication Established By Control Panel",
"System Reset"                              ,
"Installer Programming"                     ,
"Wrong Password"                            ,
"Not Sys Event"                             ,
"Not Sys Event"                             ,
"Extreme Hot Alert"                         ,
"Extreme Hot Alert Restore"                 ,
"Freeze Alert"                              ,
"Freeze Alert Restore"                      ,
"Human Cold Alert"                          ,
"Human Cold Alert Restore"                  ,
"Human Hot Alert"                           ,
"Human Hot Alert Restore"                   ,
"Temperature Sensor Trouble"                ,
"Temperature Sensor Trouble Restore"        
};


char  PmaxZoneEventTypes[21][22] = {
"None"                 ,
"Tamper Alarm"         ,
"Tamper Restore"       ,
"Open"                 ,
"Closed"               ,
"Violated (Motion)"    ,
"Panic Alarm"          ,
"RF Jamming"           ,
"Tamper Open"          ,
"Communication Failure",
"Line Failure"         ,
"Fuse"                 ,
"Not Active"           ,
"Low Battery"          ,
"AC Failure"           ,
"Fire Alarm"           ,
"Emergency"            ,
"Siren Tamper"         ,
"Siren Tamper Restore" ,
"Siren Low Battery"    ,
"Siren AC Fail"
};

char  PmaxZoneUser[87][22] = {
"System"               ,
"Zone 1"               ,
"Zone 2"               ,
"Zone 3"               ,
"Zone 4"               ,
"Zone 5"               ,
"Zone 6"               ,
"Zone 7"               ,
"Zone 8"               ,
"Zone 9"               ,
"Zone 10"              ,
"Zone 11"              ,
"Zone 12"              ,
"Zone 13"              ,
"Zone 14"              ,
"Zone 15"              ,
"Zone 16"              ,
"Zone 17"              ,
"Zone 18"              ,
"Zone 19"              ,
"Zone 20"              ,
"Zone 21"              ,
"Zone 22"              ,
"Zone 23"              ,
"Zone 24"              ,
"Zone 25"              ,
"Zone 26"              ,
"Zone 27"              ,
"Zone 28"              ,
"Zone 29"              ,
"Zone 30"              ,
"Keyfob1"              ,
"Keyfob2"              ,
"Keyfob3"              ,
"Keyfob4"              ,
"Keyfob5"              ,
"Keyfob6"              ,
"Keyfob7"              ,
"Keyfob8"              ,
"User1"                ,
"User2"                ,
"User3"                ,
"User4"                ,
"User5"                ,
"User6"                ,
"User7"                ,
"User8"                ,
"Wireless Commander1"  ,
"Wireless Commander2"  ,
"Wireless Commander3"  ,
"Wireless Commander4"  ,
"Wireless Commander5"  ,
"Wireless Commander6"  ,
"Wireless Commander7"  ,
"Wireless Commander8"  ,
"Wireless Siren1"      ,
"Wireless Siren2"      ,
"2Way Wireless Keypad1",
"2Way Wireless Keypad2",
"2Way Wireless Keypad3",
"2Way Wireless Keypad4",
"X10-1"                ,
"X10-2"                ,
"X10-3"                ,
"X10-4"                ,
"X10-5"                ,
"X10-6"                ,
"X10-7"                ,
"X10-8"                ,
"X10-9"                ,
"X10-10"               ,
"X10-11"               ,
"X10-12"               ,
"X10-13"               ,
"X10-14"               ,
"X10-15"               ,
"PGM"                  ,
"GSM"                  ,
"Powerlink"            ,
"Proxy Tag1"           ,
"Proxy Tag2"           ,
"Proxy Tag3"           ,
"Proxy Tag4"           ,
"Proxy Tag5"           ,
"Proxy Tag6"           ,
"Proxy Tag7"           ,
"Proxy Tag8"
};
