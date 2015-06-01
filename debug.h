#define DEBUG(x,...) log_dfl(x,__FUNCTION__,__LINE__,__VA_ARGS__);		

void (*LOG)(int, const char *, va_list arg);
int (*LOG_setmask)(int);

/* In a header somewhere */
void log_console(int priority, const char *format, va_list arg);
int log_console_setlogmask(int mask);

/* Private storage for the current mask */
static int log_consolemask;


log_dfl(int priority,const char *function, int line,const char *format, ...)
{
  va_list arglist;
  va_start(arglist,format);
 
  char expanded_log[1024]="[";
  char expanded_line[6];
  time_t rawtime;
  struct tm * timeinfo;
  char buffertime [80];
 
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  strftime (buffertime,80,"%c",timeinfo);

  strcat(expanded_log,buffertime);
  strcat(expanded_log," ");
  strcat(expanded_log,function);
  strcat(expanded_log,":");
  sprintf(expanded_line,"%04d]",line);
  strcat(expanded_log,expanded_line);
  strcat(expanded_log,format);
 
 
  LOG(priority,expanded_log,arglist);
  va_end(arglist);
}
  
log_fl(int priority,const char *function, int line,const char *format, ...)
{
 va_list arglist;
 char expanded_log[1024]="[";
 char expanded_line[6];
 strcat(expanded_log,function);
 strcat(expanded_log,":");
 sprintf(expanded_line,"%04d]",line);
 strcat(expanded_log,expanded_line);
 strcat(expanded_log,format); 
  
 va_start(arglist, format);
 LOG(priority,expanded_log,arglist);
 va_end(arglist);
}





 



int log_console_setlogmask(int mask)
{
  int oldmask = log_consolemask;
  if(mask == 0)
    return oldmask; /* POSIX definition for 0 mask */
  log_consolemask = mask;
  return oldmask;
}


void log_console(int priority, const char *format, va_list arg)
{
//  va_list arglist;
  const char *loglevel;
//  va_start(arglist, format);

  /* Return on MASKed log priorities */
  if (!(LOG_MASK(priority) & log_consolemask))
    return;

  switch(priority)
  {
  case LOG_EMERG:
    loglevel = "EMERG: ";
    break;
  case LOG_ALERT:
    loglevel = "ALERT: ";
    break;
  case LOG_CRIT:
    loglevel = "CRIT: ";
    break;
  case LOG_ERR:
    loglevel = "ERR: ";
    break;
  case LOG_WARNING:
    loglevel = "WARNING: ";
    break;
  case LOG_NOTICE:
    loglevel = "NOTICE: ";
    break;
  case LOG_INFO:
    loglevel = "INFO: ";
    break;
  case LOG_DEBUG:
    loglevel = "DEBUG: ";
    break;
  default:
    loglevel = "UNKNOWN: ";
    break;
  }

  printf(" %s", loglevel);
  vprintf(format, arg);
  printf("\n");
  fflush(stdout);
  //va_end(arglist);
}


