#include <stdio.h>
#include <stdlib.h>

#include <xPL.h>

static xPL_ServicePtr webgateway = NULL;
static xPL_MessagePtr xplMessage = NULL;


struct PmaxSensor {
char id[20];
char type[20];         //OK (perimeter set at boot, interior when motion is detected)
char alert[20];
char PmaxAlarmType[20];
char armed[20];                  //OK
char alarmed[20];
char lowbattery[20];             //OK
char enrolled[20];               //OK
char tampered[20];               //OK
};

struct SystemStatus {
  char status[50];
  char pmaxstatus[50];
  char readytoarm[50];
  char PmaxStatus[50];
  struct PmaxSensor sensor[31];
};

struct SystemStatus pmaxSystem;

int nbzone=0;
char zonelist[256];
char tpzonelist[256];

/*
char * xplListAt(char* element,char* xpllist, int i)
{
  element[0]=0;
  sprintf(element,"%d",i);
  
  
  char tempjsonelement[500];
  if (xpllist==NULL) return NULL;
  int length=0;
  if (strlen(xpllist)==0) return NULL;
  if (i == 0) {
    strcpy(tempjsonelement,xpllist);
    xpllist=tempjsonelement;
  }
  do  {
    xpllist++;
    if ( xpllist[0] ==',' )  {
      length++;
      if (length==i) {
        strcpy(tempjsonelement,xpllist+1);
        xpllist=tempjsonelement;
      }
    }   
  }
  while (length<(i+1) && xpllist!=NULL);  
  if (xpllist!=NULL) xpllist[0]=0;
  strcpy(element,tempjsonelement);
  if (xpllist==NULL) element[strlen(element)-1]=0; 
  return element;
} */

int XPLlistLength(char* xpllist)  {
  if (xpllist==NULL) return 0;
  if (xpllist[0]==0) return 0;
  int length=1;
  do  {
    xpllist++;
    if ( xpllist[0] ==',' ) length++;
  }
  while (xpllist[0]!=0);  
  return length;
}  

void webgatewayMessageHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue) {
 // printf( "Received a pmax Message from %s-%s.%s of type %d for %s.%s\n", 
	//  xPL_getSourceVendor(theMessage), xPL_getSourceDeviceID(theMessage), xPL_getSourceInstanceID(theMessage),
	//  xPL_getMessageType(theMessage), xPL_getSchemaClass(theMessage), xPL_getSchemaType(theMessage));
//	  printf("%s<BR>",xPL_formatMessage(theMessage));
//	printf(".");  
	  if (strcmp("gatestat",xPL_getSchemaType(theMessage))==0 )
    {
      if (xPL_getMessageNamedValue(theMessage, "status")!=NULL)
        strcpy(pmaxSystem.status    ,xPL_getMessageNamedValue(theMessage, "status"));
      if (xPL_getMessageNamedValue(theMessage, "pmaxstatus")!=NULL)
        strcpy(pmaxSystem.pmaxstatus,xPL_getMessageNamedValue(theMessage, "pmaxstatus"));
      if (xPL_getMessageNamedValue(theMessage, "readytoarm")!=NULL)
        strcpy(pmaxSystem.readytoarm,xPL_getMessageNamedValue(theMessage, "readytoarm"));
    } 
    
    
  if (strcmp("zonelist",xPL_getSchemaType(theMessage))==0 && xPL_getMessageNamedValue(theMessage, "zone-list")!=NULL )
    {
      strcpy(zonelist,xPL_getMessageNamedValue(theMessage, "zone-list"));
//      printf("%s\n",zonelist);
      nbzone=XPLlistLength(zonelist);
   //   printf("nbzone.....  %d<BR>",nbzone);   
    }
    
    if (strcmp("zoneinfo",xPL_getSchemaType(theMessage))==0 && xPL_getMessageNamedValue(theMessage, "zone-type")!=NULL && xPL_getMessageNamedValue(theMessage, "zone")!=NULL )
    {
      int zone=0;
      int i; 
      for (i=1;i<=30;i++)
      {
        if (strcmp(pmaxSystem.sensor[i].id,xPL_getMessageNamedValue(theMessage, "zone"))==0) {
          zone=i;
          break;
        }         
      }
     //printf("zinfo1 %d ",zone);  
      if (zone==0) { 
        for (i=1;i<=30;i++)
        {
          if (strcmp(pmaxSystem.sensor[i].id,"")==0) break;         
        }
        zone=i;
        strcpy(pmaxSystem.sensor[zone].id,xPL_getMessageNamedValue(theMessage, "zone"));
      //  printf("zoneid %s ",pmaxSystem.sensor[zone].id);    
    } 
    //printf("zinfo2 %d ",zone);    
      //sscanf(xPL_getMessageNamedValue(theMessage, "zone"),"%d",&zone);
      
      
      
      strcpy( pmaxSystem.sensor[zone].type , xPL_getMessageNamedValue(theMessage, "zone-type") );
    } 
   if (strcmp("zonestat",xPL_getSchemaType(theMessage))==0 && xPL_getMessageNamedValue(theMessage, "zone")!=NULL )
    {
      int zone=0;
      int i; 
      for (i=1;i<=30;i++)
      {
        if (strcmp(pmaxSystem.sensor[i].id,xPL_getMessageNamedValue(theMessage, "zone"))==0) {
          zone=i;
          break;
        }         
      } 
   //   printf("zstat1 %d ",zone);
      if (zone==0) { 
        for (i=1;i<=30;i++)
        {
          if (strcmp(pmaxSystem.sensor[i].id,"")==0) break;         
        }
        zone=i;
        strcpy(pmaxSystem.sensor[zone].id,xPL_getMessageNamedValue(theMessage, "zone"));
      //  printf("zoneid %s ",xPL_getMessageNamedValue(theMessage, "zone"));   
    }   
  //  printf("zstat2 %d ",zone);  
      
      
      
      //sscanf(xPL_getMessageNamedValue(theMessage, "zone"),"%d",&zone);
      
      if ( xPL_getMessageNamedValue(theMessage, "alert")!=NULL )
        strcpy(pmaxSystem.sensor[zone].alert,xPL_getMessageNamedValue(theMessage, "alert")); 
      if ( xPL_getMessageNamedValue(theMessage, "armed")!=NULL ) 
        strcpy(pmaxSystem.sensor[zone].armed,xPL_getMessageNamedValue(theMessage, "armed"));
      if ( xPL_getMessageNamedValue(theMessage, "tamper")!=NULL ) 
        strcpy(pmaxSystem.sensor[zone].tampered,xPL_getMessageNamedValue(theMessage, "tamper"));      
      if ( xPL_getMessageNamedValue(theMessage, "low-battery")!=NULL )
        strcpy(pmaxSystem.sensor[zone].lowbattery,xPL_getMessageNamedValue(theMessage, "low-battery"));
      if ( xPL_getMessageNamedValue(theMessage, "alarm")!=NULL )
        strcpy(pmaxSystem.sensor[zone].alarmed,xPL_getMessageNamedValue(theMessage, "alarm"));
      strcpy(pmaxSystem.sensor[zone].enrolled,"true");     
    }   
    
}



int main(void)
{
  pmaxSystem.status[0]=0;
  pmaxSystem.pmaxstatus[0]=0;
  pmaxSystem.readytoarm[0]=0;
  pmaxSystem.PmaxStatus[0]=0;
  
  int k;
  for (k=0;k<31;k++)
  {
    pmaxSystem.sensor[k].type[0]=0;
    pmaxSystem.sensor[k].alert[0]=0;
    pmaxSystem.sensor[k].PmaxAlarmType[0]=0;
    pmaxSystem.sensor[k].armed[0]=0;                  
    pmaxSystem.sensor[k].alarmed[0]=0;
    pmaxSystem.sensor[k].lowbattery[0]=0;  
    pmaxSystem.sensor[k].enrolled[0]=0;
    pmaxSystem.sensor[k].tampered[0]=0;
    pmaxSystem.sensor[k].id[0]=0;
  } 
  
//	char *data;
	long m,n;
	printf("%s%c%c\n" ,
	"Content-Type:text/html;charset=iso-8859-1",13,10);

	
  xPL_initialize(xPL_getParsedConnectionType());
  webgateway = xPL_createService("viknet", "webgateway", "default");  
  xPL_setServiceVersion(webgateway, "1.0");

  /* Add a responder for time setting */
  xPL_addServiceListener(webgateway, webgatewayMessageHandler, xPL_MESSAGE_ANY, "security", NULL, NULL);
  xPL_setServiceEnabled(webgateway, TRUE);
//  int j;
  
//  for (j=0;j<10;j++)
//  {
  /* Create a message to send */
  xplMessage = xPL_createBroadcastMessage(webgateway, xPL_MESSAGE_COMMAND);
  xPL_setSchema(xplMessage, "security", "request");
  xPL_setMessageNamedValue(xplMessage, "request", "zonelist");	
  xPL_sendMessage(xplMessage);
//  printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
  int i;
  
  
  xplMessage = xPL_createBroadcastMessage(webgateway, xPL_MESSAGE_COMMAND);
  xPL_setSchema(xplMessage, "security", "request");
  xPL_setMessageNamedValue(xplMessage, "request", "gatestat");	
  xPL_sendMessage(xplMessage);
  
  
  for (i=1;i<1000;i++)
  {
    usleep(1000);
    if (nbzone!=0) break;
    xPL_processMessages(0);
  }
  
  
  strcpy(tpzonelist,zonelist);
  char * element;
//  printf("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n");
//  printf("zone: %d, %s\n",nbzone,zonelist);
//  for (i=0;i<nbzone;i++)  {
//  char * pch;
  element = strtok (tpzonelist,",");
   while (element != NULL)
  {
//    printf ("%s\n",element);
   
   // xplListAt(element,zonelist,i);
  //  sprintf(element,"%d",i);
  //  printf("element: %s\n",element);
    xplMessage = xPL_createBroadcastMessage(webgateway, xPL_MESSAGE_COMMAND);
    xPL_setSchema(xplMessage, "security", "request");
    xPL_setMessageNamedValue(xplMessage, "request", "zoneinfo");	
    xPL_setMessageNamedValue(xplMessage, "zone", element);	
    xPL_sendMessage(xplMessage);
    
    
    xplMessage = xPL_createBroadcastMessage(webgateway, xPL_MESSAGE_COMMAND);
    xPL_setSchema(xplMessage, "security", "request");
    xPL_setMessageNamedValue(xplMessage, "request", "zonestat");	
    xPL_setMessageNamedValue(xplMessage, "zone", element);	
    xPL_sendMessage(xplMessage);
    
    
    usleep(40000);
    xPL_processMessages(0);
    element = strtok (NULL, ",");  
  }
//  printf("zonelist: %s\n",zonelist);
  for (i=0;i<10;i++)
  { 
  usleep(15000);
  xPL_processMessages(0);
  }      
  printf("{ \"status\":\"%s\",\"pmaxstatus\":\"%s\",\"readytoarm\":\"%s\",\"sensor\":[",pmaxSystem.status,pmaxSystem.pmaxstatus,pmaxSystem.readytoarm);
 
// strcpy(tpzonelist,zonelist);
//  element = strtok (tpzonelist,",");
  i=0;
//  while (element != NULL)
  char tmpbuff[200];
  char buff[6000];
  buff[0]=0;
  for (i=0;i<31;i++)
  {
//    printf ("%s\n",element);
    if ( strcmp(pmaxSystem.sensor[i].id,"")!=0 ) { 
    sprintf(tmpbuff,"{\"zone\":\"%s\",\"type\":\"%s\",\"alert\":\"%s\",\"armed\":\"%s\",\"tamper\":\"%s\",\"low-battery\":\"%s\",\"alarm\":\"%s\"},",
    pmaxSystem.sensor[i].id,
    pmaxSystem.sensor[i].type,
    pmaxSystem.sensor[i].alert,
    pmaxSystem.sensor[i].armed,
    pmaxSystem.sensor[i].tampered,
    pmaxSystem.sensor[i].lowbattery,
    pmaxSystem.sensor[i].alarmed,);
    strcat(buff,tmpbuff);
    //element = strtok (NULL, ",");
    //if (element!=NULL)
    
    //i++;
    }
  }
  buff[strlen(buff)-1]=0;
  printf("%s",buff);
  printf("]}"); 
  nbzone=0;
  zonelist[0]=0;

//}  
  
	return 0;  
}       

  
