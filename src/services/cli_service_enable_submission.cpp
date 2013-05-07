#include "cli_service_enable_submission.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"

#include "util/cliUtils.h"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//______________________________________________________________________________
cli_service_enable_submission::cli_service_enable_submission( const cli_service_common_options& opt )
 : cli_service( opt )
{
}

//______________________________________________________________________________
int cli_service_enable_submission::execute( ) throw( ) {
 
  string VO = "";
  long int i;
  if(!this->checkProxy( VO, i, m_execution_fail_message  )) {
    return 1;
  }
  
  if(!this->initConfigurationFile( VO, m_execution_fail_message ) ) {
    return 1;
  }

  this->set_logfile( "ENABLESUB_LOG_DIR", DEFAULT_LOG_DIR, "glite-ce-enable-submission" );
  
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


  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyAcceptNewJobSubmissions( true,  m_soap_timeout );
  
  if(!m_creamClient) 
    {
      m_execution_fail_message = "FAILED TO CREATE AN AbsCreamProxy object! STOP!";
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

    m_creamClient->execute( serviceAddress, true );

  } catch(BaseException& ex) {
    m_execution_fail_message = ex.what();
    return(1);
  } catch(exception& ex) {
    m_execution_fail_message = ex.what();
    return(1);
  }

  return 0;
}
