#include <stdio.h>
#include <stdlib.h>

#include <xPL.h>

static xPL_ServicePtr webgateway = NULL;
static xPL_MessagePtr xplMessage = NULL;


char * unencode(char *ch)
{
	char *p1 = ch;
	char *p2 = ch;
	if (ch==NULL) return NULL;
  	
	while(*p1 != 0) {
    if(*p1 == '+')  {
      *p1 = ' ';
    }      
    else {
      if(*p1 == '%') {
        int code;
        if(sscanf(p1+1, "%2x", &code) != 1) code = '?';
        *p2 = code;
        p1 +=2;
      }     
      else  {
        *p2 = *p1;     
      }
    }
    p1++;
    p2++;
  }
  *p2=0;
  return ch;
}


char * stripSpace (char * ch)
{
	char *p1 = ch;
	char *p2 = ch;
	if (ch==NULL) return NULL;
	while(*p1 != 0) {
		if( isspace(*p1)) {
			++p1;
		}
		else   
			*p2++ = *p1++; 
	}
	*p2 = 0;
	return ch;
}

char * findVarInURL (char * varcontent, char * URL, char * varname)
{  
  char * tpchar;
  char * end;
  char fullvarname[500];
  if (URL == NULL || varname == NULL) return NULL;
  
  strcpy(fullvarname,varname);
  strcat(fullvarname,"=");
 
  tpchar=strstr( URL, fullvarname );  
  if (tpchar == NULL) return NULL;
  tpchar=tpchar+strlen(varname)+1;
  strcpy(varcontent,tpchar);
  end = strchr( varcontent, '&');
	if (end != NULL) end[0]=0;
	return varcontent;
}

char * JSONArrayAt(char* jsonelement,char* jsonarray, int i)
{
  jsonelement[0]=0;
  char tempjsonelement[500];
  if (jsonarray==NULL) return NULL;
  int length=0;
  if (jsonarray[0] != '[') return NULL;
  if (jsonarray[1] == ']') return NULL;
  if (jsonarray[strlen(jsonarray)-1] != ']') return NULL;
  
  if (i == 0)
  {
    strcpy(tempjsonelement,jsonarray);
    jsonarray=tempjsonelement;
  } 
  do
  {
    jsonarray++;
    if ( jsonarray[0] ==',' )  {
      length++;
      if (length==i) {
        strcpy(tempjsonelement,jsonarray);
        jsonarray=tempjsonelement;
      }
//      printf("coma detected, we are at: %s<BR>",jsonarray);
    }
    else if ( jsonarray[0] =='{' ) {
      jsonarray=strchr(jsonarray+1,'}');
//      printf("openbracket detected, jumping to: %s<BR>",jsonarray);
    }
    else if ( jsonarray[0] =='"' ) {
      jsonarray=strchr(jsonarray+1,'"');
//      printf("quote detected, jumping to: %s<BR>",jsonarray);
    }   
  }
  while (length<(i+1) && jsonarray!=NULL);
  if (jsonarray!=NULL) jsonarray[0]=0;  
  strcpy(jsonelement,tempjsonelement+1);
  if (jsonarray==NULL) jsonelement[strlen(jsonelement)-1]=0;
  return jsonelement;
  
}



int JSONArrayLength(char* jsonarray)
{
if (jsonarray==NULL) return -1;
int length=1;
if (jsonarray[0] != '[') return -1;
if (jsonarray[1] == ']') return 0;


do
{
  jsonarray++;
  if ( jsonarray[0] ==',' )  {
    length++;
  }
  else if ( jsonarray[0] =='{' ) {
    jsonarray=strchr(jsonarray+1,'}');
  }
  else if ( jsonarray[0] =='"' ) {
    jsonarray=strchr(jsonarray+1,'"');
  }
}
while (jsonarray[0]!=0);

return length;

}

char * JSONtoString (char* jsonvalue)
{
if (jsonvalue==NULL) return NULL;
char tc;
tc=jsonvalue[0];
if (tc != '"') return NULL;
int i;
 for (i=1;i<strlen(jsonvalue);i++)
 {
    jsonvalue[i-1]=jsonvalue[i];
 }
jsonvalue[i-2]=0;
return  jsonvalue;
}

char * JSONfindObject (char* value, char * query, char * objectname)
{
  char * tpchar;
	char * end;
	int closechar;
	char fullobjectname[80];
	
	if (value==NULL || query==NULL || objectname==NULL) return NULL;
   
	strcpy (fullobjectname,"\"");
	strcat (fullobjectname,objectname);
	strcat (fullobjectname,"\"");
	tpchar=query;
	 do	{
		tpchar++;
		tpchar=strstr( tpchar, fullobjectname );
	 }
	 while (  tpchar!=NULL && tpchar[-1] != '{' && tpchar[-1] != ','   ); 
	if (tpchar==NULL)	return NULL;
	tpchar=strpbrk (tpchar+strlen(fullobjectname),"{[\"");
	if (tpchar==NULL)	return NULL;
	strcpy(value,tpchar);
	if (tpchar[0]=='"') closechar='"';
	if (tpchar[0]=='[') closechar=']';
	if (tpchar[0]=='{') closechar='}';
	end = strchr( value+1, closechar);
	if (end == NULL) return NULL;
	end[1]=0;    
	return value;
} 



void webgatewayMessageHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue) {
 // printf( "Received a pmax Message from %s-%s.%s of type %d for %s.%s\n", 
	//  xPL_getSourceVendor(theMessage), xPL_getSourceDeviceID(theMessage), xPL_getSourceInstanceID(theMessage),
	//  xPL_getMessageType(theMessage), xPL_getSchemaClass(theMessage), xPL_getSchemaType(theMessage));
	//  printf("%s",xPL_formatMessage(theMessage));
  
}



int main(void)
{
	char *data;
	long m,n;
	printf("%s%c%c\n",
	"Content-Type:text/html;charset=iso-8859-1",13,10);

//	printf("<h3>Multiplication results</h3>\n");

	data = getenv("QUERY_STRING");

  

	stripSpace(data);
	char schemaclass[500];
	char schematype[500];
	char namedvaluearray[500];
	char name[500];
	char value[500];
	char command[500];
	char varcontent[500];
  char jsoncontent[500];
  char NVelement[500];
//  printf("URL %s<BR>\n",data);
  strcpy(command,data);
  unencode(command);
  findVarInURL(varcontent,data,"xplpacket");
  unencode(varcontent); 
  stripSpace(varcontent);
	
  xPL_initialize(xPL_getParsedConnectionType());
  webgateway = xPL_createService("viknet", "webgateway", "default");  
  xPL_setServiceVersion(webgateway, "1.0");

  
  /* Add a responder for time setting */
  xPL_addServiceListener(webgateway, webgatewayMessageHandler, xPL_MESSAGE_ANY, "security", NULL, NULL);
  xPL_setServiceEnabled(webgateway, TRUE);
  /* Create a message to send */
  xplMessage = xPL_createBroadcastMessage(webgateway, xPL_MESSAGE_COMMAND);

  if (JSONfindObject(schemaclass,varcontent,"msgschemaclass")!=NULL && JSONfindObject(schematype,varcontent,"msgschematype")!=NULL)
    xPL_setSchema(xplMessage, JSONtoString(schemaclass), JSONtoString(schematype));
	else
    xPL_setSchema(xplMessage, "schemaclass", "schematype");	
	
	JSONfindObject(namedvaluearray,varcontent,"namevaluelist");
  int i;
   for (i=0;i<JSONArrayLength(namedvaluearray);i++)
   {
     JSONArrayAt(NVelement,namedvaluearray,i);
	   if ( JSONfindObject(name,NVelement,"name") && JSONfindObject(value,NVelement,"value"))
	   xPL_addMessageNamedValue(xplMessage, JSONtoString(name), JSONtoString(value));	   
   }
   xPL_sendMessage(xplMessage);
//   xPL_processMessages(0);
	return 0;  
}       
