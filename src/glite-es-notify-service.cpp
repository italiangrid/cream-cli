
#include "glite/ce/es-client-api-c/InternalBaseFault.h"
#include "glite/ce/es-client-api-c/CallFactory.h"
#include "glite/ce/es-client-api-c/util.h"


#include <string>
#include <vector>
#include <utility>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include "common_checks.h"
#include "hmessages_es.h"
#include "conf_parser.h"


using namespace std;
using namespace emi_es::client::wrapper;
using namespace emi_es::client::util;
using namespace emi_es::client::comm;


namespace po       = boost::program_options;

SOAP_NMAC struct Namespace namespaces[] = {};

void soap_serialize_xsd__QName(soap*, 
			       std::basic_string<char, std::char_traits<char>, 
			       std::allocator<char> > const*) {}

int main ( int argc, char* argv[] ) {

  vector<string>  notify_messages;
  string          endpoint("");
  string          certfile("");
  string          keyfile("");
  int             timeout;
  bool            debug = false;
  string          EPR = "/ce-cream-es/services/ActivityManagementService";
  string          customepr = EPR;
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
    ("endpoint,e", po::value<string>(&endpoint), "Set the endpoint to send the activity status request")
    ("epr,E", po::value<string>(&EPR), "Set the EPR for the ActivityInfo")
    ("notify-message,n", po::value<vector<string> >(&notify_messages),
     "Set the activity identifiers and messages to notify to the server; each element must be in the form <ActivityID>:<message>; currently supported messages are \"CLIENT-DATAPULL-DONE\" and \"CLIENT-DATAPUSH-DONE\"");
  
  po::positional_options_description p;
  p.add("notify-message", -1);
  
  
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
	   << "Usage: " << es_help_messages::notify_usage
	   << es_help_messages::common_opts
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

  if(!notify_messages.size()) {
    cerr << "At least one notify message is a mandatory argument. Stop." << endl;
    return 1;
  }

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
      customepr = op->get_epr( "notify" );
      delete op;
    }
  }

  if(vm.count("epr") || customepr.empty())
    customepr = EPR;
  
  
  if(::getenv("ES_CLI_NOAUTH"))
    endpoint = string("http://") + endpoint + customepr;
  else 
    endpoint = string("https://") + endpoint + customepr;
  
  /**
   *
   * Setup SOAP call's  arguments
   *
   */
  vector<pair<string, AbstractCall::NOTIFYMESSAGE> > items;
  for( vector<string>::const_iterator it = notify_messages.begin();
       it != notify_messages.end();
       ++it ) {
    vector<string> pieces;
    boost::split( pieces, *it, boost::is_any_of(":") );
    if( pieces.size() < 2 ) {
      cerr << "Skipping argument [" << *it 
	   << "] because is not in the format <ActivityID>:<message>" 
	   << endl;
	continue;
    }
    if(pieces[1]=="CLIENT-DATAPULL-DONE") {
      items.push_back( make_pair( pieces[0], AbstractCall::CLIENT_DATAPULL_DONE) );
      continue;
    }
    if(pieces[1]=="CLIENT-DATAPUSH-DONE") {
      items.push_back( make_pair( pieces[0], AbstractCall::CLIENT_DATAPUSH_DONE) );
      continue;
    }
    cerr << "Message [" << pieces[1] << "] not supported. Skipping."<<endl;
  }

  if(!items.size())
    return 0;


  /**
   * 
   *
   * Create SOAP object for remote call and invoke service's method
   *
   */
  AbstractCall *call = CallFactory::makeCall( endpoint, items );

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
    cout << "Sending NotifyService request " 
	 << "to service [" 
	 << endpoint
	 << "]" 
	 << endl;
  
  
  emi_es::client::comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( error, code ) ) {
    cerr << error << endl;
    return 1;
  }

  vector<pair<string, InternalBaseFault> > resp;
  call->getResponse( resp );

  for(vector<pair<string, InternalBaseFault> >::const_iterator it = resp.begin( );
      it != resp.end( );
      ++it)
    {
      cout << "*****************************************\n";
      cout << "ERROR for Activity ID = " << it->first << endl;
      cout << "Message     = " << it->second.Message<<endl;

      if(it->second.Timestamp)
	cout << "Timestamp   = " 
	     << time_to_string(*it->second.Timestamp)
	     << endl;

      if(it->second.Description)
	cout << "Description = " << *it->second.Description 
	     << endl;

      if(it->second.FailureCode)
	cout << "FailCode    = " 
	     << boost::lexical_cast<string>(*it->second.FailureCode) 
	     << endl;
    }

  delete call;
  
}
