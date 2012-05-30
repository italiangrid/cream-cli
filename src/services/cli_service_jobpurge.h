#ifndef GLITE_CE_CREAM_CLI_PURGE
#define GLITE_CE_CREAM_CLI_PURGE

#include "cli_service.h"

#include <utility>
#include <string>
#include <map>

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_jobpurge : public cli_service {
	  
	  std::vector< std::string >				m_jobs_to_purge;
	  std::string						m_inputfile;
	  bool							m_purgeall;
	  bool							m_joblist_from_file;
	  
	  std::map<std::string, std::string>		       *m_fail_result;
	  
	public:
	  virtual ~cli_service_jobpurge( ) throw( );
	  cli_service_jobpurge( const cli_service_common_options&, std::map<std::string, std::string>* );
	  
	  virtual int execute( void ) throw();
	  
	  void insert_job_to_purge( const std::string& job ) { m_jobs_to_purge.push_back( job ); }
	  
	  void set_inputfile( const std::string& file ) { m_inputfile = file; m_joblist_from_file = true; }
	  
	  void set_purge_all( ) { m_purgeall = true; }
	  
	  
	};
      
      }
      }
      }
      }

#endif
