#include "cli_service_esjobsubmit.h"

#include "glite/ce/es-client-api-c/ActivityDescription.h"
#include "glite/ce/es-client-api-c/CreateActivities.h"
#include "glite/ce/es-client-api-c/CreateActivitiesResponse.h"
#include "glite/ce/es-client-api-c/CreateActivityDescriptionFromXML.h"
#include "glite/ce/es-client-api-c/CreateActivityCall.h"
#include "glite/ce/es-client-api-c/CallFactory.h"
#include "glite/ce/es-client-api-c/util.h"
#include "glite/ce/es-client-api-c/InitDelegation.h"
#include "glite/ce/es-client-api-c/util.h"

#include "glite/ce/cream-client-api-c/certUtil.h"

#include "util/cliUtils.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

extern "C" {
#include "gridsite.h"
}

using namespace std;
using namespace glite::ce::cream_cli::services;
using namespace emi_es::client;

//______________________________________________________________________________
cli_service_esjobsubmit::cli_service_esjobsubmit( const cli_service_common_options& opt )
  : cli_service( opt ), m_ceid(""), m_xmlfile( "" ), 
    m_automatic_delegation(false), m_delegation_id("")
{
}

//______________________________________________________________________________
//cli_service_esjobsubmit::
int cli_service_esjobsubmit::execute( void ) throw( )
{
  vector<wrapper::ActivityDescription> caVec;
  
  vector< pair<wrapper::ActivityDescription*, string> > actDesc;
  CreateActivityDescriptionFromXML::create( m_xmlfile, actDesc );
  
  vector< pair<wrapper::ActivityDescription*, string> >::const_iterator it;
  it = actDesc.begin();
  for( ; it != actDesc.end(); ++it ) {
    wrapper::ActivityDescription* current = it->first;
    if(!current) {
      cerr << it->second << endl;
      continue;
    }
    caVec.push_back( *current );
  }
  
  string VO_from_cert;

  if(!this->checkProxy( VO_from_cert, m_expiration_time, m_execution_fail_message ) ) {
    //this->freeHelpers();
    return 1;
  }

  if(!this->initConfigurationFile( VO_from_cert, m_execution_fail_message ) ) {
    //    this->freeHelpers();
    //cerr << "Failed creation of Configuration File." << endl;
    return 1;
  }
  

  wrapper::CreateActivities CA( caVec ); 

  vector<string> pieces, endpoint_pieces;

  boost::split(pieces, m_ceid, boost::is_any_of("/"));

  boost::split(endpoint_pieces, pieces[0], boost::is_any_of(":"));

  string endpoint = endpoint_pieces[0];
  int port = 8443;
  if(endpoint_pieces.size()==2)
    port = boost::lexical_cast<int>( endpoint_pieces[1].c_str() );
   
  string serviceAddress = this->getConfMgr()->getProperty("ES_URL_PREFIX", DEFAULT_PROTO );
  serviceAddress += cliUtils::getCompleteHostName(endpoint);
  serviceAddress += ":";
  serviceAddress += boost::lexical_cast<string>(port);
  serviceAddress += this->getConfMgr()->getProperty("ES_URL_POSTFIX_CREATE", 
 						    DEFAULT_ES_POSTFIX_CREATE);
   
  string delService = this->getConfMgr()->getProperty("ESDELEGATION_URL_PREFIX", DEFAULT_PROTO)+
    cliUtils::getCompleteHostName(endpoint) + ":" + boost::lexical_cast<string>(port)
    + this->getConfMgr()->getProperty("ESDELEGATION_URL_POSTFIX", DEFAULT_ESDELEGATION_POSTFIX);

//   if(m_automatic_delegation) {
//     if(!this->make_delegation( delService ) ) {
//       return 1;
//     }
//   }

  comm::AbstractCall* call = comm::CallFactory::makeCall( serviceAddress, &CA );
  
  if(!call) {
    m_execution_fail_message = "FAILED creation of an AbstractCall object! STOP!";
    return 1;
  }

  if ( !::getenv("NOAUTH") ) {
    if(!call->init( m_certfile, m_certfile, m_execution_fail_message ))
      return 1;
  } else {
    if(!call->init("", "", m_execution_fail_message ))
      return 1;
  }
  
  comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( m_execution_fail_message, code ) ) {
    return 1;
  }

  CreateActivityDescriptionFromXML::free( actDesc );
  
  vector<wrapper::ActivityCreationResponse> resps;
  call->getResponse( resps );
  
  vector<wrapper::ActivityCreationResponse>::const_iterator ait = resps.begin( );
  for( ; ait != resps.end( ); ++ait )
    {
      cout << "*****************************************\nActivityID     = " 
	   << ait->getActivityID( ) 
	   << endl;
      if( ait->getFault( ) )
	{
	  string err;
	  comm::AbstractCall::get_string_error( err, *ait->getFault( ) );
	  cerr << "FAULT: " << err << endl;
	}
      cout << "ActivityMgrURI = " << ait->getActivityManagerURI( ) << endl;
      cout << "Status         = " 
	   << ait->getActivityStatus( )->ACTIVITYSTATUS_STRING[ait->getActivityStatus( )->getStatusNum()]<<endl;
      vector<string> tgt;
      ait->getActivityStatus( )->getStatusAttributesString( tgt );
      cout << "Status Attrs   = {" << emi_es::client::util::join( tgt, ", " ) << "}" << endl;
      
      cout << "Timestamp      = " << ait->getActivityStatus( )->getTimestampString( ) << endl;
      cout << "Description    = " << ait->getActivityStatus( )->getDescription() << endl;
      cout << "ETNSC          = " << ait->getETNSCString( )<<endl;
      cout << "STAGEIN Dir    = {" << emi_es::client::util::join(ait->getStageInDirectory(), ", ") << "}"<<endl;
      cout << "SESSION Dir    = {" << emi_es::client::util::join(ait->getSessionDirectory(), ", ") << "}"<<endl;
      cout << "STAGEOUT Dir   = {" << emi_es::client::util::join(ait->getStageOutDirectory(), ", ") <<"}"<< endl;
      
    }
  
  delete call;
  return 0;
}

//______________________________________________________________________________
bool 
cli_service_esjobsubmit::make_delegation( const string& deleg_service ) 
{
  glite::ce::cream_client_api::certUtil::generateUniqueID( m_delegation_id );
  
  wrapper::InitDelegation init("X509", 0, m_expiration_time-time(0)-5 );
  
  comm::AbstractCall* call = comm::CallFactory::makeCall( deleg_service, &init );
  
  if(!call)
    {
      m_execution_fail_message = "FAILED creation of an AbstractCall/InitDelegation object! STOP!";
      return false;
    }

  if ( !::getenv("NOAUTH") ) {
    if(!call->init( m_certfile, m_certfile, m_execution_fail_message ))
      return false;
  } else {
    if(!call->init("", "", m_execution_fail_message ))
      return false;
  }

  comm::AbstractCall::SOAP_CALL_ERROR_CODES code;
  if(!call->execute( m_execution_fail_message, code ) ) {
    return false;
  }

  //string ID;
  string cert;
  call->getResponse( m_delegation_id, cert );
  delete call;

  char *certtxt;
  if (GRSTx509MakeProxyCert(&certtxt,
			    stderr,
			    (char*)cert.c_str(),
			    (char*)m_certfile.c_str(),
			    (char*)m_certfile.c_str(),
			    (m_expiration_time-time(0)-5)/60))
    {
      m_execution_fail_message = "Delegation proxy make failed: GRSTx509MakeProxyCert returned error";
      return false;
    }

  call = comm::CallFactory::makeCall( deleg_service, m_delegation_id, certtxt );

  if(!call)
    {
      m_execution_fail_message = "FAILED creation of an AbstractCall/PutDelegation object! STOP!";
      return false;
    }
  
  if ( !::getenv("NOAUTH") ) {
    if(!call->init( m_certfile, m_certfile, m_execution_fail_message ))
      return false;
  } else {
    if(!call->init("", "", m_execution_fail_message ))
      return false;
  }

  if(!call->execute( m_execution_fail_message, code ) ) {
    return false;
  }

  return true;
}
