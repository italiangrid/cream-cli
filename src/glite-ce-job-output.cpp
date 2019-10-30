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

#include <sys/stat.h>
#include <cerrno>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <cstdio>
#include <cstring>

#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/JobStatusWrapper.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/JobInfoWrapper.h"
#include "glite/ce/cream-client-api-c/JobCommandWrapper.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "util/cliUtils.h"
#include "hmessages.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>


using namespace std;
using namespace glite::ce::cream_client_api;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::cream_exceptions;

namespace fs = boost::filesystem;

Namespace namespaces[] = {};

void printhelp(void);
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

void retrieve_output_from_endpoint( const string& endpoint, 
                                    const vector< pair<string, string> >& job_osb, 
				    const string& localdir, 
				    creamApiLogger* logger_instance, 
				    const bool noint, 
				    const int num_streams,
				    const string& initialCmd );
				    
bool checkdir( const string& localdir, const string& lcd_localdir, creamApiLogger* logger_instance, const bool noint );

//______________________________________________________________________________
void execute( AbsCreamProxy* client, 
	      const string& url ) throw()
{
  try {
    creamApiLogger::instance()->getLogger()->debug("Contacting service [%s]", url.c_str());
    client->execute(url);

  } catch(BaseException& ex ) {
      creamApiLogger::instance()->getLogger()->error( ex.what() );
      exit(1);
  } catch(exception& ex) {
    creamApiLogger::instance()->getLogger()->error( ex.what() );
    exit(1);
  }
}

/****************************************************************************
 *
 *       M A I N    F U N C T I O N
 *
 ***************************************************************************/
//______________________________________________________________________________
int main(int argc, char *argv[]) {
  
  string jobID;
  string endpoint;
  string param;
  bool debug = false;
  bool jobset=false;
  int option_index = 0;
  string logfile, statuses = "";
  bool jobids_from_file = false;
  vector<string> jobids;
  string input = "";
  map <string, vector<string> > url_jobids;
  vector<string> Status;
  
  bool redir_out = false;
  string    user_conf_file = "";
  boost::cmatch what;
  boost::regex pattern;
  string proxyfile, certfile;
  bool specified_proxyfile = false;
  creamApiLogger* logger_instance = creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();
  int timeout = 30;
  
  // Initialize the logger
  log_dev->setPriority( log4cpp::Priority::NOTICE );
  logger_instance->setLogfileEnabled( true );
  
  bool verify_ac_sign = true;
  bool list_only = false;
  
  string localdir = ".";

  int num_streams = 1;

  bool purge = true;
  bool noint = false;

  log_dev->setPriority( log4cpp::Priority::INFO );

  while( 1 ) {
    int c;
    static struct option long_options[] = {
      {"help", 0, 0, 'h'},
      {"debug", 0, 0, 'd'},
      {"version", 0, 0, 'v'},
      {"nomsg", 0, 0, 'n'},
      {"logfile", 1, 0, 'l'},
      {"input", 1, 0, 'i'},
      {"conf", 1, 0, 'c'},
      {"proxyfile", 1, 0, 'p'},
      {"timeout", 1, 0, 'T'},
      {"donot-verify-ac-sign", 0, 0, 'A'},
      {"dir", 1, 0, 'D'},
      {"num-streams", 1, 0, 's'},
      {"noint", 0, 0, 'N'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "hdAvl:nNLPi:f:t:c:p:T:D:s:", long_options, &option_index);
    if ( c == -1 )
      break;
    switch(c) {
    case 'N':
      noint = true;
      break;
    case 'P':
      purge = false;
      break;
    case 's':
      num_streams = atoi(optarg);
      break;
    case 'A':
      verify_ac_sign = false;
      break;
    case 'p':
      proxyfile = string(optarg);
      specified_proxyfile = true;
      break;
    case 'l':
      logfile = string(optarg);
      redir_out = true;
      break;
    case 'h':
      printhelp();
      exit(0);
      break;
    case 'D':
      localdir = optarg;
      break;
    case 'i':
      jobids_from_file = true;
      input = string(optarg);
      break;
    case 'v':
      help_messages::printver();
      exit(0);
      break;
      break;
    case 'n':
      log_dev->setPriority( log4cpp::Priority::ERROR );
      break;
    case 'L':
      list_only = true;
      break;
    case 'd':
      debug=true;
      log_dev->setPriority( log4cpp::Priority::DEBUG );
      break;
    case 'c':
      user_conf_file = string(optarg);
      break;
    case 'T':
      timeout = atoi(optarg);
      break;
    default:
      cerr << "Type " << argv[0] << " -h for an help" << endl;
      exit(1);
    }
  }
  
  
  /**
   *
   * Initializes CreamProxy interface and setup authN
   *
   */
  AbsCreamProxy *creamClient;
  
  string VO = "";
  
  if(!specified_proxyfile) {
    try {
      certfile = cliUtils::getProxyCertificateFile();
    } catch(exception& ex) {
      logger_instance->log(log4cpp::Priority::FATAL, ex.what(), true, true, true);
      exit(1);
    }
  } else
    certfile = proxyfile;
  
  log_dev->debug("Using certificate proxy file [%s]", certfile.c_str() );
  
  VOMSWrapper V( certfile, verify_ac_sign  );
  if( !V.IsValid( ) ) {
    logger_instance->log(log4cpp::Priority::FATAL, 
			 string("Problems with proxyfile [")+certfile+"]: "+
			 V.getErrorMessage(), true, true, true);
    exit(1);
    
  }
  
  VO = V.getVOName( );
  
  /**
   * Handle the load of configurations
   */
  vector<string> confiles;
  try {
    
    confiles = cliUtils::getConfigurationFiles(VO, 
					       user_conf_file, 
					       DEFAULT_CONF_FILE);
    
  } catch(no_user_confile_ex& ex) {
    log_dev->fatal( ex.what() );
    exit(1);
  }
  
  if ( log_dev->isInfoEnabled() ) {
    for(vector<string>::const_iterator fit = confiles.begin();
	fit != confiles.end();
	fit++) { 
      log_dev->debug( "Loading configurations from [%s]...", (*fit).c_str() );
    }
  }
  
  ConfigurationManager confMgr(ConfigurationManager::classad);
  
  try { confMgr.load(confiles); }
  catch(file_ex& ex) {
    if(debug)
    log_dev->warn( string(ex.what())+". Using built-in configuration" );
  }
  
  if(debug || redir_out) {
    if(!redir_out) {
      logfile = logger_instance->getLogFileName(confMgr.getProperty("JOBOUTPUT_LOG_DIR", DEFAULT_LOG_DIR).c_str(), "glite-ce-job-output");
    }
    log_dev->debug( "Logfile is [%s]", logfile.c_str() );
    cliUtils::mkdir(cliUtils::getPath(logfile));
    logger_instance->setLogFile(logfile.c_str());
  }
  
  if(!jobids_from_file && (argv[optind]==NULL)) {
    log_dev->fatal( "You didn't provide one or more JobID(s) to get output of. Must specify at least on option between --all or --input or specify a JobID as argument. Stop");
    exit(1);
  }

  /** ******************************************************************
   *
   * Load JobIDs from command line argument if any
   *
   */
  if(argv[optind]!=NULL) {
    jobset = true;
    while(argv[optind]!=NULL) {
      jobids.push_back(string(argv[optind]));
      optind++;
    }
  } 
  
  if(jobids_from_file) {
    bool isjoblist = true;
    int f;
    log_dev->debug( "Getting job list from file [%s]", input.c_str() );
    
    if((f=open(input.c_str(), O_RDONLY))==-1) {
      int saveerr = errno;
      log_dev->fatal( "Error accessing file [%s]: %s. Stop.", input.c_str(), strerror(saveerr));
      exit(1);
    }
    close(f);
    
    isjoblist = cliUtils::isACreamJobListFile(input.c_str());
    if(!isjoblist) {
      log_dev->fatal( "File [%s] is not a CREAM job list file. Stop.", input.c_str() );
      exit(1);
    }
    cliUtils::getJobIDFromFile(jobids, input.c_str());
  }

  Status.push_back("DONE-OK");
  Status.push_back("DONE-FAILED");
  Status.push_back("CANCELLED");
  Status.push_back("ABORTED");
  
  /**
   *
   * Prepare an hash table [CE-URL]->array_of[JobIDs]
   *
   */
  for(vector<string>::const_iterator jit = jobids.begin();
      jit != jobids.end();
      jit++) {
    if(log_dev->isInfoEnabled()) {
      string os = string("Processing job [") + *jit + "]...";
      log_dev->debug( os );
    }
    
    vector<string> pieces;
    try { CEUrl::parseJobID( *jit , pieces, confMgr.getProperty("DEFAULT_CREAM_TCPPORT", "8443")); }
    catch(CEUrl::ceid_syntax_ex& ex) {
      log_dev->error( ex.what() );
      log_dev->error( "This JobID doesn't contain the CREAM service address. Skipping it..." );
      continue;
    } 
    string tmp = confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
      pieces[1]+':'+pieces[2]+
      "/"+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
    url_jobids[tmp].push_back( *jit );
  }
  
  map<string, string> OSB_to_get;

  cout << endl;

  map<string, vector< pair<string, string> > > endpoint_job_osb;

  for(map<string, vector<string> >::iterator it = url_jobids.begin();
      it != url_jobids.end();
      it++) // iterates over endpoints
    {
      AbsCreamProxy::InfoArrayResult Iresult;

      vector<JobIdWrapper> target;
      stripCreamURL strip( &target, &confMgr );
      for_each(it->second.begin(), it->second.end(), strip);
      JobFilterWrapper filter(target, Status, 0, time(0), "", "");
      string url = it->first;
      creamClient = CreamProxyFactory::make_CreamProxyInfo( &filter, &Iresult, timeout );
      if(!creamClient)
	{
	  logger_instance->log(log4cpp::Priority::FATAL, 
			       "FAILED CREATION OF AN AbsCreamProxy object! STOP!!", 
			       true, true, true);
	  return 1;
	}
      boost::scoped_ptr< AbsCreamProxy > tmpClient;
      tmpClient.reset( creamClient );
      creamClient->setCredential( certfile );
      
      
      logger_instance->log(log4cpp::Priority::DEBUG, string("Connecting to [") + url + "]...", true, true, true);
      execute( creamClient, url );
      
      std::map< std::string, boost::tuple<JobInfoWrapper::RESULT, JobInfoWrapper, std::string> >::iterator Iit;
      Iit = Iresult.begin();
      for( ; Iit != Iresult.end( ); ++Iit ) {
	
	if( Iit->second.get<0>() == JobInfoWrapper::JOBUNKNOWN ) {
	  logger_instance->log(log4cpp::Priority::WARN, 
			       string("JobID [") + Iit->first + "] doest not exist", 
			       true, true, true);
	  continue;
	}
	
	if( Iit->second.get<0>() != JobInfoWrapper::OK ) {
	  logger_instance->log(log4cpp::Priority::WARN, 
			       string("JobID [") + Iit->first + "]:  " + Iit->second.get<2>(), 
			       true, true, true);
	  continue;
	}

	string info = string("JobID [") 
	  + Iit->first + "]: Will retrieve output from [" 
	  + Iit->second.get<1>().getCreamOSBURI( ) + "]";

	logger_instance->log(log4cpp::Priority::DEBUG, info, true, true, true);
	
	endpoint_job_osb[ url ].push_back( make_pair( Iit->first, Iit->second.get<1>().getCreamOSBURI( ) ) );
	
      }
    }
  // Now xfer the OSBs



  /**
   * Loop over the different endpoint
   */
   
  string cmd = confMgr.getProperty("UBERFTP_CLIENT", "/usr/bin/uberftp");
   
  map<string, vector< pair<string, string> > >::const_iterator eit = endpoint_job_osb.begin();
  for( ; eit != endpoint_job_osb.end( ); ++eit ) {
    
    string thisEndpoint = eit->first;
    
    boost::replace_all( thisEndpoint, confMgr.getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO), "" );
    boost::replace_all( thisEndpoint, string("/")+confMgr.getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX), "" );
    //cout << thisEndpoint << endl;
    
    retrieve_output_from_endpoint( thisEndpoint, eit->second, localdir, logger_instance, noint, num_streams, cmd );
    
  }
  
  return 0;
  
}

//------------------------------------------------------------------------------------------
void 
retrieve_output_from_endpoint( const string& endpoint, 
                               const vector< pair<string, string> >& job_osb, 
			       const string& localdir, 
			       creamApiLogger* logger_instance,
			       const bool noint,
			       const int num_streams,
			       const string& initialCmd ) {
  
  vector<pair<string, string> >::const_iterator jit = job_osb.begin();
  
  string cmd = initialCmd;//confMgr.getProperty("UBERFTP_CLIENT", "/usr/bin/uberftp");
  string thisPieceOfGet = "";
  for( ; jit != job_osb.end( ); ++jit ) {
    
    string remotepath = jit->second;
    string::size_type pos = remotepath.find( "://", 0 ); 
    if( pos == string::npos ) continue;
    remotepath = remotepath.substr( pos+3, remotepath.length( ) - 3 - pos );
    pos = remotepath.find( "/", 0 );
    remotepath = remotepath.substr( pos, remotepath.length( ) - pos );
    
    string localpath = jit->first;
    boost::replace_all( localpath, "http://", "" );
    boost::replace_all( localpath, "https://", "" );
    boost::replace_all( localpath, "/", "_" );
    boost::replace_all( localpath, ":", "_" );

    string lcd_localdir = localdir + "/" + localpath;
      
    if( !checkdir( localdir, lcd_localdir, logger_instance, noint ) )
      continue;
      
    logger_instance->log(log4cpp::Priority::INFO, string("For JobID [") + jit->first + "] output will be stored in the dir " + lcd_localdir, true, true, true);
    
    //fs::path P( fs::current_path( ) );
    
//     char *cwd, *ret;
//     do {
//       cwd = (char*)malloc(initial_cwdlen);
//       memset((void*)cwd,0,initial_cwdlen);
//       ret = getcwd(cwd, initial_cwdlen);
//       initial_cwdlen *= 2;
//     } while(!ret);
    
    //string origpath( P.string( ) );
    
    thisPieceOfGet += boost::str( boost::format("lcd %s; cd %s; mget *; lcd %s;") % lcd_localdir % remotepath %fs::current_path( ).string( ) );
    
  } // loop over vector<pair<job,osb>>
  
  string hostname = endpoint;
  string::size_type pos = hostname.find( ":", 0 );
  if(pos != string::npos)
    hostname = hostname.substr( 0, pos );
  
  
  
  
  cmd += " " + boost::str( boost::format("%s \"parallel %d; %s quit\"") % hostname % num_streams % thisPieceOfGet );
  
  logger_instance->log(log4cpp::Priority::DEBUG, string("Will execute command [") + cmd + "]", true, true, true);
  
  FILE *res = popen(cmd.c_str(), "r");
  if(!res) {
    logger_instance->log(log4cpp::Priority::ERROR, "popen failed starting UBERFTP. Stop.", true, true, true);
    return;
  }
  
  string output;
  while(!feof(res)) {
    output += fgetc(res);
  }
  if(pclose(res)) { // the command failed for some reason 
    string putlog = string("UBERFTP ERROR OUTPUT: ") + output;
    logger_instance->log(log4cpp::Priority::ERROR, putlog, true, true, true);
    return;
  }
  
}





//----------------------------------------------------------------------------------------------------
bool checkdir( const string& localdir, const string& lcd_localdir, creamApiLogger* logger_instance, const bool noint ) {

      /**
	 Checking if directory already exists
      */
      
      struct stat buf;
      
      bool localdir_exists = false;
      if(::stat( localdir.c_str(), &buf )) {
	if(errno == ENOENT) {
	  ; // OK
	} else {
	  logger_instance->log(log4cpp::Priority::FATAL, string("Failed stating of directory [") + localdir +"]: " + strerror(errno), true, true, true);
	  return false;
	}
      } else {
	if( !S_ISDIR(buf.st_mode) ) {
	  logger_instance->log(log4cpp::Priority::FATAL, string("Path [") + localdir +"] does exist but is not a directory", true, true, true);
	  return false;
	} else {
	  localdir_exists = true;
	}
      }
      
      if( !localdir_exists ) {
	if(localdir != "." ) {
	  try {
#ifdef NEWBOOSTFS
	    fs::path createP( localdir );
#else
	    fs::path createP( localdir, fs::native );
#endif
	    fs::create_directory( createP );
	  } catch(exception& ex) {
	    logger_instance->log(log4cpp::Priority::FATAL, 
				 string("Failed creation of directory [") 
				 + localdir +"]: " 
				 + ex.what(), 
				 true, true, true);
	    return false;
	  }
// 	  if(::mkdir( localdir.c_str(), 0777 )) {
// 	    logger_instance->log(log4cpp::Priority::FATAL, string("Failed creation of directory [") + localdir +"]: " + strerror(errno), true, true, true);
// 	    return false;
// 	  }
	}
      }

      bool lcd_localdir_exist = false;
      struct stat buf2;
      if(::stat( lcd_localdir.c_str(), &buf2 )) {
	if(errno == ENOENT) {
	  ; // OK
	} else {
	  logger_instance->log(log4cpp::Priority::FATAL, string("Failed stating of directory [") + lcd_localdir +"]: " + strerror(errno), true, true, true);
	  return false;
	}
      } else {
	if( !S_ISDIR(buf2.st_mode) ) {
	  logger_instance->log(log4cpp::Priority::FATAL, string("Path [") + lcd_localdir +"] does exist but is not a directory", true, true, true);
	  return false;
	} else {
	  lcd_localdir_exist = true;
	}
      }
      
      if( lcd_localdir_exist ) {//else {
        char answer;
	if(!noint) {
	  cout << "Destination path [" << lcd_localdir << "] already exists. Do you want overwrite it (y/n) ? ";
	  cin >> answer;
	} else answer = 'y';

	if(answer == 'y') {
#ifdef NEWBOOSTFS
	  fs::remove_all( fs::path( lcd_localdir ) );
#else
	  fs::remove_all( fs::path( lcd_localdir, fs::native ) );
#endif
	} else return false; // FIXME: COSA FARE se si risponde di no ?!?!?
      }  
      
      try {
#ifdef NEWBOOSTFS
        fs::path lcdP (lcd_localdir);
#else
        fs::path lcdP (lcd_localdir, fs::native);
#endif
	fs::create_directory( lcdP );
      } catch(exception& ex) {
	logger_instance->log(log4cpp::Priority::FATAL, 
			     string("Failed creation of directory [") 
			     + lcd_localdir +"]: " 
			     + ex.what(), 
			     true, true, true);
	return false;
      }

//       if(::mkdir( lcd_localdir.c_str(), 0755 ) )
//       {
// 	logger_instance->log(log4cpp::Priority::FATAL, string("Failed creation of directory [") + lcd_localdir +"]: " + strerror(errno), true, true, true);
// 	return false;
//       }
      
      return true;
}

//----------------------------------------------------------------------------------------------------
void printhelp(void) {
  cout << help_messages::getOutputHelp() << endl;
}
