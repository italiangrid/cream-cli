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

#ifndef __ES_HMESSAGES__
#define __ES_HMESSAGES__

#include <string>
#include <stdio.h>

using namespace std;

namespace es_help_messages {
  
  static string VersionID = "ES User Interface version 1.0";
  static string reportTo  = "Please report any bug to alvise.dorigo@pd.infn.it";

  static string endpoint = string("\n - <endpoint> is in the format <hostname>[:TCPPORT]; TCPPORT is the optional\n")
    + "   TCP port number where the ES service is listening on; default is 8443.\n";

  static string common_opts = string("\nOptions:\n")
    + " --help|-h\n"
    + "     shows this help and exits.\n\n"
    + " --debug|-d\n"
    + "     Enables more verbose output.\n\n"
    + " --proxyfile|-p <alternate_proxy_file>\n"
    + "     Sets to <alternate_proxy_file> the file containing the user\n"
    + "     proxy certificate;\n"
    + "     default is /tmp/x509up_u<UID>, where UID is the unix user identifier\n"
    + "     (output of 'id -u' command).\n\n"
    + " --certfile|-c <user_certificate_file>\n"
    + "     This option must be used with --keyfile; --certfile and --proxyfile\n"
    + "     are mutually exclusive.\n"
    + "     --certfile and --keyfile set respectively the user's certificate and\n"
    + "     key files that need to be used instead of a proxy (tipically because\n"
    + "     an ES service does not support proxy certificates).\n\n"
    + " --keyfile|-k <user_key_file>\n"
    + "     Sets the user's key file (for explanation see --certfile).\n\n"
    + " --timeout|-t <N>\n"
    + "     Sets the connection timeou to Nt; default is 30 seconds.\n\n"
    + " --donot-verify-ac-sign|-A\n"
    + "     Disables verification of Certification Authority's\n"
    + "     signature on user's certificate.\n\n"
    + " --conf|-C <conf_file>\n"
    + "     Reads the mapping of the operation to the service's EPR from the\n"
    + "     <conf_file>. Specification of the service's EPR by mean of the\n"
    + "     --epr option has higher priority over the EPR obtained by the\n"
    + "     configuration file. The configuration file must not be emtpy\n"
    + "     and each its line must have the format:\n"
    + "     <hostname>:<tcpport>,<operation>,<Service_EPR_for_operation>\\n\n"
    + "     where <operation> can be:\n"
    + "     create\n"
    + "     status\n"
    + "     list\n"
    + "     info\n"
    + "     pause\n"
    + "     resume\n"
    + "     restart\n"
    + "     cancel\n"
    + "     wipe\n"
    + "     notify\n"
    + "     delegate\n"
    + "     delegation-info\n"
    + "     delegation-renew\n\n"
    + "     If for and endpoint and current operation there is not a mapped\n"
    + "     EPR (no corresponding line in the conf file) an hard-coded\n"
    + "     default will be used\n\n"
    + " --epr|-E <EPR>\n"
    + "     Selects an alternative for EPR; default depends on the type of operation\n     (see also --conf).\n\n";
  
  static string delegate_usage = string("glite-es-delegate-proxy <endpoint> [OPTIONS]\n")
    + endpoint + "\n";
  
  static string deleginfo_usage = string("glite-es-delegation-info --endpoint|-e <endpoint> [OPTIONS] <Deleg_Identifier>\n")
    + endpoint
    + " - <Deleg_Identier> is a string representing the delegation to get\n   information of.\n";
    
  static string delegrenew_usage = string("glite-es-delegation-renew --endpoint|-e <endpoint> [OPTIONS] <Deleg_Identifier>\n")
    + endpoint
    + " - <Deleg_Identier> is a string representing the delegation to renew.\n";
    
  static string create_usage = string("glite-es-activity-create --endpoint|-e <endpoint> [OPTIONS] <ADL_File>\n")
    + endpoint
    + " - <ADL_File> is an XML file containing the Activity Description.\n";
    
  static string cancel_usage = string("glite-es-activity-cancel --endpoint|-e <endpoint> [OPTIONS] <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint + "\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to cancel on the remote service.\n";

  static string list_usage = string("glite-es-activity-list [OPTIONS] <endpoint>\n")
    + endpoint + "\n";

  static string info_usage = string("glite-es-activity-info [OPTIONS] --endpoint|-e <endpoint> <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint + "\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to query the info of on the remote service.\n";
    
  static string notify_usage = string("glite-es-notify-service [OPTIONS] --endpoint|-e <endpoint> <notify_message>\n")
    + endpoint + "\n" 
    + " - <notify_message> is the notification to send to the service;\n"
    + "   it must have the format <ActivityID>:<MESSAGE>; at the moment the\n"
    + "   only possible values for <MESSAGE> are\n"
    + "     - CLIENT-DATAPUSH-DONE\n"
    + "     - CLIENT-DATAPULL-DONE\n\n";

  static string list_custom_opts = string("")+
    + " --limit|-l <N>\n"
    + "     Sets to N the maximum number of returned entries.\n\n"
    + " --from-date|-F <date>\n"
    + "     Sets the lower date filter: activities created before this date\n"
    + "     will not be returned by the service. Format of <date> must be:\n"
    + "     YYYY-MM-DD HH:mm:ss.\n\n"
    + " --to-date|-T <date>\n"
    + "     Sets the upper date filter: activities created after this date\n"
    + "     will not be returned by the service. Format of <date> must be:\n"
    + "     YYYY-MM-DD HH:mm:ss.\n\n"
    + " --status-filter|-S <comma_separated_states/attrs>\n"
    + "     Sets the status/status-attrs filter: each status/status-attr\n"
    + "     is a string in this format <STATUS>[:<STATUS_ATTRIBUTE>];\n"
    + "     multiple status/status-attr items can be separated by ',' and\n"
    + "     are OR'ed by the service when it tries to selects activities to\n"
    + "     return; e.g. -STERMINAL,PREPROCESSING:VALIDATING,TERMINAL:APP_FAILURE\n"
    + "     Allowed states are:\n"
    + "      ACCEPTED\n"
    + "      PREPROCESSING\n"
    + "      PROCESSING\n"
    + "      PROCESSING_ACCEPTING\n"
    + "      PROCESSING_QUEUED\n"
    + "      PROCESSING_RUNNING\n"
    + "      POSTPROCESSING\n"
    + "      TERMINAL\n\n"
    + "     Allowed status attributes are:\n"
    + "      VALIDATING\n"               
    + "      SERVER_PAUSED\n"            
    + "      CLIENT_PAUSED\n"            
    + "      CLIENT_STAGEIN_POSSIBLE\n"  
    + "      CLIENT_STAGEOUT_POSSIBLE\n"
    + "      PROVISIONING\n"            
    + "      EPROVISIONING\n"           
    + "      SERVER_STAGEIN\n"          
    + "      SERVER_STAGEOUT\n"         
    + "      BATCH_SUSPEND\n"           
    + "      APP_RUNNING\n"            
    + "      PREPROCESSING_CANCEL\n"    
    + "      PROCESSING_CANCEL\n"       
    + "      POSTPROCESSING_CANCEL\n"   
    + "      VALIDATION_FAILURE\n"      
    + "      PREPROCESSING_FAILURE\n"   
    + "      PROCESSING_FAILURE\n"      
    + "      POSTPROCESSING_FAILURE\n"  
    + "      APP_FAILURE\n"             
    + "      EXPIRED\n\n";                 


  static string cancel_custom_opts = string("") + 
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to cancel; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string wipe_usage = string("glite-es-activity-wipe --endpoint|-e <endpoint> [OPTIONS] <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint +"\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to wipe on the remote service.\n";


  static string pause_usage = string("glite-es-activity-pause --endpoint|-e <endpoint> [OPTIONS] <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint +"\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to pause on the remote service.\n";

  static string resume_usage = string("glite-es-activity-resume --endpoint|-e <endpoint> [OPTIONS] <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint +"\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to resume on the remote service.\n";

  static string restart_usage = string("glite-es-activity-restart --endpoint|-e <endpoint> [OPTIONS] <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint +"\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to restart on the remote service.\n";

  static string status_usage = string("glite-es-activity-status --endpoint|-e <endpoint> [OPTIONS] <Activity_ID_1> <Activity_ID_2> ... <Activity_ID_N> \n")
    + endpoint +"\n"
    + " - <Activity_ID_1> ... <Activity_ID_N> are the identifiers (N must be >= 1)\n"
    + "   to query the status of on the remote service.\n";

  static string getresourceinfo_usage = "";
  static string queryresourceinfo_usage = "";
   
  static string wipe_custom_opts = string("") + 
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to wipe; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create or glite-es-activity-list and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string info_custom_opts = string("") +
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to get info of; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create or glite-es-activity-list and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string pause_custom_opts = string("") + 
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to pause; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create or glite-es-activity-list and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string resume_custom_opts = string("") + 
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to resume; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create or glite-es-activity-list and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string restart_custom_opts = string("") + 
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to restart; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create or glite-es-activity-list and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string status_custom_opts = string("") + 
    + " --input|-i\n"
    + "     Specifies an input file containing the list of activity identifiers\n"
    + "     to get status of; the identifiers loaded from the input file will be added\n"
    + "     to the list of the identifiers specified as command's arguments.\n"
    + "     The input file must have a particular format for the header text line\n"
    + "     and should have been created by the --output option of\n"
    + "     glite-es-activity-create or glite-es-activity-list and not by the user.\n"
    + "     Cannot use this option with --endpoint; in fact using --input the\n"
    + "     endpoint will be loaded from the input file itself.\n\n";

  static string create_custom_opts = string(" --output|-o\n")
    + "       Sets the output filename where Activity identifiers will be written into.\n"
    + "       An output file contains activity identifiers created on the\n       same endpoint.\n"
    + "       You cannot write identifiers created on endpoint B into a file previously\n"
    + "       filled with identifiers created on endpoint A.\n\n";

  static string getresourceinfo_custom_opts = "";
  static string queryresourceinfo_custom_opts = "";
 
  static string delegate_custom_opts = "";
  static string deleginfo_custom_opts = "";
  static string delegrenew_custom_opts = "";
}
#endif
