#include "cli_service_service_info.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"

#include "util/cliUtils.h"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//________________________________________________________________________
string time_t_to_string( time_t tval ) {
  char buf[26]; // ctime_r wants a buffer of at least 26 bytes
  ctime_r( &tval, buf );
  if(buf[strlen(buf)-1] == '\n')
    buf[strlen(buf)-1] = '\0';
  return string( buf );
}


//______________________________________________________________________________
cli_service_service_info::cli_service_service_info( const cli_service_common_options& opt, apiproxy::ServiceInfoWrapper* info )
 : cli_service( opt ), m_verbosity( 2 ), m_service_info( info )
{
}

//______________________________________________________________________________
int cli_service_service_info::execute( ) throw( ) {
 
  string VO = "", errmex = "";
  long int i;
  if(!this->checkProxy( VO, i, errmex ) ) {
    m_execution_fail_message = errmex;
    return 1;
  }
  
  if(!this->initConfigurationFile( VO, errmex ) ) {
    m_execution_fail_message = errmex;
    return 1;
  }

  this->set_logfile( "GETSERVICEINFO_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-service-info" );
  
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

  if (this->getLogger()->isInfoEnabled()) {
//    if(m_logging)
      this->getLogger()->info( "Service address=[%s]", serviceAddress.c_str() );
  }

  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyServiceInfo( m_service_info, m_verbosity, m_soap_timeout );

  if(!m_creamClient) 
    {
      this->getLogger()->fatal( "FAILED CREATION OF AN AbsCreamProxy OBJECT! STOP!" );
      return 1;
    }

  m_creamClient->setCredential( m_certfile );

  try {
    m_creamClient->execute( serviceAddress );
  } catch(BaseException& ex) {
    m_execution_fail_message = ex.what();
    return(1);
  } catch(exception& ex) {
    m_execution_fail_message = ex.what();
    return(1);
  }

  return 0;
}

