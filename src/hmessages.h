/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010. 
See http://www.eu-egee.org/partners/ for details on the copyright
holders.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. 
See the License for the specific language governing permissions and 
limitations under the License.

END LICENSE */

#ifndef __HMESSAGES__
#define __HMESSAGES__

#include <string>
#include <stdio.h>

using namespace std;

namespace help_messages {
  
  char* VersionID = "CREAM User Interface version 1.2.0";
  char* reportTo  = "\nPlease report any bug to:\n	alvise.dorigo@pd.infn.it\n";
  char* common    = "The specification of the tcpport (:<port>) is optional. If not specified it\nwill be obtained by the configuration file or the default 8443 will be used\n\n";
  char* commonJobID = "The specification of JobIDs as command line argument, and the --all option are\nexclusives; also --input and --all are exclusives.\n\nFurthermore the specification of --endpoint option definitely sets the endpoint\nto send the request to and overwrites all endpoints specified in the JobIDs.\n\nFinally the specification of --all with --endpoint and no JobIDs argument\nneither --input <file>";
  char* Cancel_end_mex = " cancels all jobs of the current user on the specified\nendpoint.\n\n";
  char* Status_end_mex = " returns status of all jobs of the current user on the specified\nendpoint.\n\n";
  char* Suspend_end_mex = " suspends all jobs of the current user on the specified\nendpoint.\n\n";
  char* Resume_end_mex = " resumes all jobs of the current user on the specified\nendpoint.\n\n";
  char* Purge_end_mex = " purges all jobs of the current user on the specified\nendpoint.\n\n";
  
  char* commonFile = "If the configuration file <file> is not specified the client will try to read the file specified by\nthe environmental variable GLITE_CREAM_CLIENT_CONFIG; otherwise it will try to\nreadthe file $HOME/.glite/glite_cream.conf.\nFinally if previous tries fail, built-in configurations will be used.\n\n";
  
  //______________________________________________________________________________
  const string submit_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-submit allows the user to submit a job for execution on a CREAM based CE\n\n") +
    string("\nUsage: glite-ce-job-submit [options] -r <CEID> <JDLFile>\n\n") +
    string("  --resource, -r CEID		Select the CE to send the JDL to. Format must be\n\n")+
    string("			  <host>[:<port>]/cream-<lrms-system-name>-<queue-name>\n\n")+
    string("  <JDLFile>			Is the file containing the JDL directives for\n")+
    string("                                job submission;\n")+
    string("\nOptions:\n") +
    string("  --help, -h			prints this help and exits;\n\n") +
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --nomsg, -n                   Suppresses any message except fatal\n")+
    string("                                errors and output;\n\n")+
    string("  --version, -v			Prints the version and exits;\n\n")+
    string("  --output, -o <result_file>	writes the returned JobID on file result_file;\n")+
    string("                                see below for more details about its format\n\n")+//if the file already exists AND is , truncates it before\n")+
    //string("                                writing into it;\n\n")+
    string("  --autm-delegation, -a		Generates a new delegated proxy with an internal\n")+
    string("                                delegation ID;\n\n")+
    string("  --delegationid, -D <ID>	uses a previously delegated proxy certificate\n")+
    string("				identified by <ID>;\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                logfile; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --vo, -V <VO_NAME>            specifies the Virtual Organisation the user\n")+
    string("                                belongs to;\n\n")+
    string("  --noint, -N			turns off any user interactivity (assuming\n")+
    string("				'yes' for all questions);\n\n")+
    string("  --ftp-streams, -s <number>	specifies the number of streams the gridftp\n")+
    string("				client should use when sending InputSandbox.\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    string("  --leaseId, -L                Specify a identifier of a precreated lease to associate\n");
    string("                             with the job to submit.\n\n");

    commonFile+
    string("If <result_file> already exists AND its first line is '##CREAMJOBS##' the\noutput will be appended to it; otherwise you will be asked to overwrite it or\nexit.\n\n")+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string cancel_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-cancel cancels one or more jobs previously submitted to CREAM based CEs\n\n") +
    string("\nUsage: glite-ce-job-cancel [options] <JobID_1> <JobID_2> ... <JobID_N>\n\n")+
    string("  <JobID_1> ...			The identifiers of the jobs to cancel,\n")+
    string("                                as returned by glite-ce-job-submit;\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the cancellation\n")+
    string("                                request to. If this option is not\n")+
    string("  				specified, the endpoint will be extracted from\n")+
    string("                                the <JobID> string;\n")+
    string("                                endpoint must have the format <host>:<port>\n\n")+
    string("  --input, -i <joblist>         lets the user to interactively choose the job(s)\n")+
    string("				to cancel from the <joblist> file\n")+
    string("  --noint, -N			turns off any user interactivity; assumes 'y' or\n")+
    string("				'all' as answer to the questions\n\n")+
    string("  --all, -a			cancels all user's jobs (requires --endpoint\n")+
    string("    				to specify the CEId where the jobs to cancel are\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --nomsg, -n                   suppresses any message except fatal\n")+
    string("                                errors and faults raised by the service;\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    commonJobID+Cancel_end_mex+
    common+
    help_messages::reportTo;

  //______________________________________________________________________________
  const string evtqry_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-event-query Queries one or more events for current user and for his jobs submitted to a CREAM based CEs. An Event is a job status change with additional info.\n\n") +
    string("\nUsage: glite-ce-event-query [options] <from_evt_number>-<to_evt_number>\n\n")+
    string("                               \n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the query event\n")+
    string("                                request to. Mandatory\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --nomsg, -n                   suppresses any message except fatal\n")+
    string("                                errors and faults raised by the service;\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;

  //______________________________________________________________________________
  const string assess_help = string(help_messages::VersionID) +
    string("\nUsage: glite-ce-job-assess [options] -r <CEID> -p <assess_param> <JDLFile>\n\n") +
    string("  --resource, -r CEID	  select the CE to send the JDL to get assess for.\n")+
    string("                          CEID format must be in the two forms:\n\n")+
    string("  			<host>[:<port>]/cream-<lrms-system-name>-<queue-name>\n\n")+
    string("  --assess-param, -p 	  specifies the assess parameter\n")+
    string("  <JDLFile>	   	  is the file containing the JDL directives for job")+
    string("                               submission\n\n")+
    string("\nOptions:\n") +
    string("  --help, -h	  	  prints this help and exits\n\n") +
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --version, -v	  	  prints version and exits\n\n")+
    string("  --conf, -c <file>	        Specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;

  //______________________________________________________________________________
  const string suspend_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-suspend suspends one or more jobs running on CREAM based CEs\n\n") +
    string("\nUsage: glite-ce-job-suspend [options] <JobID_1> <JobID_2> ... <JobID_N>\n\n")+
    string("  <JobID_1> ...			The identifiers of the jobs to suspend,\n")+
    string("                                as returned by glite-ce-job-submit;\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the suspend\n")+
    string("                                request to. If this option is not\n")+
    string("  				specified, the endpoint will be extracted from\n")+
    string("                                the <JobID> string;\n")+
    string("                                endpoint must have the format <host>:<port>\n\n")+
    string("  --input, -i <joblist>         lets the user to interactively choose the job(s)\n")+
    string("				to suspend from the <joblist> file\n\n")+
    string("  --noint, -N			turns off any user interactivity; assumes 'y' or\n")+
    string("				'all' as answer to the questions\n\n")+
    string("  --all, -a			suspends all user's jobs (requires --endpoint\n")+
    string("    				to specify the CEId where the jobs to\n")+
    string("                                suspend are)\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --nomsg, -n                   suppresses any message except fatal\n")+
    string("                                errors and faults from service;\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    commonJobID+Suspend_end_mex+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string resume_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-resume resumes one or more jobs, which have been previously suspended\n\n") +
    string("\nUsage: glite-ce-job-resume [options] <JobID_1> <JobID_2> ... <JobID_N>\n\n")+
    string("  <JobID_1> ...			The identifiers of the jobs to resume,\n")+
    string("                                as returned by glite-ce-job-submit;\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the resume\n")+
    string("                                request to. If this option is not\n")+
    string("  				specified, the endpoint will be extracted from\n")+
    string("                                the <JobID> string;\n")+
    string("                                endpoint must have the format <host>:<port>\n\n")+
    string("  --input, -i <joblist>         lets the user to interactively choose the job(s)\n")+
    string("				to resume from the <joblist> file\n\n")+
    string("  --noint, -N			turns off any user interactivity; assumes 'y' or\n")+
    string("				'all' as answer to the questions\n\n")+
    string("  --all, -a			resumes all user's jobs (requires --endpoint\n")+
    string("    				to specify the CEId where the jobs to resume are\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --nomsg, -n                   suppresses any message except fatal\n")+
    string("                                errors and faults from service;\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    commonJobID+Resume_end_mex+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string purge_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-purge cleans one or more jobs from CREAM based CEs\n\n") +
    string("\nUsage: glite-ce-job-purge [options] <JobID_1> <JobID_2> ... <JobID_N>\n\n")+
    string("  <JobID_1> ...			The identifiers of the jobs to purge,\n")+
    string("                                as returned by glite-ce-job-submit;\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the purge\n")+
    string("                                request to. If this option is not\n")+
    string("  				specified, the endpoint will be extracted from\n")+
    string("                                the <JobID> string;\n")+
    string("                                endpoint must have the format <host>:<port>\n\n")+
    string("  --input, -i <joblist>         lets the user to interactively choose the job(s)\n")+
    string("				to purge from the <joblist> file\n\n")+
    string("  --noint, -N			turns off any user interactivity; assumes 'y' or\n")+
    string("				'all' as answer to the questions\n\n")+
    string("  --all, -a			purges all user's jobs (requires --endpoint\n")+
    string("    				to specify the CEId where the jobs to purge are\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --nomsg, -n                   suppresses any message except fatal\n")+
    string("                                errors and faults from service;\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    commonJobID+Purge_end_mex+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string proxyRenew_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-proxy-renew renews the proxy for one or more delegation ID;\n") +
    string("therefore it renews the proxy of the jobs using these delegations.\n\n") +
    
    string("\nUsage: glite-ce-proxy-renew [options] <DelegationID_1> <DelegationID_2> ... <DelegationID_N>\n\n")+
    string("  <DelegationID_1> ...                     The identifiers of the delegation to renew the proxy for\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the renew\n")+
    string("                                request to;\n")+
    string("                                endpoint must have the format <host>:<port>\n")+
    string("                                This option is mandatory\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --nomsg, -n                   suppresses any message except fatal\n")+
    string("                                errors and faults from service;\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  

//______________________________________________________________________________
  const string signal_help = string(help_messages::VersionID) +
    string("\nUsage: glite-ce-job-signal [options] <JobID>\n\n")+
    string("  <JobID>			The identifier of the job to be signal-ed, as returned by\n")+
    string("                                glite-ce-job-submit;\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n") +
    string("  --version, -v			prints version and exits\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --output, -o <result_file>	Writes service's response (excluding faults)\n")+
    string("  				into <result_file>\n")+
    string("  --endpoint, -e <host>[:<port>] Selects the endpoint to send the signal\n")+
    string("                                request to. If this option is not\n")+
    string("  				specified, the endpoint will be extracted from\n")+
    string("                                the <JobID> string;\n")+
    string("                                Endpoint must have the format <host>:<port>\n")+
    string("  --conf, -c <file>	        Specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;

  //______________________________________________________________________________
  const string status_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-status displays information concerning jobs submitted to CREAM based CEs\n\n") +
    string("\nUsage: glite-ce-job-status [options] <JobID_1> <JobID_2> ... <JobID_N>\n\n")+
    string("  <JobID_1> ...			The identifiers of the jobs to ask the status of,\n")+
    string("                                as returned by glite-ce-job-submit;\n")+
    string("\nOptions:\n")+
    string("  --help, -h			prints this help and exits\n\n") +
    string("  --version, -v			prints version and exits\n\n")+
    string("  --debug, -d			fires out debug messages and activate the\n") +
    string("  				writing on logfile;\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the status\n")+
    string("                                request to. If this option is not\n")+
    string("  				specified, the endpoint will be extracted from\n")+
    string("                                the <JobID> strings;\n")+
    string("                                <endpoint> must have the format <host>:<port>\n\n")+
    string("  --nomsg, -n			suppresses any non-error message on STDOUT\n")+
    string("				(but they still will be printed on logfile\n\n")+
    string("  --input, -i <file>            obtains the list of JobIDs to get the status of\n")+
    string("                                from <file>; <file> must have the same format of\n")+
    string("                                output files generated by glite-ce-job-submit;\n\n")+
    string("  --all, -a                     gets the status of all jobs of the current user\n")+
    string("				located at the endpoint specified by the\n")+
    string("				--endpoint option (that is required in this\n")+
    string("				case);\n\n")+
    string("  --level, -L <verb_num>         sets the verbosity output for a Job's infos:\n")+
    string("				0: prints CREAMJobID and job's status\n")+
    string("				1: also prints times, worker node and exit code\n")+
    string("				2: also prints JDL, proxy delegeation ID,\n")+
    string("				   Input/OutputSandboxes\n\n")+
    string("  --status, -s <STATUS>         specifies a filter for job states;\n")+
    string("				(see below for details);\n\n")+
    string("  --from, -f <TIME>		filters out Jobs before <TIME>\n")+
    string("				(see below for details)\n\n")+
    string("  --to, -t <TIME>		filters out Jobs after <TIME>\n")+
    string("				(see below for details)\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    commonJobID+Status_end_mex+
    string("<STATUS> optional argument must have the format\n\n\tSTATUS_1[:STATUS_2:...:STATUS_N]\n\nwhere ':' means ")+
    string("OR; this option will filter out all jobs that are not in one\nof the states OR'ed. STATUS_X can ")+
    string("be one of the following:\n\n\tDONE-OK\n\tDONE-FAILED\n\tREGISTERED\n\tPENDING\n\tIDLE\n\tRUNNING\n\tREALLY-RUNNING\n\tHELD\n\tCANCELLED\n\tABORTED\n\tUNKNOWN\n")+
    string("\n<TIME>  must have the format 'YYYY-MM-DD HH:mm:ss' where:\n")+
    string("\tYYYY is the year as a decimal number including the century\n")+
    string("\tMM is the month as a decimal number (1-12)\n")+
    string("\tDD is the day of month as a decimal number (1-31)\n")+
    string("\tHH, mm, ss are respectively the hour (0-23), the minutes (0-59) and\n\tthe seconds (0-59)\n\n")+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string delegate_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-delegate-proxy allows the user to delegate her proxy credentials, that can be used later for her job submissions\n\n") +
    string("\nUsage glite-ce-delegate-proxy [options] -e|--endpoint <endpoint> <ID>\n\n") +
    string("  <ID>			Specifies the identifier of a delegated proxy;\n\n") +
    string("  <endpoint>  	Is the remote delegation service and its\n")+
    string("			format must be <hostname>[:<tcpport>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	  	  prints this help and exits\n\n") +
    string("  --debug, -d		  fires out debug messages and activate the\n") +
    string("  			  writing on logfile;\n\n")+
    string("  --version, -v	  	  prints version and exits\n\n")+
    string("  --logfile, -l <logfile> writes any log and/or error message on the file\n")+
    string("                          <logfile>; if the file already exists,\n")+
    string("                          the output is appended to it;\n\n")+
    string("			  file pointed by X509_USER_PROXY env var will be\n")+
    string("			  looked for; if it is not present,\n")+
    string("			  /tmp/x509_u<UID> will be tried. If no file is found\n") +
    string("			  delegation will stop by exiting with an error code\n\n") +
    string("  --conf, -c <file>	  specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string CEMonURL_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-get-cemon-url returns the end-point of the CEMon service coupled to the considered CREAM service\n\n") +
    string("\nUsage glite-ce-get-cemon-url [options] <endpoint>\n\n") +
    string("  <endpoint>  	Is the remote CREAM service and its\n")+
    string("			format must be <hostname>[:<tcpport>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	  	  prints this help and exits\n\n") +
    string("  --debug, -d		  fires out debug messages and activate the\n") +
    string("  			  writing on logfile;\n\n")+
    string("  --version, -v	  	  prints version and exits\n\n")+
    string("  --logfile, -l <logfile> writes any log and/or error message on the file\n")+
    string("                          <logfile>; if the file already exists,\n")+
    string("                          the output is appended to it;\n\n")+
    string("			  file pointed by X509_USER_PROXY env var will be\n")+
    string("			  looked for; if it is not present,\n")+
    string("			  /tmp/x509_u<UID> will be tried. If no file is found\n") +
    string("			  program will stop by exiting with an error code\n\n") +
    string("  --conf, -c <file>	  specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  
  
  //______________________________________________________________________________
  const string list_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-list displays the Ids of all the jobs, belonging to the user issuing the command, which are known to a specified CREAM based CE\n\n") +  
    string("\nUsage glite-ce-job-list [options] <endpoint>\n\n") +
    string("  <endpoit>    Specifies the remote endpoint where a CREAM service is\n")+
    string("               running on; format must be <host>[:<port>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	                prints this help and exits\n\n") +
    string("  --debug, -d	                fires out debug messages and activate the\n") +
    string("  		                writing on logfile;\n\n")+
    string("  --version, -v                 prints version and exits\n\n")+
    string("  --nomsg, -n     		suppresses any message except fatal ones\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  --output, -o <result_file>	writes service's response (excluding faults)\n")+
    string("  				see below for more details about its format\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    string("If <result_file> already exists AND its first line is '##CREAMJOBS##' the output\nwill be appended to it; otherwise you will be asked to overwrite it or exit\n\n")+
    common+
    help_messages::reportTo;
  
  
  //______________________________________________________________________________
  const string Enable_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-enable-submission enables job submissions to the specified CREAM CE\n\n") +
    string("\nUsage glite-ce-enable-submission [options] <endpoint>\n\n") +
    string("  <endpoit>    Specifies the remote endpoint where a CREAM service is\n")+
    string("               running on; format must be <host>[:<port>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	                prints this help and exits\n\n") +
    string("  --debug, -d	                fires out debug messages and activate the\n") +
    string("  		                writing on logfile;\n\n")+
    string("  --version, -v                 prints version and exits\n\n")+
    string("  --nomsg, -n     		suppresses any message except fatal ones\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  				see below for more details about its format\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string Disable_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-disable-submission disables job submissions to the specified CREAM CE\n\n") +
    string("\nUsage glite-ce-disable-submission [options] <endpoint>\n\n") +
    string("  <endpoit>    Specifies the remote endpoint where a CREAM service is\n")+
    string("               running on; format must be <host>[:<port>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	                prints this help and exits\n\n") +
    string("  --debug, -d	                fires out debug messages and activate the\n") +
    string("  		                writing on logfile;\n\n")+
    string("  --version, -v                 prints version and exits\n\n")+
    string("  --nomsg, -n     		suppresses any message except fatal ones\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  				see below for more details about its format\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string DoesSub_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-allowed-submission checks if new job submissions to the specified CREAM CE are allowed\n\n") + 
    string("\nUsage glite-ce-allowed-submission [options] <endpoint>\n\n") +
    string("  <endpoit>    Specifies the remote endpoint where a CREAM service is\n")+
    string("               running on; format must be <host>[:<port>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	                prints this help and exits\n\n") +
    string("  --debug, -d	                fires out debug messages and activate the\n") +
    string("  		                writing on logfile;\n\n")+
    string("  --version, -v                 prints version and exits\n\n")+
    string("  --nomsg, -n     		suppresses any message except fatal ones\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  				see below for more details about its format\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string ServiceInfo_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-service-info returns information of a CE CREAM service\n\n") + 
    string("\nUsage glite-ce-service-info [options] <endpoint>\n\n") +
    string("  <endpoit>    Specifies the remote endpoint where a CREAM service is\n")+
    string("               running on; format must be <host>[:<port>]\n\n") +
    string("Options:\n")+
    string("  --help, -h	                prints this help and exits\n\n") +
    string("  --debug, -d	                fires out debug messages and activate the\n") +
    string("  		                writing on logfile;\n\n")+
    string("  --version, -v                 prints version and exits\n\n")+
    string("  --nomsg, -n     		suppresses any message except fatal ones\n\n")+
    string("  --logfile, -l <logfile>	writes any log and/or error message on the file\n")+
    string("                                <logfile>; if the file already exists,\n")+
    string("                                the output is appended to it;\n\n")+
    string("  				see below for more details about its format\n\n")+
    string("  --conf, -c <file>	        specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A    Allows the execution of the command even if the VO voms\n")+
    string("                                server certificate is not installed.\n")+
    string("  --level, -L			specifies the verbosity level of the info returned by CREAM\n\n")+ 
    commonFile+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string jobLease_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-lease sets a LeaseID and its duration time to the specified CREAM CE\n\n") + 
    string("\nUsage glite-ce-job-lease [options] -e|--endpoint <endpoint> --leaseTime <Lease_Time> <Lease_ID>\n\n") +
    string("  <endpoit>    Specifies the remote endpoint where a CREAM service is\n")+
    string("               running on; format must be <host>[:<port>]\n\n") +
    string("Options:\n")+
    string("  --help, -h			    prints this help and exits\n\n") +
    string("  --version, -v		    prints version and exits\n\n")+
    string("  --debug, -d		    fires out debug messages and activate the\n") +
    string("  				    writing on logfile;\n\n")+
    string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the lease\n")+
    string("                                 request to.\n")+
    string("                                 endpoint must have the format <host>:<port>\n\n")+
    string("  --logfile, -l <logfile>	    writes any log and/or error message on the file\n")+
    string("                                 <logfile>; if the file already exists,\n")+
    string("                                 the output is appended to it;\n\n")+
    string("  --nomsg, -n                    suppresses any message except fatal\n")+
    string("                                 errors and faults from service;\n\n")+
    string("  --conf, -c <file>	            specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A   Allows the execution of the command even if the VO voms\n")+
    string("                             server certificate is not installed.\n\n")+
    commonFile+
    common+
    help_messages::reportTo;
  
  //______________________________________________________________________________
  const string jobOutput_help = string(help_messages::VersionID) +
    string("\n\nglite-ce-job-output retrieve the job's OutputSandBox(es) from a CREAM CE\n\n") + 
    string("\nUsage glite-ce-job-output [options] <JobID1> <JobID2>  ... <JobIDN>\n\n") +
    string("  <JobID>...   Specifies the CREAM job identifiers the user\n")+
    string("               wants to retrieve the output of.\n\n") +
    string("Options:\n")+
    string("  --help, -h		  Prints this help and exits\n\n") +
    string("  --version, -v		  Prints version and exits\n\n")+
    string("  --debug, -d		  Fires out debug messages and activate the\n") +
    string("  				  writing on logfile;\n\n")+
    string("  --input, -i <joblist>       Retrieve the outputs of the jobs listed\n")+
    string("                              in the <joblist> file\n\n")+
    string("  --dir, -D <output dir>      Specifies the parent directory where to create\n")+
    string("                              the job directory where the OSB files\n")+
    string("                              for that job will be saved\n\n")+
    string("  --num-streams, -s <#>       If the files are big the user can specifies\n")+
    string("                              the number of parallel streams of the gridftp\n")+
    string("                              transfer (the higher the number the faster the transfer\n\n")+
    string("  --noint, -n		  Assume 'yes' to all questions\n\n")+

//     string("  --endpoint, -e <host>[:<port>] selects the endpoint to send the lease\n")+
//     string("                                 request to.\n")+
//     string("                                 endpoint must have the format <host>:<port>\n\n")+
    string("  --logfile, -l <logfile>	  Writes any log and/or error message on the file\n")+
    string("                              <logfile>; if the file already exists,\n")+
    string("                              the output is appended to it;\n\n")+
    string("  --nomsg, -n                 Suppresses any message except fatal\n")+
    string("                              errors and faults from service;\n\n")+
    string("  --conf, -c <file>	          Specifies the configuration file\n\n")+
    string("  --donot-verify-ac-sign, -A  Allows the execution of the command even if the VO voms\n")+
    string("                              server certificate is not installed.\n\n")+
    commonFile+
    help_messages::reportTo;

  //______________________________________________________________________________
  string getStartMessage(void) {
    time_t now  = time(NULL);
    string smex = help_messages::VersionID+string(" - Starting at ")+string(ctime(&now));
    return smex;
  }
  
  string getVersion(void)      { return help_messages::VersionID;     }
  string getSubmitHelp(void)   { return help_messages::submit_help;   }
  string getCancelHelp(void)   { return help_messages::cancel_help;   }
  string getAssessHelp(void)   { return help_messages::assess_help;   }
  string getStatusHelp(void)   { return help_messages::status_help;   }
  string getResumeHelp(void)   { return help_messages::resume_help;   }
  string getSuspendHelp(void)  { return help_messages::suspend_help;  }
  string getSignalHelp(void)   { return help_messages::signal_help;   }
  string getDelegateHelp(void) { return help_messages::delegate_help; }
  string getListHelp(void)     { return help_messages::list_help;     }
  string getPurgeHelp(void)    { return help_messages::purge_help;    }
  string getCEMonURLHelp(void) { return help_messages::CEMonURL_help; }
  string getEnableSubHelp(void){ return help_messages::Enable_help;   }
  string getDisableSubHelp()   { return help_messages::Disable_help;  }
  string getDoesSubHelp(void)  { return help_messages::DoesSub_help;  }
  string getRenewHelp(void)    { return help_messages::proxyRenew_help;  }
  string getJobLeaseHelp(void) { return help_messages::jobLease_help;  }
  string getQueryEventHelp( void ) { return help_messages::evtqry_help; }
  string getOutputHelp( void ) { return help_messages::jobOutput_help; }
  
  void printver(void) {
    printf("%s\n",help_messages::getVersion().c_str());
  }
} // namespace help_messages
#endif
