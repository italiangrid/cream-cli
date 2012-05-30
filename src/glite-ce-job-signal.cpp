/*
  Copyright (c) Members of the EGEE Collaboration. 2004. 
  See http://www.eu-egee.org/partners/ for details on the copyright
  holders.  
  
  Licensed under the Apache License, Version 2.0 (the "License"); 
  you may not use this file except in compliance with the License. 
  You may obtain a copy of the License at 
  
      http://www.apache.org/licenses/LICENSE-2.0 
  
  Unless required by applicable law or agreed to in writing, software 
  distributed under the License is distributed on an "AS IS" BASIS, 
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
  See the License for the specific language governing permissions and 
  limitations under the License.
*/

/*
        glite-ce-job-signal: a command line interface to send a signal to a job submitted to cream

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: glite-ce-job-signal.cpp,v 1.26.12.1.2.2.2.3 2012/04/16 17:00:14 adorigo Exp $
*/               
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include "glite/ce/cream-client-api-c/CREAMBinding.nsmap"
#include "glite/ce/cream-client-api-c/CreamProxy.h"
//#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "hmessages.h"

using namespace std;

Namespace namespaces[] = {};

void printhelp(void);
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  string signal;
  string jobID;
  string URL;
  bool debug = false;
  bool hostset = false;
  bool jobset=false;
  bool sigset=false;
  bool nomsg=false;
  char opt;
  //  creamApiLogger* logger_instance = creamApiLogger::instance();
  //  log4cpp::Category* log_dev = logger_instance->getLogger();

  // Initialize the logger
  log_dev->setPriority( log4cpp::Priority::NOTICE );
  logger_instance->setLogfileEnabled( WRITE_LOG_ON_FILE );

  while( -1 != (opt = getopt(argc, argv, "nsdhr:"))) {
    switch(opt) {
    case 'h':
      printhelp();
      exit(0);
      break;
    case 'n':
      nomsg=true;
      log_dev->setPriority( log4cpp::Priority::ERROR );
      break;
    case 'd':
      debug=true;
      log_dev->setPriority( log4cpp::Priority::INFO );
      break;
      // 		case 'j':
      // 		jobID = string(optarg);
      // 	        jobset=true;
      // 		break;
    case 'r':
      URL = string(optarg);
      hostset=true;
      break;
    case 's':
      signal = string(optarg);
      sigset=true;
      break;
    default:
      cerr << "Unrecognized option "<<opt<<endl;
      cerr << "Type " << argv[0] << " -h for an help"<<endl;
      exit(1);
    }
  }
  
  if(argv[optind]==NULL) {
    cerr << "Job ID not specified in the command line arguments. Stop."<<endl;
    exit(1);
  } else {
    jobID = string(argv[optind]);
    jobset = true;
  }

  if(!hostset) {
     if(debug)
      cerr << "<host>:<port> not explicitly specified. Using address embedded in JobID"<<endl;
  }
  
  if(!sigset) {
    cerr << "Signal not specified in the command line arguments. Type "<<argv[0]<< " -h for help"<<endl;
    exit(1);
  }
  
  CreamProxy *initSoap;
  try {
    initSoap = new CreamProxy(false);
    initSoap->printDebug(debug);
//    initSoap->printOnConsole(!nomsg);
    string certfile = cliUtils::getProxyCertificateFile();
     if(debug) 
       logger_instance->log(log4cpp::Priority::INFO, string("Using certificate proxy file [")
			      + certfile + "]", true, WRITE_LOG_ON_FILE, true);
     initSoap->Authenticate(certfile);
  } catch(soap_ex& s) {
  	cerr << s.what() << endl;
	exit(1);
  } catch(auth_ex& auth) {
     	cerr << auth.what() << endl;
	exit(1);
  } catch(auth_NO_VO_ex& auth) {}
  
  /**
   * 
   */
  string serviceAddress;
  if(hostset) {
    serviceAddress = CEUrl::getCreamURI(URL);
  } else {
    VectorString pieces;
    try { pieces = CEUrl::parseJobID(jobID); }
    catch(ceid_syntax_ex& ex) {
	cerr <<ex.what()<<endl;
	exit(1);
    }
    serviceAddress = CEUrl::getCreamURI(pieces[1]+":"+pieces[2]);
  }
  if(debug) {
    cerr << "service address=" << serviceAddress << endl;
    cerr<< "Job ID="<<jobID<<endl;
  }

   try {
      //initSoap->signal(serviceAddress.c_str(), jobID, signal);
   } catch(soap_ex& ex) {
      cerr << ex.what() << endl;
      exit(1);
   }
   return 0;
}

void printhelp(void) {
}
