#ifndef GLITE_CE_CREAM_CLI_CANCEL
#define GLITE_CE_CREAM_CLI_CANCEL

#include "cli_service.h"

#include <utility>
#include <string>
#include <map>

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_jobcancel : public cli_service {
	  
	  std::vector< std::string >				m_jobs_to_cancel;
	  std::string						m_inputfile;
	  bool							m_cancelall;
	  bool							m_joblist_from_file;
	  
	  std::map<std::string, std::string>		       *m_fail_result;
	  
	public:
	  virtual ~cli_service_jobcancel( ) throw( );
	  cli_service_jobcancel( const cli_service_common_options&, std::map<std::string, std::string>* );
	  
	  virtual int execute( void ) throw();
	  
	  void insert_job_to_cancel( const std::string& job ) { m_jobs_to_cancel.push_back( job ); }
	  
	  void set_inputfile( const std::string& file ) { m_inputfile = file; m_joblist_from_file = true; }
	  
	  void set_cancel_all( ) { m_cancelall = true; }
	  
	  
	};
      
      }
      }
      }
      }

#endif
