#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>

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
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
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

      break;

    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

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


  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  return 0;
}

