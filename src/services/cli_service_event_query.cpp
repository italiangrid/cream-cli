#include "cli_service_event_query.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/EventWrapper.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"

#include "util/cliUtils.h"

#include "boost/lexical_cast.hpp"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//______________________________________________________________________________
cli_service_event_query::cli_service_event_query( const cli_service_common_options& opt, std::list<apiproxy::EventWrapper*>* target )
 : cli_service( opt ), m_nevents(100), m_status( vector<pair<string,string> >() ), m_evtStart("0"), m_evtEnd("99"), m_result( target )
{
}

//______________________________________________________________________________
void cli_service_event_query::set_events( const string& start, const string& end ) 
{
  m_evtStart = start;
  m_evtEnd = end ;
}

//______________________________________________________________________________
void cli_service_event_query::set_num_events( const int nevents ) 
{
  m_nevents = nevents;
}

//______________________________________________________________________________
void cli_service_event_query::set_status( const vector<pair<string,string> >& states ) 
{
  m_status = states;
}

//______________________________________________________________________________
cli_service_event_query::~cli_service_event_query( ) throw( )
{
}

//______________________________________________________________________________
int cli_service_event_query::execute( void ) throw( )
{
  string VO = "";
  long int i;
  if(!this->checkProxy( VO, i, m_execution_fail_message ) ) {
     return 1;
  }

  if(!this->initConfigurationFile( VO, m_execution_fail_message ) ) {
    return 1;
  }

  this->set_logfile( "EVENTQUERY_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-event-query" );
  

  if( !cliUtils::checkEndpointFormat( m_endpoint ) )
    {
      m_execution_fail_message = "Endpoint empty or not specified in the right format: should be <host>[:tcpport]. Stop.";
      return(1);
    }

  /**
   *
   */
  try {
    if(!cliUtils::containsTCPPort( m_endpoint )) {
      m_endpoint = m_endpoint + ":" + this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443");
    }
  } catch(internal_ex& ex) {
    m_execution_fail_message = ex.what();
    return(1);
  }

  string serviceAddress = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
    m_endpoint+"/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);

//  if ( m_logging ) {
      this->getLogger()->debug( "Service address=[%s]", serviceAddress.c_str() );
//  }
  
  /**
  
  
  
  
  */
  time_t exec_time;
  string db_id;
  
  m_creamClient = 
    apiproxy::CreamProxyFactory::make_CreamProxy_QueryEvent( make_pair(m_evtStart, m_evtEnd),make_pair( (time_t)-1, (time_t)-1 ), "JOB_STATUS", m_nevents,0,m_status,exec_time,db_id,*m_result,m_soap_timeout );

  if(!m_creamClient)
    {
      m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
      return 1;
    }

     try {
    m_creamClient->setCredential( m_certfile );
  } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    //this->getLogger()->fatal( ex.what( ) );
    m_execution_fail_message = ex.what();
    return 1;
  }
    
  string baseServiceAddr = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO) + m_endpoint +"/";

  try {

     m_creamClient->execute(serviceAddress);
     
  } catch (BaseException& ex) {
     m_execution_fail_message = ex.what();
     return 1;
  } catch (exception& ex) {
     m_execution_fail_message = ex.what();
     return 1;
  }

//  if(m_logging)
    this->getLogger()->debug( string("Database ID = [") + db_id  + "]");
  
  string et;
  try {
    et = boost::lexical_cast<string>(exec_time);
  }catch( boost::bad_lexical_cast&) {}
  
//  if(m_logging)
    this->getLogger()->debug( string("Exec time   = [") + et/*tmp.str()*/ + "ms]" );
  
  return 0;
}
