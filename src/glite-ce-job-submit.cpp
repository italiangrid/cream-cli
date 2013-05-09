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

/**
 *
 * SYSTEM C/C++ INCLUDE FILES
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>
#include <fstream>
#include <dlfcn.h>
#include <cstdio>

/**
 *
 * LOCAL INCLUDE FILES
 *
 */
#include "ftpclient/gridftpClient.h"
#include "services/cli_service_jobsubmit.h"
#include "services/cli_service_esjobsubmit.h"
#include "util/jdlHelper.h"
#include "util/cliUtils.h"
#include "hmessages.h"

/**
 *
 * CREAM CLIENT API C++ INCLUDE FILES
 *
 */
#include "glite/ce/cream-client-api-c/JobDescriptionWrapper.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/JobPropertyWrapper.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

/**
 *
 * BOOST INCLUDE FILES
 *
 */
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api::cream_exceptions;
using namespace glite::ce::cream_client_api::soap_proxy;

namespace fs = boost::filesystem;
namespace po = boost::program_options;

Namespace namespaces[] = {};

void printhelp();
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

//string make_delegation( const string&, const string &, const int ) throw();

int main(int argc, char *argv[]) {

  string 	 URL;
  vector<string> JDLFiles;

  string 	VO_from_cert = "";
  bool 		debug = false;

  string    	logfile;
  string    	outfile;
  string    	user_VO;
  bool      	nomsg = false;
  string    	DID;
  ofstream  	filestr;
  bool      	noint = false;
  int       	num_streams = 4;
  bool      	redir_out = false;
  string    	user_conf_file = "";
  string 	certfile, proxyfile;
  string 	leaseId = "";
  int		timeout = 30;


  //  creamApiLogger* logger_instance = creamApiLogger::instance();
  //  log4cpp::Category* log_dev = logger_instance->getLogger();



  // Initialize the logger
  //  log_dev->setPriority( log4cpp::Priority::NOTICE );
  //  logger_instance->setLogfileEnabled( true );

  bool verify_ac_sign = true;
  po::options_description desc("Usage");
  try {
  desc.add_options()
    ("help,h", "display this help and exit")
    (
     "debug,d",
     "run in daemon mode and save the pid in this file"
     )
    (
     "version,v", "display version and exit"
     )
    (
     "nomsg,N",
     "Do not write any message on stdout"
     )
    (
     "noint,n",
     "Do not write any message on stdout"
     )
    (
     "logfile,l",
     po::value<string>(&logfile),
     "Set the lof filename"
     )       
    (
     "conf,c",
     po::value<string>(&user_conf_file)->default_value(""),
     "Set the configuration filename"
     )
    (
     "proxyfile,p",
     po::value<string>(&certfile)->default_value(cliUtils::getProxyCertificateFile()),
     "Set the proxy filename"
     )
    (
     "timeout,t",
     po::value<int>(&timeout)->default_value(30),
     "Set the soap connection timeout in seconds"
     )
    (
     "donot-verify-ac-sign,A",
     "Disable AC's signature verification"
     )
    ("resource,r", po::value<string>(&URL), "Set the ceid to send the job to")
    (
     "fake,F", "Filter on certain jobs's states"
     )
    (
     "delegationId,D", po::value<string>(&DID), "Set maximum number or events CREAM must return"
     )
    (
     "autm-delegation,a", "Set maximum number or events CREAM must return"
     )
    (
     "vo,V", po::value<string>(&user_VO), ""
     )
    (
     "leaseId,L", po::value<string>(&leaseId), ""
     )
    (
     "output,o", po::value<string>(&outfile), ""
     )
    (
     "ftp-streams,s", po::value<int>(&num_streams)->default_value(4), "" 
     )
    (
     "jdl-file,J", po::value<vector<string> >(&JDLFiles), "Set the range extremes of events CREAM must return"
     );
  } catch(glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    cerr << "FATAL: " << ex.what() << endl;
    return 1;
  }
  po::positional_options_description p;
  p.add("jdl-file", -1);

        
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
	      options(desc).positional(p).run(), vm);
    po::notify( vm );
  } catch( std::exception& ex ) {
    cerr << "There was an error parsing the command line. "
	 << "Error was: " << ex.what() << endl
	 << "Type " << argv[0] 
	 << " --help for the list of available options"
	 << endl;
    exit( 1 );
  }

  if ( vm.count("help") ) {
    printhelp();
    exit(0);
  }

  if( vm.count("donot-verify-ac-sign") )
    verify_ac_sign = false;

  if( vm.count("nomsg") && vm.count("debug") )
    {
      cerr << "Cannot specify both --debug and --nomsg options. Stop." << endl;
      return 1;
    }

  if( vm.count("debug") )
    debug = true;
  if( vm.count("nomsg") )
    nomsg = true;
  if( vm.count("noint") )
    noint = true;

  if( !vm.count("jdl-file") )
    {
      cerr << "JDL file not specified in the command line arguments. Stop." << endl;
      return 1;
    }

  if( !vm.count("resource") )
    {
      cerr << "CEId not specified in the command line arguments. Stop." << endl;
      return 1;
    }

  if( vm.count("autm-delegation") && vm.count("delegationId") )
    {
      cerr << "Cannot specify both --autm-delegation and --delegationId options, they're mutually exclusive. Stop." << endl;
      return 1;
    }

  if( !vm.count("autm-delegation") && !vm.count("delegationId") )
    {
      cerr << "Must specify one (and only one) of the two options --autm-delegation --delegationId. Stop." << endl;
      return 1;
    }

  glite::ce::cream_cli::services::cli_service_common_options opt;
  opt.m_debug = debug;
  opt.m_nomsg = nomsg;
  opt.m_noint = noint;
  opt.m_verify_ac_sign = verify_ac_sign;
  opt.m_redir_out = redir_out;
  opt.m_user_conf_file = user_conf_file;
  opt.m_certfile = certfile;
  if(vm.count("logfile"))
    opt.m_redir_out = true;
  opt.m_logfile = logfile;
  opt.m_endpoint = "";//endpoint;
  opt.m_soap_timeout = timeout;  
  
  boost::regex pattern_cream;//, pattern_es;
  boost::cmatch what;
  pattern_cream = "^([^:]+)(:[0-9]+)?/cream-[^-]+-.+";
//  pattern_es    = "^([^:]+)(:[0-9]+)?/es";
  
  if( !boost::regex_match(URL.c_str(), what, pattern_cream) )// &&
//      !boost::regex_match(URL.c_str(), what, pattern_es) )
      {
 //       cerr << "Specified CEID has wrong format. Must be <host>[:<tcp_port>]/cream-<batch_system_name>-<queue_name> OR <host>[:<tcp_port>]/es" << endl;
        cerr << "Specified CEID has wrong format. Must be <host>[:<tcp_port>]/cream-<batch_system_name>-<queue_name>" << endl;
        return 1;
      }
  
//  bool isES = boost::regex_match(URL.c_str(), what, pattern_es);

//  if(!isES) {
  
  glite::ce::cream_cli::services::cli_service_jobsubmit JS( opt );

  if( vm.count("fake") )
    JS.set_fake_run( );

  if( vm.count("vo") )
    JS.set_user_specified_vo( user_VO );

  for(vector<string>::const_iterator it = JDLFiles.begin();
      it != JDLFiles.end();
      ++it )
    {
      JS.add_jdl_file( *it );
    }

  JS.set_delegation( vm.count("autm-delegation")!=0, DID );

  JS.set_lease_id( leaseId );

  if( vm.count("output") )
    JS.set_outputfile( outfile );

  JS.set_ceid( URL );

  if( JS.execute( ) )
    {
      JS.getLogger( )->fatal( JS.get_execution_error_message( ) );
      return 1;
    }

  vector<string> jobs( JS.get_jobids( ) );
  for(vector<string>::const_iterator it = jobs.begin();
      it != jobs.end();
      ++it )
    {
      cout << *it << endl;
    }
//  } else {
//    glite::ce::cream_cli::services::cli_service_esjobsubmit ESJS( opt );
//    ESJS.set_xml_file( JDLFiles.at(0) );
//    ESJS.set_ceid( URL );
//    if(ESJS.execute( )) {
//      cerr << ESJS.get_execution_error_message( ) << endl;
//    }
//  }
  
  return 0;

}



//------------------------------------------------------------------------------
void printhelp() {
  printf("%s\n",help_messages::getSubmitHelp().c_str());
}



//------------------------------------------------------------------------------
// string make_delegation( const string& deleg_service, const string& certfile, const int timeout) throw()
// {
//   creamApiLogger* logger_instance = creamApiLogger::instance();
//   string delegID;
//   glite::ce::cream_client_api::certUtil::generateUniqueID( delegID );
//   AbsCreamProxy *creamDelegClient =  CreamProxyFactory::make_CreamProxyDelegate( delegID, timeout );
// 
//   if(!creamDelegClient)
//     {
//       logger_instance->log(log4cpp::Priority::FATAL, "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", true, true, true);
//       exit( 1 );
//     }
// 
//   boost::scoped_ptr< AbsCreamProxy > tmpClient;
//   tmpClient.reset( creamDelegClient );
// 
//   creamDelegClient->setCredential( certfile );
// 
//   try {
//     creamDelegClient->execute( deleg_service );
//   } catch(exception& ex) {
//     logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
//     exit(1);
//   }
//   
//   return delegID;
// }
