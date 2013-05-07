#include "cli_service_get_cemon_url.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/ServiceInfoWrapper.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"

#include "util/cliUtils.h"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//______________________________________________________________________________
cli_service_get_cemon_url::cli_service_get_cemon_url( const cli_service_common_options& opt )
 : cli_service( opt ), m_url( "" )
{
}

//______________________________________________________________________________
int cli_service_get_cemon_url::execute( ) throw( ) {
 
  string VO = "";
  long int i;
  if(!this->checkProxy( VO, i, m_execution_fail_message ) ) {
    return 1;
  }
  
  if(!this->initConfigurationFile( VO, m_execution_fail_message ) ) {
    return 1;
  }

  this->set_logfile( "GETCEMONURL_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-get-cemon-url" );
  
  try{
    if( !cliUtils::checkEndpointFormat( m_endpoint ) )
      {
      
        m_execution_fail_message = "Endpoint not specified in the right format: should be <host>[:tcpport]; tcpport must be a positive number <= 65535. Stop.";
	return(1);
      }
  } catch(internal_ex& ex) {
  
    m_execution_fail_message = ex.what();
    return(1);
    
  }

  /**
   
   */
  try {
    if(!cliUtils::containsTCPPort(m_endpoint)) {
      m_endpoint = m_endpoint + ":" + this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443");
    }
  } catch(internal_ex& ex) {
  
    m_execution_fail_message = ex.what();
    return(1);
  }

  string serviceAddress = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)
    + m_endpoint+"/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);

//  if(m_logging)
    this->getLogger()->debug( "Service address=[%s]", serviceAddress.c_str() );

  apiproxy::ServiceInfoWrapper serviceInfoWrapper;
  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyServiceInfo(&serviceInfoWrapper, 2, m_soap_timeout );
  
  if(!m_creamClient) 
    {
      m_execution_fail_message = "FAILED TO CREATE AN AbsCreamProxy object!! STOP!";
      return 1;
    }
  
     try {
    m_creamClient->setCredential( m_certfile );
  } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    //this->getLogger()->fatal( ex.what( ) );
    m_execution_fail_message = ex.what();
    return 1;
  }
  
  try {

    m_creamClient->execute( serviceAddress );

  } catch(BaseException& ex) {
  
    m_execution_fail_message = ex.what();
  
    return(1);
  } catch(exception& ex) {
    m_execution_fail_message = ex.what();
    return(1);
  }

  //printf("Operation for enabling new submissions succeeded\n");

  m_url = serviceInfoWrapper.getCEMonURL().c_str();

  return 0;
}
