#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h> /* POSIX terminal control definitions */
#include <sys/time.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <stdbool.h>
#include <libconfig.h> 


time_t lastValidSerialIO;

#include "debug.h" 
#include "pmax_constant.h"
#include "pmaxd-xpl.c"

 

int fd; /* File descriptor for the port */
int foregroundOption = 0;
int verboseLevel = 0;
config_t cfg, *cf;





//struct timeval  tvLastSerialCharTime,tvCurrentTime;
//double          lastSerialCharTime,currentTime;
 
 	
  
unsigned char *bufptr;  /* Current char in buffer */


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

void initSerialPort() {
  struct termios options;
	const config_setting_t *device;
	int count;
	device = config_lookup(cf, "device");
  count = config_setting_length(device);
    
  DEBUG(LOG_INFO,"there are %d device in your config file",count);
  int n;
  for (n = 0; n < count; n++) {
    fd = open(config_setting_get_string_elem(device, n), O_RDWR | O_NOCTTY);
    if (fd != -1) break;
  }
	
	if (fd == -1) {
    DEBUG(LOG_ERR,"open_port: Unable to open serial ports");
    printf("exiting: no serial port available");
    exit(EXIT_FAILURE);/*
    * Could not open the port.
	 */}
	DEBUG(LOG_INFO,"opening %s",config_setting_get_string_elem(device, n));
	
  fcntl(fd, F_SETFL, 0);
			
  /*
  * Get the current options for the port...
	*/ 
   tcgetattr(fd, &options);
	/*
 	* Set the baud rates to 9600...
  */
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	
	/*
  * Enable the receiver and set local mode...
	*/ 
  options.c_cflag |= (CLOCAL | CREAD | CS8 );
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
//	options.c_cflag &= ~CRTSCTS ;
    	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  options.c_iflag &= ~(ICRNL | IXON | IXOFF | IXANY);
    
  options.c_oflag &= ~OPOST;
    
  /*
	* Set the new options for the port...
	*/
	tcsetattr(fd, TCSAFLUSH, &options);
  fcntl(fd, F_SETFL, FNDELAY);
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
    
void logBuffer(int priority,struct PlinkBuffer * Buff)  {
  unsigned short i;
  char printBuffer[MAX_BUFFER_SIZE*3+3];
  char *bufptr;      /* Current char in buffer */
	bufptr=printBuffer;
  for (i=0;i<(Buff->size);i++) {
    sprintf(bufptr,"%02X ",Buff->buffer[i]);
    bufptr=bufptr+3;
  }  
  DEBUG(LOG_DEBUG,"BufferSize: %d" ,Buff->size);
  DEBUG(priority,"Buffer: %s", printBuffer);  
}
  
unsigned char calculChecksum(struct PlinkBuffer * Buff) {
	unsigned short checksum,i;
  checksum=0xFFFF;
  for (i=0;i<(Buff->size);i++)
    checksum=checksum-Buff->buffer[i];
  checksum=checksum%0xFF;
  DEBUG(LOG_DEBUG,"checksum: %04X",checksum);
  return (unsigned char) checksum;
} 
     
void sendBuffer(struct PlinkBuffer * Buff)  {
  int i,err;
  struct PlinkBuffer writeBuffer;
  DEBUG(LOG_DEBUG,"Sending the following buffer to serial TTY");  
  logBuffer(LOG_DEBUG,Buff);
  writeBuffer.buffer[0]=0x0D;
  for (i=0;i<(Buff->size);i++)
    writeBuffer.buffer[i+1]=Buff->buffer[i];
  writeBuffer.buffer[Buff->size+1]=calculChecksum(Buff);
  writeBuffer.buffer[2+Buff->size]=0x0A;
  writeBuffer.size=Buff->size+3;
  err=write(fd,writeBuffer.buffer,Buff->size+3);
  DEBUG(LOG_DEBUG,"result of serial write:: %i",err); 	
}
  


bool deFormatBuffer(struct PlinkBuffer  * Buff) {
  int i;
  unsigned char checkedChecksum,checksum;
  checksum=Buff->buffer[Buff->size-2];

  for (i=0;i<Buff->size;i++)
    Buff->buffer[i]=Buff->buffer[i+1]; 
              
  Buff->size=Buff->size-3;
  checkedChecksum=calculChecksum(Buff);
  if (checksum==checkedChecksum)  {
    DEBUG(LOG_DEBUG,"checksum OK");
    lastValidSerialIO = time (NULL);
  } 
  else  {
    DEBUG(LOG_ERR,"checksum NOK calculated:%04X in packet:%04X",checkedChecksum,checksum);
  }  
  return checksum==checkedChecksum;
}  

 

bool compareBuffer(struct PlinkBuffer  * Buff1,struct PlinkBuffer  * Buff2) {
  int i=0;
  if (Buff1->size!=Buff2->size) return false;
  
  for (i=0;i<Buff1->size;i++) {
    if (Buff1->buffer[i] != Buff2->buffer[i]) return false;
  }
  DEBUG(LOG_DEBUG,"Both buffers are equal");
  return true;   
}

bool findCommand(struct PlinkBuffer  * Buff,struct PlinkBuffer  * BuffCommand)  {
  int i=0;
  if (Buff->size!=BuffCommand->size)  return false;
  for (i=0;i<Buff->size;i++)  {
    if ((Buff->buffer[i] != BuffCommand->buffer[i]) && (BuffCommand->buffer[i] != 0xFF )) return false;
  }
  DEBUG(LOG_DEBUG,"Command find !!!!");
  return true;   
}

  
// check if a key has been hit, usefull to break a loop  
int kbhit(void) {
  struct termios oldt, newt;
  int ch;
  int oldf;
     
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch !=EOF)  {
    ungetc(ch, stdin);
    return 1;
  }     
  return 0;
}

void KeyPressHandling(void) {
  char c; 
  if ( kbhit() )  {
    c = getchar();
 
    if ( c == 'c' ) {
      printf("YOU HIT 'c' ...NOW QUITTING");
      exit(1);
    }
    if ( c == 'h' ) {
      DEBUG(LOG_NOTICE,"Arming home");
      sendBuffer(&PowerlinkCommand[Pmax_ARMHOME]);
    }
    if ( c == 'd' ) {
      DEBUG(LOG_NOTICE,"Disarm");
      sendBuffer(&PowerlinkCommand[Pmax_DISARM]);
    }  
    if ( c == 'a' ) {
      DEBUG(LOG_NOTICE,"Arming away");
      sendBuffer(&PowerlinkCommand[Pmax_ARMAWAY]);
    }
    if ( c == 'g' ) {
      DEBUG(LOG_NOTICE,"Get Event log");
      sendBuffer(&PowerlinkCommand[Pmax_GETEVENTLOG]);
    }
    if ( c == 't' ) {
      DEBUG(LOG_NOTICE,"try re-enroll");
      sendBuffer(&PowerlinkCommand[Pmax_REENROLL]);
    }
    if ( c == 'v' ) {
      DEBUG(LOG_NOTICE,"getting versions string");
      sendBuffer(&PowerlinkCommand[Pmax_GETVERSION]);
    }
    if ( c == 'r' ) {
      DEBUG(LOG_NOTICE,"Request Status Update");
      sendBuffer(&PowerlinkCommand[Pmax_REQSTATUS]);
    }
  }
}        


    ////////////////////////////////////////////

void serialHandler() {
  int nomorechar=0;
  int eop=0;
  struct PlinkBuffer commandBuffer;
  commandBuffer.size=0;   
  do {
    usleep(PACKET_TIMEOUT);
    nomorechar=0;
    while (  (read(fd, commandBuffer.size+commandBuffer.buffer, 1) == 1)  ) { 
//	if  ((read(fd, commandBuffer->size+commandBuffer->buffer, 1) == 1)) { 
		  if (commandBuffer.size<(MAX_BUFFER_SIZE-1))
		    commandBuffer.size++;
      else
        DEBUG(LOG_DEBUG,"Packet too big detected"); 	  
//			gettimeofday(&tvLastSerialCharTime, NULL);
//      lastSerialCharTime = tvLastSerialCharTime.tv_sec*1000000 + (tvLastSerialCharTime.tv_usec);
      eop=1;
      nomorechar=1;
		}     		
	}
	while (nomorechar==1);
   	
		if (eop==1) {
	//  	gettimeofday(&tvCurrentTime, NULL);
  //    currentTime = tvCurrentTime.tv_sec*1000000 + (tvCurrentTime.tv_usec);
      
      // if timeout, assume packet is finished, and manage it (check format/ deformat/......) 
  //    if ((currentTime-lastSerialCharTime)> PACKET_TIMEOUT ) {
        packetManager(&commandBuffer);
  //      eop=0;
  //   }       
    }      
   }
        /////////////////////////////////////////

 
void packetManager(struct PlinkBuffer  * commandBuffer) {
  DEBUG(LOG_DEBUG,"Timeout while waiting packet: assumig packet is complete......");                
  if (deFormatBuffer(commandBuffer)) {
    DEBUG(LOG_DEBUG,"Packet received");
    logBuffer(LOG_DEBUG,commandBuffer);         
    int cmd_not_recognized=1;
    int i;
    for (i=0;i<Pmax_NBCOMMAND;i++)  {
      if (findCommand(commandBuffer,&PmaxCommand[i]))  {
        PmaxCommand[i].action(commandBuffer);
        cmd_not_recognized=0;
        break;
      }   
    }  
    if ( cmd_not_recognized==1 )  {
      DEBUG(LOG_INFO,"Packet not recognized");
      logBuffer(LOG_INFO,commandBuffer);
      sendBuffer(&PowerlinkCommand[Pmax_ACK]);    
    }                  
  }                                                         
  else  {
    DEBUG(LOG_ERR,"Packet not correctly formated");
    logBuffer(LOG_ERR,commandBuffer);
  }              
  //command has been treated, reset the commandbuffer
  commandBuffer->size=0;                    
  //reset End Of Packet to listen for a new packet       
        
  DEBUG(LOG_DEBUG,"End of packet treatment");
}

          
int main(int argc, char **argv) {
  cf = &cfg;
  config_init(cf);
  
  if (!config_read_file(cf, "/etc/pmaxd.conf")) {
            fprintf(stderr, "%s:%d - %s\n",
                config_error_file(cf),
                config_error_line(cf),
                config_error_text(cf));
            config_destroy(cf);
            return(EXIT_FAILURE);
        }
        
        config_lookup_int(cf, "packet_timeout", &PACKET_TIMEOUT);

        

         
  /* Our process ID and Session ID */
  pid_t pid=0, sid=0;        
  
  int helpOption = 0;
   
  int j=0;
  int k=0;
  struct PlinkBuffer testbuffer;
 
  parseCommandLineArgs(argc, argv); 
        
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
       
  DEBUG (LOG_NOTICE, "Program started by User %d", getuid ());
        
  DEBUG (LOG_INFO, "setting SID"); 
                                     
  /* Create a new SID for the child process */
  if  (foregroundOption==0) sid = setsid();
  if (sid < 0) {
    /* Log the failure */
    DEBUG (LOG_INFO, "cannot set SID");
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
   
  /* Daemon-specific initialization goes here */
          
        

// 	struct PlinkBuffer commandBuffer;
// 	commandBuffer.size=0;
 	
 	int loop=0;
  DEBUG(LOG_NOTICE,"Starting......");

  
  
 	initXpl();
  initSerialPort();  
	PmaxInit(); 
  DEBUG(LOG_DEBUG,"Sarting main loop....");


/* read characters into our string buffer until timeout */  
  while (!loop) {
   
    if (foregroundOption==1)  //manage keypress if interactive mode
      KeyPressHandling();
    
       
    serialHandler();
    
    
       
    usleep(1000);
    xPL_processMessages(0);
  }  
  closelog ();
  exit(EXIT_SUCCESS);
}