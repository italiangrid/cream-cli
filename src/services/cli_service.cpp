#include "util/cliUtils.h"
#include "cli_service.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "hmessages.h"

using namespace std;
namespace cream_cli = glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//________________________________________________________________________________________________
cream_cli::cli_service::cli_service( const cli_service_common_options& opt )
  : m_confMgr( 0 ), m_log_dev( 0 ),
    m_debug( opt.m_debug ), m_nomsg( opt.m_nomsg ), m_noint( opt.m_noint ), 
    m_verify_ac_sign( opt.m_verify_ac_sign ), m_redir_out( opt.m_redir_out ), 
    m_user_conf_file( opt.m_user_conf_file ), m_certfile( opt.m_certfile ), 
    m_logfile( opt.m_logfile ), m_endpoint( opt.m_endpoint ), m_soap_timeout( opt.m_soap_timeout ),
    m_creamClient( 0 ),
    m_esClient( 0 ),
    m_initialized( false ),
    m_execution_fail_message( "" )
//    m_logging( true )
{
  m_log_dev = glite::ce::cream_client_api::util::creamApiLogger::instance()->getLogger();

  if(m_debug)
    m_log_dev->setPriority( log4cpp::Priority::DEBUG );
  else
    m_log_dev->setPriority( log4cpp::Priority::WARN );
  
  if(m_redir_out)
    glite::ce::cream_client_api::util::creamApiLogger::instance()->setLogfileEnabled( true );
  
  // if(m_debug)
  //   m_log_dev->setPriority( log4cpp::Priority::DEBUG );
  if(m_nomsg)
    m_log_dev->setPriority( log4cpp::Priority::ERROR );

  glite::ce::cream_client_api::util::creamApiLogger::instance()->setMaxLogFileSize( -1 );
}

//________________________________________________________________________________________________
bool cream_cli::cli_service::checkProxy( string& VO, long int& expiryDate, string& errmex ) throw( )
{
  /**
     Check proxy certificate and, if valid, extract VO name
  */
  try {
    
//    if ( m_logging ) {
	m_log_dev->debug("Using certificate proxy file [%s]", m_certfile.c_str() );
//    }
    
    apiproxy::VOMSWrapper V( m_certfile, m_verify_ac_sign );
    if( !V.IsValid( ) ) {
      errmex = string("Problems with proxyfile [")+m_certfile+"]: "+ V.getErrorMessage();
      return false;
    }    
    
    VO = V.getVOName( );
    expiryDate = V.getProxyTimeEnd();
    
  } catch(apiproxy::soap_ex& s) {
    errmex = s.what();
    return false;
  } catch(apiproxy::auth_ex& auth) {
    errmex = auth.what();
    return false;
  }
  return true;
}

//________________________________________________________________________________________________
bool cream_cli::cli_service::initConfigurationFile( const string& VO, string& errmex ) throw( )
{
  /**
    Check and load configuration from CONF FILE
  */
  vector<string> confiles;
  try {
    confiles = cliUtils::getConfigurationFiles(VO,
					       m_user_conf_file,
					       DEFAULT_CONF_FILE);
  } catch(no_user_confile_ex& ex) {
    errmex = ex.what();
    return false;
  }
  
  m_confMgr = new apiutil::ConfigurationManager(apiutil::ConfigurationManager::classad);
  if(!m_confMgr) {
    
    errmex = "Failed allocation of pointer m_confMgr! Stop!";
    return false;
  }
  
  try { m_confMgr->load(confiles); }
  catch(apiutil::file_ex& ex) {
    if( m_debug /*&& m_logging*/ )
      m_log_dev->warn( string(ex.what())+". Using built-in configuration" );
  }
  
  return true;
}

//________________________________________________________________________________________________
void cream_cli::cli_service::set_logfile( const char* logdir, const char* def_logdir, const char* cmd_name )
{
  if(m_debug || m_redir_out) {
      if(!m_redir_out ) {
          m_logfile = apiutil::creamApiLogger::instance()->getLogFileName( m_confMgr->getProperty( logdir, def_logdir ).c_str(), cmd_name );
      }
      
//      if(m_logging)
        this->getLogger()->debug( "Logfile is [%s]", m_logfile.c_str() );
      
      cliUtils::mkdir(cliUtils::getPath( m_logfile ));
      apiutil::creamApiLogger::instance()->setLogFile(m_logfile.c_str());
      
  }
}
