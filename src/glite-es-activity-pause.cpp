#include "glite/ce/es-client-api-c/InternalBaseFault.h"
#include "glite/ce/es-client-api-c/CallFactory.h"

#include "common_checks.h"
#include "hmessages_es.h"
#include "conf_parser.h"

#include <string>
#include <vector>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace emi_es::client::comm;
using namespace emi_es::client::wrapper;
using namespace emi_es::client::util;

namespace po       = boost::program_options;

SOAP_NMAC struct Namespace namespaces[] = {};

void soap_serialize_xsd__QName(soap*, 
			       basic_string<char, char_traits<char>,
			       allocator<char> > const*) {}

int main ( int argc, char* argv[] ) {

  vector<string>  aIDs;
  string          endpoint("");
  string          certfile("");
  string          keyfile("");
  int             timeout;
  string          fromDate(""), toDate("");
  bool            debug = false;
  string          EPR = "/ce-cream-es/services/ActivityManagementService";
  string          customepr = EPR;
  string          inputFile("");
  string          proxyfile("");

  uid_t pid = ::getuid( );
  string default_certfile = "/tmp/x509up_u" + boost::lexical_cast<string>(pid);
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
     "activity-id,a",
     po::value<vector<string> >(&aIDs),
     "The activity identifier(s) to pause"
     )
    (
     "endpoint,e", 
     po::value<string>(&endpoint), 
     "Set the endpoint to send the activity pause request"
     )
    (
     "epr,E",
     po::value<string>(&EPR),
     "Set the EPR for the ActivityInfo"
     )
    (
     "input,i",
     po::value<string>(&inputFile),
     "Set the input file where to find the Activity identifiers to pause"
     );

  po::positional_options_description p;
  p.add("activity-id", -1);
        
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
	   << "Usage: " << es_help_messages::pause_usage
	   << es_help_messages::common_opts
	   << es_help_messages::pause_custom_opts
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
  if( vm.count("input") && vm.count("endpoint") ) {
    cerr << "Options --endpoint and --input are mutually exclusive. Stop." << endl;
    return 1;
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
    if(!common_check(endpoint, !inputFile.empty(), certfile.c_str(), !vm.count("donot-verify-ac-sign"), expirationtime, error) ){
      cerr << error << endl;
      return 1;
    }
  } else {
    // in this case the certfile will NOT be "VOMS-validated"
    if(!common_check(endpoint, !inputFile.empty(), 0, !vm.count("donot-verify-ac-sign"), expirationtime, error) ){
      cerr << error << endl;
      return 1;
    } 
  }
  
  
  /**
   *
   *
   *
   * Check there's at least one activity id or
   * that the input file has been specified
   *
   *
   *
   */
  if(aIDs.size()==0 && inputFile.empty()) {
    cerr << "Must specify at least one Activity Identifier as mandatory argument or an input file as argument of the '--input' option. Stop." << endl;
    return 1;
  }
  
  /**
   *
   * Check the input file format, extract Activity IDS and endpoint
   *
   */
  vector<string> IDS;
  if(!inputFile.empty()) {
    if(!check_input_format(inputFile, endpoint, IDS, error)) {
      cerr << error << ". Stop." << endl;
      return 1;
    }
  }
  
  if(vm.count("conf") && !vm.count("epr")) {
    OperationEndpoint *op = get_OperationEndpoint( confile, endpoint, error );
    if(!op) {
      cerr << "Error parsing conf file: " << error << endl;
      cerr << "Default EPR will be used [" << EPR << "]..." << endl;
      cerr << "You can override the EPR with --epr <CUSTOM_EPR> option." << endl;
     
      //      return 1;
    } else {
      customepr = op->get_epr( "pause" );
      delete op;
    }
  }

  if(vm.count("epr") || customepr.empty())
    customepr = EPR;
  
  if(::getenv("ES_CLI_NOAUTH"))
    endpoint = string("http://") + endpoint + customepr;
  else 
    endpoint = string("https://") + endpoint + customepr;

  for( vector<string>::const_iterator it = IDS.begin( );
       it != IDS.end( );
       ++it )
    {
      if(debug)
	cout << "Adding Activity Identifier [" << *it <<"] from input file..." << endl;
      aIDs.push_back(*it);
    }

  AbstractCall* call = CallFactory::makeCall( endpoint, aIDs, AbstractCall::PAUSE );
  
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

  if(debug)
    cout << "Sending ActivityPause request for ActivityID(s) {" 
	 << join(aIDs, ", ") 
	 << "} to service [" 
	 << endpoint
	 << "]" 
	 << endl;
  
  emi_es::client::comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( error, code ) ) {
    cerr << error << endl;
    return 1;
  }
  
  map<string, long long int> successful;
  map<string, InternalBaseFault> failed;
  
  call->getResponse( successful, failed );
  
  {
    map<string, long long int>::const_iterator it;
    for( it = successful.begin( ); it != successful.end( ); ++it )
      {
	cout << "*****************************************" << endl;
	cout << it->first << "  -->  Estimated exec time: ";
	cout << ((it->second == -1) ? "undefined" : boost::lexical_cast<string>(it->second)) << endl;
      }
  }
  
  {
    map<string, InternalBaseFault>::const_iterator it;
    for( it = failed.begin( ); it != failed.end( ); ++it )
      {
	cout << "*****************************************" << endl;
	cout << it->first << "  -->  Fault:";
	cout << it->second.toString( ) << endl;
      }
  }
  
  delete call;

}
