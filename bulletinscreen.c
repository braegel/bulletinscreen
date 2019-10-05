#include <argp.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
				       {"verbose",  'v', 0,      0,  "Produce verbose output" },
				       {"quiet",    'q', 0,      0,  "Don't produce any output" },
				       { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[2];                /* arg1 & arg2 */
  int silent, verbose, range;
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
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

// For socket
void error(char *msg)
{
  perror(msg);
  exit(0);
}


int
main (int argc, char **argv)
{
  struct arguments arguments;

  /* Default values. */
  arguments.silent = 0;
  arguments.verbose = 0;
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
  if (arguments.verbose)
    printf("%s\n",buffer);

 
  // user -1 pass -1 vers bulletinscreen 0.0.1 filter t/m
  bzero(buffer,256);
  snprintf(buffer, sizeof(buffer), "user -1 pass -1 vers %s filter t/m\n", argp_program_version);
  n = write(sockfd,buffer,strlen(buffer));
  if (n < 0) 
    error("ERROR writing to socket");

  while (1){
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
      error("ERROR reading from socket");
    printf("%s\n",buffer);
  }
  return 0;
}

