
#include "glite/ce/es-client-api-c/InitDelegation.h"
#include "glite/ce/es-client-api-c/CallFactory.h"

//#include "glite/ce/cream-client-api-c/VOMSWrapper.h"

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

using namespace glite::ce::cream_client_api::soap_proxy;
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
    ("endpoint,e", po::value<string>(&endpoint), "Set the endpoint where renew the delegation")
    ("epr,E", po::value<string>(&EPR), "Set the EPR for the ActivityInfo")
    ("delegation-id,D", po::value<string>(&delegid), "The delegation identifier to renew");
    
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
           << "Usage: " << es_help_messages::delegrenew_usage
           << es_help_messages::common_opts
           << es_help_messages::delegrenew_custom_opts
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

  if(vm.count("conf") && !vm.count("epr")) {
    OperationEndpoint *op = get_OperationEndpoint( confile, endpoint, error );
    if(!op) {
      cerr << "Error parsing conf file: " << error << endl;
      cerr << "Default EPR will be used [" << EPR << "]..." << endl;
      cerr << "You can override the EPR with --epr <CUSTOM_EPR> option." << endl;
      
      //      return 1;
    } else {
      customepr = op->get_epr( "delegationrenew" );
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
   * Perform actual delegation renew
   *
   *
   */
  int duration = expirationtime-time(0);
  InitDelegation     init("RFC3820", &delegid, duration );
  AbstractCall *call = CallFactory::makeCall( deleg_service, &init );
    
  if(debug)
    cout << "Renewing delegation with ID [" 
         << delegid << "] to service [" << deleg_service << "]" << endl;
  if(!call->init( certfile, keyfile, error ))
    {
      cerr << error << endl;
      return 1;
    }
  
  if(debug)
    cout << "Sending Delegation Renew request to service ["
	 << deleg_service << "] for delegation ID ["
	 << delegid << "]" << endl;

  emi_es::client::comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( error, code ) ) {
    cerr << error << endl;
    return 1;
  }
  
  string ID;
  string cert;
  call->getResponse( ID, cert );
  
  //cout << "Server's CERT to sign [" << cert << "]\n\nfor deleg id [" << ID << "]";
  
  delete call;
  char *certtxt;
  if(debug)
    cout << "Signing service's certificate with our certificate file [" << certfile << "] and key file [" << keyfile << "]..." << endl;

    if (GRSTx509MakeProxyCert(&certtxt,
			      stderr,
			      (char*)cert.c_str(),
			      (char*)certfile.c_str(),
			      (char*)keyfile.c_str(),
			      (duration)/60))
      {
	error = "GRSTx509MakeProxyCert failed";
	return 1;
      }
    
    call = CallFactory::makeCall( deleg_service, ID, certtxt );
    
    if(!call)
      {
	error = "FAILED creation of an AbstractCall/PutDelegation object";
	return 1;
      }
    
    if(!call->init( certfile, keyfile, error ))
      return 1;
    
    if(debug)
      cout << "Sending signed proxy to service [" << deleg_service << "]..." << endl;

    if(!call->execute( error, code ) ) {
      cerr << "Fault while renewing delegation: " << error << endl;      
      return 1;
    }  
  cout << "Delegation with identifier [" << delegid 
       << "] successfully renewed" << endl;

  delete call;
  return 0;
}
