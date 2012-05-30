#ifndef GLITE_CE_CREAM_CLI_JOBLIST
#define GLITE_CE_CREAM_CLI_JOBLIST

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_joblist : public cli_service {
	  
	  bool 				m_jobid_on_file;
	  std::string			m_outputfile;
	  std::vector<std::string>	m_joblist;
	  
	public:
	  virtual ~cli_service_joblist( ) throw( ) {}
	  cli_service_joblist( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  void set_output( const std::string& );
	  
	  void get_job_ids( std::vector<std::string>& target ) { target = m_joblist; }
	};
      
      }
      }
      }
      }

#endif
