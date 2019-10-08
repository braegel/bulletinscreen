#include <argp.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO
// Range/Callsign filter

struct arguments arguments;

const char *config_file_name =
  "~/.bulletinscreen";
const char *argp_program_version =
  "bulletinscreen 0.0.1";
const char *argp_program_bug_address =
  "<bernd@braegelmann.net>";

/* Program documentation. */
static char doc[] =
  "bulletinscreen -- a program to create a bulletin screen from APRS messages";

/* A description of the arguments we accept. */
//static char args_doc[] = "--callsign --passcode";
static char args_doc[] = "";

/* The options we understand. */
static struct argp_option options[] = {
				       {"callsign", 'c', "CALLSIGN",      0,  "Your callsign (default: -1)" },
				       {"passcode", 'p', "PASSCODE",      0,  "APRS-IS passcode (default: -1)" },
				       {"server", 's', "PASSCODE",      0,  "APRS-IS server (default: rotate.aprs2.net)" },
				       {"port", 'P', "port",      0,  "APRS-IS server port (default: 14580)" },

				       {"range",    'r', 0,      0,  "Filter around your callsign in km" },
				       {"debug",  'd', 0,      0,  "Produce debugging output" },
				       {"verbose",  'v', 0,      0,  "Produce verbose output" },
				       {"quiet",    'q', 0,      0,  "Don't produce any output" },
				       { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[2];                /* arg1 & arg2 */
  int silent, verbose, range, debug;
  char *callsign;
  char *passcode;
  char *server;
  int port;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  int port;
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q':
      arguments->silent = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'c':
      arguments->callsign = arg;
      break;
    case 'p':
      arguments->passcode = arg;
    case 's':
      arguments->server = arg;
    case 'P':
      sscanf(arg, "%d", &port);
      arguments->port = port;
      break;
    case 'd':
      arguments->verbose = 1;
      arguments->debug = 1;
      break;

    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

void trimTrailing(char * str)
{
  int index, i;

  /* Set default index */
  index = -1;

  /* Find last index of non-white space character */
  i = 0;
  while(str[i] != '\0')
    {
      if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
	  index= i;
        }

      i++;
    }
  /* Mark next character to last non-white space character as NULL */
  str[index + 1] = '\0';
}

// For socket
void error(char *msg)
{
  perror(msg);
  exit(0);
}

bool parseable_message(unsigned char *buffer){
  // NEXT
  regex_t regex;
  int reti;
  reti = regcomp(&regex, "^[A-Z0-9]", 0);
  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    exit(1);
  }
  reti =regexec(&regex, buffer, 0,NULL,0);
  if (!reti)
    {
      return(true);
    }
  if (arguments.debug) printf("Not parsable\n");
  return(false);
}

void process(char *buffer){
  if (buffer[0] == '#') { return; }
  
  int i;
  unsigned char sender[30]; // TODO 7+1 byte should suffice, however i see longer sender callsigns.
  unsigned char addressee[30];
  unsigned char msg[100]; // TODO
  
  if (arguments.debug) printf("%s",buffer);

  if (parseable_message(buffer)) {
    //parse sender
    for(i=0;i<strlen(buffer);i++){
      if (buffer[i]=='>'){
	strncpy(sender,buffer,i);
	sender[i]='\0';
	break;
      }
    }

    //parse message + addressee
    char *r_position_ptr = strrchr(buffer, ':');
    int r_position = (r_position_ptr == NULL ? -1 : r_position_ptr - buffer);
    memcpy(msg, &buffer[r_position+1], strlen(buffer)-(r_position+1) );
    msg[strlen(buffer)-(r_position+1)] = '\0';

    for(i=0;i<strlen(msg);i++){
      //    printf("%i: %02x\n",i,msg[i]);
      if (msg[i]==(char)0x0d || msg[i]=='{'){
	msg[i]='\0';
	break;
      }
    }
  
    memcpy(addressee,&buffer[r_position-9], 9);
    addressee[9]='\0';
    trimTrailing(addressee);
    if (arguments.verbose) printf("%s -> %s: '%s'\n",sender,addressee,msg);
  }  
}

int
main (int argc, char **argv)
{

  /* Default values. */
  arguments.silent = 0;
  arguments.verbose = 0;
  arguments.debug = 0;
  arguments.range=-1;
  arguments.passcode="-1";
  arguments.callsign="-1";
  arguments.server="rotate.aprs2.net";
  arguments.port=14580;

  //for socket
  int sockfd, portno, n;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */

  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  while(1) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
      error("ERROR opening socket");
    server = gethostbyname(arguments.server);
    if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serv_addr.sin_addr.s_addr,
	  server->h_length);
    serv_addr.sin_port = htons(arguments.port);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
      error("ERROR connecting");

    // read ack string from host
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
      error("ERROR reading from socket");
    if (arguments.debug)
      printf("%s\n",buffer);

 
    // user -1 pass -1 vers bulletinscreen 0.0.1 filter t/m
    bzero(buffer,256);
    snprintf(buffer, sizeof(buffer), "user -1 pass -1 vers %s filter t/m\n", argp_program_version);
    if (arguments.debug) {
      printf("Sending to server: '%s'\n",buffer);
    }
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
      error("ERROR writing to socket");

    n=1;
    while (n>=0){
      bzero(buffer,256);
      n = read(sockfd,buffer,255);
      process(buffer);
      //printf(">%s<\n",buffer);
    }
    close(sockfd);
    error("ERROR reading from socket");
  }
  return 0;
}

