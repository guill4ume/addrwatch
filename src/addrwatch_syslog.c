#include <syslog.h>
#include "shm.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <argp.h>

#define MAC_STR_LEN 18


const char *argp_program_version = "Dunno da version, still workin' on it !";
const char *argp_program_bug_address = "<julius.kriukas[at]gmail.com> or <guillaume.marques[at]gmx.com>";

//----- Program documentation -----//
static char doc[] = "Argp implementation, second try !\nThis documentation is useless, you should not use it. ";

//----- Arguments accepted -----//
static char args_doc[] = " ARG1 ARG2 ";
static struct arguments arguments;


//----- Choosable options -----//
static struct argp_option options[] = {
	{"foreground", 'f', 0, 0, "Do not allow the program to demonize itself" },
	{"facility", 'l', "Facility", 0,"Allow the user to change the facility"},
	{ 0 }
};

static int int_facility[] = {
	LOG_LOCAL0,
	LOG_LOCAL1,
	LOG_LOCAL2,
	LOG_LOCAL3,
	LOG_LOCAL4,
	LOG_LOCAL5,
	LOG_LOCAL6,
	LOG_LOCAL7,
};

struct arguments {
	int foreground;
	int facility;
};

//----- Parse option(s) -----//
/* get the input argument from argp_parse,
which we know is a pointer to our arguments structure */
static error_t
parse_opt (int key, char *arg, struct argp_state *state) {
	int f;
	struct arguments *arguments = state->input;
switch (key) {
	case 'f' : 
		arguments->foreground = 1;
		break;
	case 'l' :
		if (strcmp(arg, "user") == 0)
			arguments->facility = LOG_USER;
		else if(strlen(arg) == 6 && strncmp(arg, "local", 5) == 0) {
			f = arg[5] - '0';
			if (f < 0 || f > 7) 
				argp_error(state, "ILLEGAL USE OF FACILITY NAME");
			arguments->facility = int_facility[f]; 
		} else
		  argp_error(state, "ILLEGAL USE OF FACILITY NAME");
		  
	}
	return 0;
}


//----- Argument parser -----//
static struct argp argp = { options, parse_opt, args_doc, doc};

void process_entry(struct shm_log_entry *e)
{
        char mac_str[MAC_STR_LEN];
        char ip_str[INET6_ADDRSTRLEN];
        ether_ntoa_m(e->mac_address, mac_str);

        if (e->ip_len == 16)
                ip6_ntoa(e->ip_address, ip_str);
        else
                ip4_ntoa(e->ip_address, ip_str);


	   syslog(LOG_INFO,"%lu %s %u %s %s %s", e->timestamp, e->interface, e->vlan_tag, 
                                mac_str, ip_str, origin_to_string(e->origin));
	  if (arguments.foreground == 1)
		printf("%lu %s %u %s %s %s\n", e->timestamp, e->interface, e->vlan_tag, 
                                mac_str, ip_str, origin_to_string(e->origin));
}



int main(int argc, char *argv[]) 
{



	arguments.facility = LOG_USER;
	arguments.foreground = 0;
	
	argp_parse(&argp, argc, argv, 0, 0, &arguments);




if (!arguments.foreground) {
	// Daemonizing the process
		pid_t pid, sid;
		pid = fork();
		if(pid < 0)
			syslog(LOG_ERR,"Error forking the process !");
		if(pid > 0)
			exit(EXIT_SUCCESS);
		umask(0);
		sid = setsid();
		if(sid < 0)
			syslog(LOG_ERR,"Error starting new session");
		

}

        openlog("output_syslogd", 0, arguments.facility);		// Opening connection to syslog
 	main_loop(process_entry);
	closelog ();


}
