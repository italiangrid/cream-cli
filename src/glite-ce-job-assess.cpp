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

#include <string>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "glite/ce/cream-client-api-c/CreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "util/cliUtils.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/VectorString.h"
#include "hmessages.h"
#include <cerrno>
#include <getopt.h>
#include "glite/ce/cream-client-api-c/CREAMBinding.nsmap"
#include "glite/wmsutils/exception/Exception.h"
#include "glite/wms/jdl/JobAd.h"
#include "glite/wms/jdl/RequestAdExceptions.h"

//extern int errno;

using namespace std;

Namespace namespaces[] = {};

void printhelp();
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  VectorString OSB;   // OutputSandbox(es)
  VectorString OSBDU; // OutputSandboxDestURI(s)
  string URL;
  string JDLFile;
  string param;
  string VO;
  bool jdlset = false;
  bool mustcheckVO = false;
  bool debug = false;
  bool ceidset = false;
  bool pset = false;
  string logfile;
  bool redir_out = false;
  string outfile;
  bool assess_on_file = false;
  bool nomsg = false;
  bool fake = false;
  int option_index = 0;
  string user_VO;
  bool specified_vo = false;
  bool WRITE_LOG_ON_FILE = true;

  creamApiLogger* logger_instance = creamApiLogger::instance();
  log4cpp::Category* log_dev = logger_instance->getLogger();

  // Initialize the logger
  log_dev->setPriority( log4cpp::Priority::NOTICE );
  logger_instance->setLogfileEnabled( WRITE_LOG_ON_FILE );

  while(1) {
    int c;
    static struct option long_options[] = {
      {"help", 0, 0, 'h'},
      {"debug", 0, 0, 'd'},
      {"nomsg", 0, 0, 'n'},
      {"version", 0, 0, 'v'},
      {"fake", 0, 0, 'F'},
      {"resource", 1, 0, 'r'},
      {"logfile", 1, 0, 'l'},
      {"output", 1, 0, 'o'},
      {"assess-parameter", 1, 0, 'p'},
      {"vo", 1, 0, 'V'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "hdvr:FnV:o:l:p:", long_options, &option_index);

    switch(c) {
    case 'h':
      printhelp();
      exit(0);
      break;
      case 'V':
      user_VO = string(optarg);
      specified_vo = true;
      break;
    case 'v':
      help_messages::printver();
      exit(0);
    break;
    case 'd':
      debug=true;
      logger_instance->setPriority( log4cpp::Priority::INFO );
      break;
    case 'n':
      nomsg = true;
      logger_instance->setPriority::ERROR );
      break;
    case 'r':
      URL = string(optarg);
      ceidset=true;
      break;
    case 'l':
      redir_out = true;
      logfile = string(optarg);
      break;
    case 'o':
      assess_on_file = true;
      outfile = string(optarg);
      break;
      case 'F':
      fake = true;
      break;
    case 'p':
      pset = true;
      param = string(optarg);
      break;
    default:
      cerr << "Type " << argv[0] << " -h for an help"<<endl;
      exit(1);
    }
  }

  if(debug || redir_out) {
    if(!redir_out) logfile = logger_instance->getLogFileName("glite-ce-job-assess");
    logger_instance->setLogFile(logfile.c_str());
  }
  
  if(debug) {
    logger_instance->log(log4cpp::Priority::NOTSET, "*************************************", false, WRITE_LOG_ON_FILE, debug);
    logger_instance->log(log4cpp::Priority::NOTSET, help_messages::getStartMessage().c_str(), false, WRITE_LOG_ON_FILE, debug);
  }
  
  if(argv[optind]==NULL) {
    logger_instance->log(log4cpp::Priority::ERROR, "JDL file not specified in the command line arguments. Stop.", false, true, true);
    exit(1);
  } else {
    JDLFile = string(argv[optind]);
    jdlset = true;
  }

  if(!pset) {
    ostringstream os("");
    os << "assess parameter not specified in the command line arguments. Type "
	 <<argv[0]<< " -h for help";
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), false, true, true);
    exit(1);
  }
  if(!ceidset) {
    ostringstream os("");
    os << "CEId not specified in the command line arguments. Type "
	 <<argv[0]<< " -h for help";
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), false, true, true);
    exit(1);
  }

  if(!jdlset) {
    ostringstream os("");
    os << "JDL file not specified in the command line arguments. Type "
	 <<argv[0]<< " -h for help";
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), false, true, true);
    exit(1);
  }

  streambuf *psbuf;
  ofstream filestr;
  if(redir_out) {
	filestr.open(logfile.c_str(), ios::app);
	psbuf = filestr.rdbuf();
	cout.rdbuf(psbuf);
	cerr.rdbuf(psbuf);
  }

  vector<String> ceid_pieces;
  try {
    ceid_pieces = CEUrl::parseCEID(URL);
    
  } catch(ceid_syntax_ex& ex) {
    logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  string queName = ceid_pieces[1];
  struct stat buf;
  if(stat((const char*)JDLFile.c_str(), &buf) < 0) {
    int saveerr = errno;
    if(saveerr == ENOENT) {
      ostringstream os("");
      os << "JDL file " << JDLFile << " missing on disk. Stop." << endl;
      logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
      exit(1);
    }
    else {
      logger_instance->log(log4cpp::Priority::ERROR, strerror(saveerr), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  
  if(!(buf.st_mode & S_IRUSR)) {
    string err = String("JDL file ") + JDLFile + " is there but is not readable. Stop.";
    logger_instance->log(log4cpp::Priority::ERROR, err.c_str(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }

  glite::wms::jdl::JobAd job;
  /**
   *
   * Reads the JDL File from disk and filql a char array
   *
   */
  istream *is = new ifstream( (const char*) JDLFile.c_str(), ios::in );
  try {
    job.fromStream(*is);
  } 
  catch(glite::wms::jdl::AdSyntaxException& ex) {
    ostringstream os("");
    os <<"JDL Parsing Error: "<<ex.what();
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  catch(glite::wms::jdl::AdClassAdException& ex) {
    ostringstream os("");
    os << "JDL Parsing Error: "<<ex.what()<<endl;
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  catch(glite::wms::jdl::AdMismatchException& ex) {
    ostringstream os("");
    os << "JDL Parsing Error: "<<ex.what()<<endl;
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  catch(glite::wms::jdl::AdListException& ex) {
    ostringstream os("");
    os << "JDL Parsing Error: "<<ex.what()<<endl;
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  catch(glite::wms::jdl::AdFormatException& ex) {
    ostringstream os("");
    os << "JDL Parsing Error: "<<ex.what()<<endl;
    logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }
  /**
   *
   * Cheks that "executable" JDL attribute
   * is there
   *
   */
  try {
    if(debug)
      logger_instance->log(log4cpp::Priority::INFO, "Checking for 'executable' attribute...", true, WRITE_LOG_ON_FILE, true);
    job.getStringValue("executable");

  } catch(glite::wmsutils::exception::Exception ex) {
    if(ex.getExceptionName().compare("AdEmptyException") == 0) {
      logger_instance->log(log4cpp::Priority::INFO, "Missing 'executable' mandatory attribute in the JDL. Stop.", true, WRITE_LOG_ON_FILE, true);
      exit(1);
    }
    logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, WRITE_LOG_ON_FILE, true);
    exit(1);
  }

  try {
    if(debug)
      logger_instance->log(log4cpp::Priority::INFO, "Checking for 'OutputSandbox' attribute'...", true, WRITE_LOG_ON_FILE, true);
    OSB = job.getStringValue("OutputSandbox");
    // if the exception has not been raised it means that the OUTPUTSANDBOX
    // ATTRIBUTE HAS BEEN SPECIFIED.

    // If there're elements in the OutputSandBox let's check for
    // OutputSandboxDestURI...
    if(OSB.size() > 0) {
      try {
	if(debug)
	  logger_instance->log(log4cpp::Priority::INFO, "Checking for 'OutputSandboxDestURI' attribute'...", true, WRITE_LOG_ON_FILE, true);
	OSBDU = job.getStringValue("OutputSandboxDestURI");
	if( (OSBDU.size() > 1) && (OSBDU.size()!=OSB.size())) {
	  ostringstream os("");
	  os << "OutputSandBoxDestURI specified with a number of destinations >1 "
	       <<"and that differs from the number of elements of OutputSandBox. Stop."<<endl;
	  logger_instance->log(log4cpp::Priority::ERROR, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
	  exit(1);
	}
      } catch(glite::wmsutils::exception::Exception ex) {
	// OUTPUTSANDBOX HAS BEEN SPECIFIED BUT OUTPUTSANDBOXDESTURI HASN'T
	// STOP
	logger_instance->log(log4cpp::Priority::ERROR, "OutputSandbox has been specified without OutputSandboxDestURI. Stop.", true, WRITE_LOG_ON_FILE, true);
	exit(1);
      }
    }
  } catch(glite::wmsutils::exception::Exception ex) {
    // there's not OUTPUTSANDBOX. DO NOTHING... it's a user choice
  }


  for(int j=0; j<OSB.size(); j++) {
    if(OSB.at(j).compare("") == 0)
      {
	logger_instance->log(log4cpp::Priority::ERROR, "Blank value in 'OutputSandBox' elements. Stop.", true, WRITE_LOG_ON_FILE, true);
	exit(1);
      }
  }

  for(int j=0; j<OSBDU.size(); j++) {
    if(OSBDU.at(j).compare("") == 0)
      {
	logger_instance->log(log4cpp::Priority::ERROR, "Blank value in 'OutputSandBoxDestURI' elements. Stop.", true, WRITE_LOG_ON_FILE, true);
	exit(1);
      }
  }
  
  
  try {
    if(debug)
      logger_instance->log(log4cpp::Priority::INFO, "Checking for 'Type' attribute'...", true, WRITE_LOG_ON_FILE, true);
    vector<string> type = job.getStringValue("Type");
    if(type.size() > 1)  {
      if(!nomsg)
        logger_instance->log(logger::WARN, "more than one velue for 'Type' attribute. Only the first will be considered.", true, WRITE_LOG_ON_FILE, true);
    }
    if(type.at(0).compare("Job") != 0)
      {
	logger_instance->log(log4cpp::Priority::ERROR, "CREAM only supports 'Job' as value of 'Type' attribute. Stop.", true, WRITE_LOG_ON_FILE, true);
	exit(1);
      }
  } catch(glite::wmsutils::exception::Exception ex) {}
  
  try {
    if(debug)
      logger_instance->log(log4cpp::Priority::INFO, "Checking for 'JobType' attribute'...", true, WRITE_LOG_ON_FILE, true);
    vector<string> jtype = job.getStringValue("JobType");
    if(jtype.size() > 1)  {
      if(!nomsg)
        logger_instance->log(logger::WARN, "more than one velue for 'JobType' attribute. Only the first will be considered.", true, WRITE_LOG_ON_FILE, true);
    }
    if(jtype.at(0).compare("Normal") != 0)
      {
	logger_instance->log(log4cpp::Priority::ERROR, "CREAM only supports 'Normal' as value of 'JobType' attribute. Stop.", true, WRITE_LOG_ON_FILE, true);
	exit(1);
      }
  } catch(glite::wmsutils::exception::Exception ex) {}


  CreamProxy *initSoap;
  try {
    initSoap = new CreamProxy(false);
    // initSoap->printOnConsole(!nomsg);
    // initSoap->printDebug(debug);
    string certfile = cliUtils::getProxyCertificateFile();
    if(debug) 
      logger_instance->log(log4cpp::Priority::INFO, string("Using certificate proxy file [")
			     + certfile + "]", true, WRITE_LOG_ON_FILE, true);
    VO = initSoap->Authenticate(certfile);
    if(VO.compare("") != 0) {
      if(job.hasAttribute("VirtualOrganisation")) {
        if(!nomsg) {
	  ostringstream os("");
	  os <<"VirtualOrganisation specified in the JDL but overriden with ["<<VO<<"]";
	  logger_instance->log(logger::WARN, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
	}
	job.delAttribute(string("VirtualOrganisation"));
      }
      job.addAttribute(string("VirtualOrganisation"), VO);
    } else {
      mustcheckVO = true;
    }
  } catch(soap_ex& s) {
    cerr << s.what() << endl;
    exit(1);
  } catch(auth_ex& auth) {
    cerr << auth.what() << endl;
    exit(1);
  } catch(auth_NO_VO_ex& auth) {
    mustcheckVO = true;
  }

  if(mustcheckVO) {
    if(debug)
      logger_instance->log(logger::WARN, "No VO information. Considering VirtualOrganisation specified in the command line options...", true, WRITE_LOG_ON_FILE, true);
    if(specified_vo) {
	if(job.hasAttribute("VirtualOrganisation")) {
        if(!nomsg) {
	  ostringstream os("");
	  os <<"VirtualOrganisation specified in the JDL but overriden with ["<<user_VO<<"]";
	  logger_instance->log(logger::WARN, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
	}
	job.delAttribute(string("VirtualOrganisation"));
      }
      job.addAttribute(string("VirtualOrganisation"), user_VO);
    } else {
	logger_instance->log(logger::WARN, "no VO information specified in the command line options. Checking for VirtualOrganisation in JDL file...", true, WRITE_LOG_ON_FILE, true);

      try {
        job.getStringValue("VirtualOrganisation");
      }
      catch(glite::wms::jdl::AdEmptyException& empty) {
	logger_instance->log(log4cpp::Priority::ERROR, "Missing 'VirtualOrganisation' attribute in the JDL file. Stop.", true, WRITE_LOG_ON_FILE, true);
	exit(1);
      }
      catch(glite::wmsutils::exception::Exception ex) {
	logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, WRITE_LOG_ON_FILE, true);
        exit(1);
      }
    }
  }

  job.addAttribute(string("QueueName"), ceid_pieces[3]);
  job.addAttribute(string("BatchSystem"), ceid_pieces[2]);
  string JDLBuffer = job.toString();

  /**
   *
   */
  string serviceAddress = CEUrl::getCreamURI(ceid_pieces[0]+":"+ceid_pieces[1]);
  if(debug) {
    ostringstream os("");
    os <<"service address=[" << serviceAddress <<"]";
    logger_instance->log(log4cpp::Priority::INFO, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
  }
  if(debug) {
    ostringstream os("");
    os <<"Sending JDL [" << JDLBuffer <<"]";
    logger_instance->log(log4cpp::Priority::INFO, os.str().c_str(), true, WRITE_LOG_ON_FILE, true);
  }

  if(fake) return 0;

  string assess;
  int err = 0;
  try {
    assess = initSoap->Assess(serviceAddress.c_str(), JDLBuffer, param);
  } catch(soap_ex& ex) {
    logger_instance->log(log4cpp::Priority::ERROR, ex.what(), true, WRITE_LOG_ON_FILE, true);
    err = 1;
  }
  if(assess_on_file) {
    ofstream assessout;
    assessout.open(outfile.c_str(), ios::trunc);
    assessout.write(assess.c_str(), assess.length());
    assessout.close();
  } else printf("%s\n", assess.c_str());

  return err;
}
}

void printhelp() {
  printf("%s\n",help_messages::getAssessHelp().c_str());
}
