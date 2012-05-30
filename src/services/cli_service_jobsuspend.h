#ifndef GLITE_CE_CREAM_CLI_SUSP
#define GLITE_CE_CREAM_CLI_SUSP

#include "cli_service.h"

#include <utility>
#include <string>
#include <map>

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_jobsuspend : public cli_service {
	  
	  std::vector< std::string >				m_jobs_to_suspend;
	  std::string						m_inputfile;
	  bool							m_suspendall;
	  bool							m_joblist_from_file;
	  
	  std::map<std::string, std::string>		       *m_fail_result;
	  
	public:
	  virtual ~cli_service_jobsuspend( ) throw( );
	  cli_service_jobsuspend( const cli_service_common_options&, std::map<std::string, std::string>* );
	  
	  virtual int execute( void ) throw();
	  
	  void insert_job_to_suspend( const std::string& job ) { m_jobs_to_suspend.push_back( job ); }
	  
	  void set_inputfile( const std::string& file ) { m_inputfile = file; m_joblist_from_file = true; }
	  
	  void set_suspend_all( ) { m_suspendall = true; }
	  
	  
	};
      
      }
      }
      }
      }

#endif
