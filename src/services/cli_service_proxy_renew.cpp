#include "cli_service_proxy_renew.h"

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
cli_service_proxy_renew::cli_service_proxy_renew( const cli_service_common_options& opt )
 : cli_service( opt )
{
}

//______________________________________________________________________________
int cli_service_proxy_renew::execute( ) throw( ) 
{
  string VO = "";
  long int i;
  if(!this->checkProxy( VO, i, m_execution_fail_message ) ) {
    return 1;
  }
  
  if(!this->initConfigurationFile( VO, m_execution_fail_message ) ) {
    return 1;
  }

  this->set_logfile( "PROXYRENEW_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-proxy-renew" );
  
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
  
  

  string service =
      this->getConfMgr()->getProperty("CREAMDELEGATION_URL_PREFIX", DEFAULT_PROTO)
      + m_endpoint+"/"
      + this->getConfMgr()->getProperty("CREAMDELEGATION_URL_POSTFIX", DEFAULT_CEURLDELEGATION_POSTFIX);
//  if(m_logging)  
    this->getLogger()->debug( string("Delegating proxy on service [") + service + "] with ID [" + m_delegation_id + "] ...");
  
  
  string certtxt;
  
  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxy_ProxyRenew(m_delegation_id, m_soap_timeout );
  
  if(!m_creamClient)
    {
      m_execution_fail_message = "FAILED TO CREATE AN AbsCreamProxy object! STOP!";
      return(1);
    }
  
  try{     
    
    m_creamClient->setCredential( m_certfile );	
    m_creamClient->execute( service );
    
  } catch(BaseException& ex) {
    m_execution_fail_message = ex.what() ;
    return 1;
  } catch(exception& ex) {
    m_execution_fail_message = ex.what();
    return 1;
  }
  
  return 0;
}
