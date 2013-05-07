/**
 *
 *
 */
#include "glite/ce/es-client-api-c/ActivityDescription.h"
#include "glite/ce/es-client-api-c/CreateActivities.h"
#include "glite/ce/es-client-api-c/CreateActivitiesResponse.h"
#include "glite/ce/es-client-api-c/CreateActivityDescriptionFromXML.h"
#include "glite/ce/es-client-api-c/CreateActivityCall.h"
#include "glite/ce/es-client-api-c/CallFactory.h"
#include "glite/ce/es-client-api-c/util.h"

#include "common_checks.h"
#include "hmessages_es.h"
#include "conf_parser.h"

#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace emi_es::client::wrapper;
using namespace emi_es::client::comm;
using namespace emi_es::client;
using namespace emi_es::client::util;

namespace po       = boost::program_options;

SOAP_NMAC struct Namespace namespaces[] = {};

void soap_serialize_xsd__QName(soap*, 
			       std::basic_string<char, std::char_traits<char>, 
			       std::allocator<char> > const*) 
{}

//------------------------------------------------------------------------------
int main( int argc, char* argv[] ) {
  
  string  ADLFile("");
  string  endpoint("");
  string  base_endpoint("");
  string  certfile("");
  string  keyfile("");
  int     timeout;
  bool    debug = false;
  string  outputfile("");
  string  EPR = "/ce-cream-es/services/CreationService";
  string  customepr = EPR;
  string  proxyfile("");

  uid_t   pid = ::getuid( );
  string  default_certfile = "/tmp/x509up_u" + boost::lexical_cast<string>(pid);
  string  confile("");

  certfile  = default_certfile;
  keyfile   = default_certfile;
  proxyfile = default_certfile;

  po::options_description desc("Usage");
  desc.add_options()
    ("help,h", "display this help and exit")
    (
     "debug,d",
     "Activate verbose logging"
     )
    (
     "conf,C",
     po::value<string>(&confile),
     "Sets the configuration file"
     )
    (
     "proxyfile,p",
     po::value<string>(&proxyfile),
     "Set the proxy filename"
     )
    (
     "certfile,c",
     po::value<string>(&certfile),
     "Set the proxy filename"
     )
    (
     "keyfile,k",
     po::value<string>(&keyfile),
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
    (
     "output,o",
     po::value<string>(&outputfile),
     "Set the output file where write the Activity ID"
     )
    ("endpoint,e", po::value<string>(&endpoint), "Set the endpoint to send the activity to")
    ("epr,E", po::value<string>(&EPR), "Set the EPR for the ActivityInfo")
    ("adl-file,a", po::value<string>(&ADLFile), "Set the \"Activity Description\" filename");

  po::positional_options_description p;
  p.add("adl-file", -1);

        
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
  
  if(vm.count("help"))
    {
      cout << endl << es_help_messages::VersionID 
	   << endl
	   << es_help_messages::reportTo
	   << endl
	   << endl
	   << "Usage: " << es_help_messages::create_usage
	   << es_help_messages::common_opts
	   << es_help_messages::create_custom_opts
	   << endl;
      return 0;
    }

  if( (vm.count("certfile") && !vm.count("keyfile")) ||
      (!vm.count("certfile") && vm.count("keyfile")))
    {
      cerr << "--certfile option must be used with --keyfile "
	   << "option and vice-versa. Stop." << endl;
      return 1;
    }

  if( (vm.count("certfile") && vm.count("proxyfile"))) {
    cerr << "--certfile and --proxyfile are mutually exclusive. Stop." << endl;
    return 1;
  }

  if(vm.count("proxyfile")) {
    certfile = proxyfile;
    keyfile  = proxyfile;
  }

  if(vm.count("debug"))
    debug = true;


  time_t expirationtime;

  /**
   *
   * Invoke the function for endpoint check and proxy file validation
   *
   */
  string error;
  if(!vm.count("certfile")) {
    // in this case the certfile will be "VOMS-validated"
    if(!common_check(endpoint, false, certfile.c_str(), !vm.count("donot-verify-ac-sign"), expirationtime, error) ){
      cerr << error << endl;
      return 1;
    }
  } else {
    // in this case the certfile will NOT be "VOMS-validated"
    if(!common_check(endpoint, false, 0, !vm.count("donot-verify-ac-sign"), expirationtime, error) ){
      cerr << error << endl;
      return 1;
    }
  }
  
  base_endpoint = endpoint;

  if(vm.count("conf") && !vm.count("epr")) {
    OperationEndpoint *op = get_OperationEndpoint( confile, endpoint, error );
    if(!op) {
      cerr << "Error parsing conf file: " << error << endl;
      cerr << "Default EPR will be used [" << EPR << "]..." << endl;
      cerr << "You can override the EPR with --epr <CUSTOM_EPR> option." << endl;
      
      //      return 1;
    } else {
      customepr = op->get_epr( "create" );
      delete op;
    }
  }

  if(vm.count("epr") || customepr.empty())
    customepr = EPR;
  
  if(::getenv("ES_CLI_NOAUTH"))
    endpoint = string("http://") + endpoint + customepr;
  else 
    endpoint = string("https://") + endpoint + customepr;
  
  if(ADLFile.empty( )) {
    cerr << "ADL filename is a mandatory command argument. Stop." << endl;
    return 1;
  }


  /**
   *
   *
   *
   * Convert ADL XML nodes into C++/SOAP objects
   *
   *
   *
   *
   */
  vector<ActivityDescription> caVec;

  if(debug)
    cout << "Parsing XML file [" << ADLFile << "] and converting to SOAP objects..." << endl;
  
  vector< pair<ActivityDescription*, string> > actDesc;
  try {
    CreateActivityDescriptionFromXML::create( ADLFile, actDesc );
  } catch(string& ex) {
    cerr << ex << endl;
    return 1;
  }
  vector< pair<ActivityDescription*, string> >::const_iterator it;
  it = actDesc.begin();
  for( ; it != actDesc.end(); ++it ) {
    ActivityDescription* current = it->first;
    if(!current) {
      cerr << "ADL error: " << it->second << endl;
      continue;
    }
    caVec.push_back( *current );
  }

  if(!caVec.size())
    {
      cerr << "All Activity Descriptions specified in the ADL file had problems and have been discarded. Stop." << endl;
      return 1;
    }

  if(debug)
    cout << "There're " << caVec.size() << " activity/ies to submit" << endl;

  CreateActivities CA( caVec ); 

  /**
   *
   *
   *
   * Setup remote object call
   *
   *
   *
   *
   */
  AbstractCall* call = CallFactory::makeCall( endpoint, &CA );
  call->set_soap_timeout( timeout );

  if ( !::getenv("ES_CLI_NOAUTH") ) {
    if(!call->init( certfile, keyfile, error ))
      {
	cerr << error << endl;
	return 1;
      }
  } else {
    if(!call->init("", "", error ))
      {
	cerr << error << endl;
	return 1;
      }
  }
  
  

  /**
   *
   *
   *
   * Invoke remote ES's operation CreateActivity
   *
   *
   *
   *
   */ 
  if(debug)
    cout << "Sending CreateActivity request to service [" << endpoint << "]" << endl;

  AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( error, code ) ) {
    cerr << error << endl;
    return 1;
  }
  cout << endl << "SOAP BUFFER=[" << call->get_soap_buffer( ) << "]" << endl << endl;
  CreateActivityDescriptionFromXML::free( actDesc );
  

  /**
   *
   *
   *
   * Parse CreateActivity's response and print out Activity's info
   *
   *
   *
   *
   */ 
  vector<ActivityCreationResponse> resps;
  call->getResponse( resps );
  
  vector<ActivityCreationResponse>::const_iterator ait = resps.begin( );
  for( ; ait != resps.end( ); ++ait ) {
    
    cout << "*****************************************\nActivityID     = " 
	 << ait->getActivityID( ) 
	 << endl;
    if( ait->getFault( ) ) {
      string err;
      AbstractCall::get_string_error( err, *ait->getFault( ) );
      cerr << "FAULT: " << err << endl;
    }
    cout << "ActivityMgrURI = " << ait->getActivityManagerURI( ) << endl;
    if(ait->getActivityStatus( ))
      cout << "Status         = " 
	   << ait->getActivityStatus( )->ACTIVITYSTATUS_STRING[ait->getActivityStatus( )->getStatusNum()]<<endl;
    else
      cout << "Status         = N/A" << endl;
    if(ait->getActivityStatus( )) {
      vector<string> tgt;
      ait->getActivityStatus( )->getStatusAttributesString( tgt );
      cout << "Status Attrs   = {" << join( tgt, ", " ) << "}" << endl;
      
      cout << "Timestamp      = " << ait->getActivityStatus( )->getTimestampString( ) << endl;
      cout << "Description    = " << ait->getActivityStatus( )->getDescription() << endl;
    }
    cout << "ETNSC          = " << ait->getETNSCString( )<<endl;
    cout << "STAGEIN Dir    = {" << join(ait->getStageInDirectory(), ", ") << "}"<<endl;
    cout << "SESSION Dir    = {" << join(ait->getSessionDirectory(), ", ") << "}"<<endl;
    cout << "STAGEOUT Dir   = {" << join(ait->getStageOutDirectory(), ", ") <<"}"<< endl;
    
    if(!outputfile.empty()) {
      bool wrong_output = false;
      ofstream out;
      if(boost::filesystem::exists(outputfile)) {
	string header;
	{
	  ifstream in(outputfile.c_str());
	  in >> header;
	  boost::trim_left_if(header, boost::is_any_of("#"));
	  boost::trim_right_if(header, boost::is_any_of("#"));
	  vector<string> pieces;
	  boost::split(pieces, header, boost::is_any_of("@"));
	  if(pieces[1] != (base_endpoint)) {
	    cerr << "This output file [" << outputfile << "] contains"
		 << " activity identifiers created a on different endpoint ("
		 << pieces[1] << "). Will not write Activity ID on this file."
		 << endl;
	    wrong_output = true;
	  }
	}

	out.open(outputfile.c_str(), ios::app);

      }
      else {
	out.open(outputfile.c_str());
	out << "##ACTIVITY_IDS@"<< base_endpoint << "##" << endl;
      }

      if(!wrong_output)
	out << ait->getActivityID( ) << endl;
    }
  }
  
  // Free heap memory
  delete call;

  // OK!
  return 0;
}
