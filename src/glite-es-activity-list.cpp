
#include "glite/ce/es-client-api-c/ListActivityCall.h"
#include "glite/ce/es-client-api-c/ListActivities.h"
#include "glite/ce/es-client-api-c/util.h"
#include "glite/ce/es-client-api-c/CallFactory.h"

#include "common_checks.h"
#include "hmessages_es.h"
#include "conf_parser.h"

#include <string>
#include <vector>
#include <iostream>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace emi_es::client::wrapper;
using namespace emi_es::client::comm;
using namespace emi_es::client::util;

namespace po       = boost::program_options;

SOAP_NMAC struct Namespace namespaces[] = {};

void soap_serialize_xsd__QName(soap*, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const*) {}

//------------------------------------------------------------------------------
int main( int argc, char* argv[] ) {

  int      limit;
  string   states_and_attrs;
  string   endpoint("");
  string   certfile("");
  string   keyfile("");
  int      timeout;
  string   fromDate(""), toDate("");
  bool     debug = false;
  string   EPR = "/ce-cream-es/services/ActivityInfoService";
  string   customepr = EPR;
  string   outputfile("");
  string   base_endpoint("");
  string   proxyfile("");
  
 
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
     "status-filter,S",
     po::value<string>(&states_and_attrs),
     "Set the status/status-attributes filter; can be specified by comma separated strings; each string must have the format <statusname>[:statusattr]"
     )
    (
     "from-date,F",
     po::value<string>(&fromDate),
     "Set the lower limit time filter"
     )
    (
     "to-date,T",
     po::value<string>(&toDate),
     "Set the upper limit time filter"
     )
    (
     "limit,l",
     po::value<int>(&limit),
     "Set the maximum number of entries to ask the server"
)
    ("endpoint,e", 
     po::value<string>(&endpoint), 
     "Set the endpoint to send the activity status request")
    (
     "output,o",
     po::value<string>(&outputfile),
     "Set the output file where write the Activity ID"
     )
    ("epr,E", po::value<string>(&EPR), "Set the EPR for the ActivityInfo");

  po::positional_options_description p;
  p.add("endpoint", -1);

        
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
	   << "Usage: " << es_help_messages::list_usage
	   << es_help_messages::common_opts
	   << es_help_messages::list_custom_opts
	   << es_help_messages::create_custom_opts// this is why --output option is the same for create-activity
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
  
  base_endpoint = endpoint;
  
  if(vm.count("conf") && !vm.count("epr")) {
    OperationEndpoint *op = get_OperationEndpoint( confile, endpoint, error );
    if(!op) {
      cerr << "Error parsing conf file: " << error << endl;
      cerr << "Default EPR will be used [" << EPR << "]..." << endl;
      cerr << "You can override the EPR with --epr <CUSTOM_EPR> option." << endl;
      
      //      return 1
    } else {
      customepr = op->get_epr( "list" );
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
   *
   *
   * Check status and attr filters
   *
   *
   *
   *
   */
  map<string, ActivityStatus::ACTIVITYSTATUS> allowedStates;
  map<string, ActivityStatus::ACTIVITYSTATUSATTRS> allowedAttrs;
  vector<ActivityStatus::ACTIVITYSTATUS> states;
  vector<ActivityStatus::ACTIVITYSTATUSATTRS> statesattrs;

  allowedStates["ACCEPTED"]             = (ActivityStatus::ACTIVITYSTATUS)0;
  allowedStates["PREPROCESSING"]        = (ActivityStatus::ACTIVITYSTATUS)1;
  allowedStates["PROCESSING"]           = (ActivityStatus::ACTIVITYSTATUS)2;
  allowedStates["PROCESSING_ACCEPTING"] = (ActivityStatus::ACTIVITYSTATUS)3;
  allowedStates["PROCESSING_QUEUED"]    = (ActivityStatus::ACTIVITYSTATUS)4;
  allowedStates["PROCESSING_RUNNING"]   = (ActivityStatus::ACTIVITYSTATUS)5;
  allowedStates["POSTPROCESSING"]       = (ActivityStatus::ACTIVITYSTATUS)6;
  allowedStates["TERMINAL"]             = (ActivityStatus::ACTIVITYSTATUS)7;

  allowedAttrs["VALIDATING"]                = (ActivityStatus::ACTIVITYSTATUSATTRS)0;
  allowedAttrs["SERVER_PAUSED"]             = (ActivityStatus::ACTIVITYSTATUSATTRS)1;
  allowedAttrs["CLIENT_PAUSED"]             = (ActivityStatus::ACTIVITYSTATUSATTRS)2;
  allowedAttrs["CLIENT_STAGEIN_POSSIBLE"]   = (ActivityStatus::ACTIVITYSTATUSATTRS)3;
  allowedAttrs["CLIENT_STAGEOUT_POSSIBLE"]  = (ActivityStatus::ACTIVITYSTATUSATTRS)4;
  allowedAttrs["PROVISIONING"]              = (ActivityStatus::ACTIVITYSTATUSATTRS)5;
  allowedAttrs["EPROVISIONING"]             = (ActivityStatus::ACTIVITYSTATUSATTRS)6;
  allowedAttrs["SERVER_STAGEIN"]            = (ActivityStatus::ACTIVITYSTATUSATTRS)7;
  allowedAttrs["SERVER_STAGEOUT"]           = (ActivityStatus::ACTIVITYSTATUSATTRS)8;
  allowedAttrs["BATCH_SUSPEND"]             = (ActivityStatus::ACTIVITYSTATUSATTRS)9;
  allowedAttrs["APP_RUNNING"]               = (ActivityStatus::ACTIVITYSTATUSATTRS)10;
  allowedAttrs["PREPROCESSING_CANCEL"]      = (ActivityStatus::ACTIVITYSTATUSATTRS)11;
  allowedAttrs["PROCESSING_CANCEL"]         = (ActivityStatus::ACTIVITYSTATUSATTRS)12;
  allowedAttrs["POSTPROCESSING_CANCEL"]     = (ActivityStatus::ACTIVITYSTATUSATTRS)13;
  allowedAttrs["VALIDATION_FAILURE"]        = (ActivityStatus::ACTIVITYSTATUSATTRS)14;
  allowedAttrs["PREPROCESSING_FAILURE"]     = (ActivityStatus::ACTIVITYSTATUSATTRS)15;
  allowedAttrs["PROCESSING_FAILURE"]        = (ActivityStatus::ACTIVITYSTATUSATTRS)16;
  allowedAttrs["POSTPROCESSING_FAILURE"]    = (ActivityStatus::ACTIVITYSTATUSATTRS)17;
  allowedAttrs["APP_FAILURE"]               = (ActivityStatus::ACTIVITYSTATUSATTRS)18;
  allowedAttrs["EXPIRED"]                   = (ActivityStatus::ACTIVITYSTATUSATTRS)19;
  
  vector<string> states_and_attrs_pieces;
  if(!states_and_attrs.empty()) {
    boost::split(states_and_attrs_pieces, states_and_attrs, boost::is_any_of(","));

    vector<string>::const_iterator it  = states_and_attrs_pieces.begin( );
    vector<string>::const_iterator end = states_and_attrs_pieces.end( );
    for( ; it != end; ++it )
      {
	if(debug)
	  cout << "Processing filter item [" << *it << "]" << endl;
	vector<string> pieces;
	boost::split( pieces, *it, boost::is_any_of(":") );
	if(allowedStates.find(pieces[0]) == allowedStates.end() )
	  {
	    cerr << "Status [" << pieces[0] << "] not allowed. Skipping..." << endl;
	    continue;
	  }
	states.push_back( allowedStates[pieces[0]] );
	
	if(pieces.size()==2) {
	  if( allowedAttrs.find(pieces[1]) == allowedAttrs.end( ) ) {
	    cerr << "Status attribute [" << pieces[1] << "] not allowed. Skipping..." << endl;
	    // continue;
	  } else
	    statesattrs.push_back( allowedAttrs[pieces[1]] );
	}
      }
  }
  /**
   *
   *
   * Check the presence and the format of time filters
   *
   *
   */
  boost::regex pattern;
  boost::cmatch what;
  time_t *SINCE_timestamp = 0, *TO_timestamp = 0;
  if( !fromDate.empty( ) ) {
    pattern = "^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})\\s([0-9][0-9]:[0-9][0-9]:[0-9][0-9])\\b";
    if( !boost::regex_match(fromDate.c_str(), what, pattern) ) {
      cerr << "Specified from date is wrong; must have the format 'YYYY-MM-DD HH:mm:ss'. Stop" << endl;
      return 1;
    }
    struct tm tmp;
    strptime(fromDate.c_str(), "%Y-%m-%d %T", &tmp);
    SINCE_timestamp = new time_t( mktime(&tmp) );
  }
  
  if( !toDate.empty( ) ) {
    pattern = "^([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})\\s([0-9][0-9]:[0-9][0-9]:[0-9][0-9])\\b";
    if( !boost::regex_match(toDate.c_str(), what, pattern) ) {
      cerr << "Specified to date is wrong; must have the format 'YYYY-MM-DD HH:mm:ss'. Stop" << endl;
      return(1);
    }
    struct tm tmp;
    strptime(toDate.c_str(), "%Y-%m-%d %T", &tmp);
    TO_timestamp = new time_t( mktime(&tmp) );
  }

  /**
   *
   *
   * Check the presence of the limit filter
   *
   *
   */
  int *l = 0;
  if( vm.count("limit") ) {
    l = new int( limit );
  }


  /**
   *
   *
   * Setup the call's arguments
   *
   *
   */
  ListActivities list( SINCE_timestamp, TO_timestamp, l, states, statesattrs );
  
  AbstractCall* call = CallFactory::makeCall( endpoint, &list );
  
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
    cout << "Sending ActivityList request to service ["<< endpoint << "]" << endl;

  emi_es::client::comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( error, code ) ) {
    cerr << error << endl;
    return 1;
  }
  cout << endl << "SOAP BUFFER=[" << call->get_soap_buffer( ) << "]" << endl << endl;
  vector<string> IDS;
  bool trunc;
  call->getResponse( trunc, IDS );
  
  vector<string> ids_to_print;

  vector<string>::const_iterator idit = IDS.begin();
  for( ; idit != IDS.end( ); ++idit ) {
    cout << *idit <<  endl;
    if(!outputfile.empty())
      ids_to_print.push_back( *idit );
  }
  
  delete call;
  
  set<string> already_present_ids;
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
	if(pieces[1] != base_endpoint) {
	  cerr << "This output file [" << outputfile << "] contains"
	       << " activity identifiers created a on different endpoint ("
	       << pieces[1] << "). Will not write Activity ID on this file."
	       << endl;
	  wrong_output = true;
	}
	if(!wrong_output) { //load all IDs already present in output file
	  string line("");
	  while(!in.eof()) {
	    line = "";
	    in >> line;
	    boost::trim_right_if(line, boost::is_any_of("\n"));
	    already_present_ids.insert( line );
	  }
	}
      }
      
      out.open(outputfile.c_str(), ios::app);
      
    }
    else {
      out.open(outputfile.c_str());
      out << "##ACTIVITY_IDS@"<< base_endpoint << "##" << endl;
    }
    
    if(!wrong_output) {
      for(vector<string>::const_iterator it=ids_to_print.begin( );
	  it != ids_to_print.end( );
	  ++it )
	if(already_present_ids.find(*it)==already_present_ids.end())
	  out << *it << endl;
    }
  }
  return 0;
}
