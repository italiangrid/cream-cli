
#include "glite/ce/es-client-api-c/CallFactory.h"
#include "glite/ce/es-client-api-c/util.h"

#include "common_checks.h"
#include "hmessages_es.h"
#include "conf_parser.h"

#include <string>
#include <vector>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

extern "C" {
#include "gridsite.h"
}

using namespace std;
using namespace emi_es::client::wrapper;
using namespace emi_es::client::comm;
using namespace emi_es::client::util;

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

SOAP_NMAC struct Namespace namespaces[] = {};

void soap_serialize_xsd__QName(soap*, 
			       std::basic_string<char, std::char_traits<char>, 
			       std::allocator<char> > const*) {}


//------------------------------------------------------------------------------
int main( int argc, char* argv[] ) {

  string  delegid("");
  string  endpoint("");
  string  certfile("");
  string  keyfile("");
  int     timeout;
  bool    debug = false;
  string  EPR = "/ce-cream-es/services/DelegationService";
  string  customepr = EPR;
  string  proxyfile("");

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
    ("endpoint,e", po::value<string>(&endpoint), "Set the endpoint where query delegation info")
    ("epr,E", po::value<string>(&EPR), "Set the EPR for the ActivityInfo")
    ("delegation-id,D", po::value<string>(&delegid), "The delegation identifier to query");
    
  po::positional_options_description p;
  p.add("delegation-id", -1);
        
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
           << "Usage: " << es_help_messages::deleginfo_usage
           << es_help_messages::common_opts
           << es_help_messages::deleginfo_custom_opts
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
  
  if(vm.count("conf") && !vm.count("epr")) {
    OperationEndpoint *op = get_OperationEndpoint( confile, endpoint, error );
    if(!op) {
      cerr << "Error parsing conf file: " << error << endl;
      cerr << "Default EPR will be used [" << EPR << "]..." << endl;
      cerr << "You can override the EPR with --epr <CUSTOM_EPR> option." << endl;
     
      //      return 1;
    } else {
      customepr = op->get_epr( "delegationinfo" );
      delete op;
    }
  }

  if(vm.count("epr") || customepr.empty())
    customepr = EPR;
  
  string deleg_service;
  deleg_service = string("https://") + endpoint + customepr;

  /**
   *
   *
   * Check delegation id
   *
   *
   */
  if(delegid.empty( )) {
    cerr << "The delegation identifier string is a mandatory argument. Stop." << endl;
    return 1;
  }

  /**
   *
   *
   * Perform actual delegation info call
   *
   *
   */
  AbstractCall *call = CallFactory::makeCall( deleg_service, delegid );
    
  if(debug)
    cout << "Asking info about delegation [" 
	 << delegid << "] to service [" << deleg_service << "]" << endl;

  if(!call->init( certfile, keyfile, error ))
    {
      cerr << error << endl;
      return 1;
    }
  
  emi_es::client::comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( error, code ) ) {
    cerr << error << endl;
    return 1;
  }
  
  time_t lifetime;
  string issuer;
  string subject;
  
  call->getResponse( lifetime, issuer, subject );
  
  cout << "Lifetime = " << time_to_string(lifetime) << endl;
  cout << "Issuer   = " << issuer<< endl;
  cout << "Subject  = " << subject<< endl;
  
  delete call;
  
  return 0;
}
