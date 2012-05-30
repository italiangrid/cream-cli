#ifndef GLITE_CE_CREAM_CLI_SERVICE
#define GLITE_CE_CREAM_CLI_SERVICE

/**
 * CREAM CLIENT LIB STUFF
 */
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"

/**
 * ES CLIENT LIB STUFF
 */
#include "glite/ce/es-client-api-c/AbstractCall.h"

#include "cli_service_common_options.h"

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service {
	
	private:
	
	  log4cpp::Category* 					    m_log_dev;
	
	protected:
	
	  glite::ce::cream_client_api::util::ConfigurationManager*  m_confMgr;

	  bool			m_debug;
	  bool			m_nomsg;
	  bool			m_noint;
	  bool			m_verify_ac_sign;
	  bool			m_redir_out;
	  std::string		m_user_conf_file;
	  std::string		m_certfile;
	  std::string		m_logfile;
	  std::string		m_endpoint;
	  int			m_soap_timeout;
	  
	  bool			m_initialized;
	  
	  std::string		m_execution_fail_message;
	  
//	  bool			m_logging;
	  
	  glite::ce::cream_client_api::soap_proxy::AbsCreamProxy*  m_creamClient;
	  emi_es::client::comm::AbstractCall*                      m_esClient; 
	  
	public:
	  virtual ~cli_service( ) throw( ) { delete m_confMgr; delete m_creamClient; delete m_esClient; }
	  cli_service( const cli_service_common_options& );
	  virtual int execute( void ) throw() = 0;
	  
	  bool checkProxy( std::string& vo, long int&, std::string& errmex ) throw( );

	  bool initConfigurationFile( const std::string& VO, std::string& errmex ) throw( );

	  log4cpp::Category* getLogger( void ) { return m_log_dev; }

	  glite::ce::cream_client_api::util::ConfigurationManager* getConfMgr( void ) { return m_confMgr; }
	  
	  void set_logfile( const char*, const char*, const char* );
	  
	  std::string get_logfile( void ) const { return m_logfile; }
	  
	  //void activate_logging( void ) { m_logging = true; }
	  //void deactivate_logging( void ) { m_logging = false; }
	  
	  std::string get_execution_error_message( void ) const { return m_execution_fail_message; }
	};
      
      }
      }
      }
      }
      
#endif
