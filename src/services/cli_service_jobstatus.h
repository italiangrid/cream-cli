#ifndef GLITE_CE_CREAM_CLI_STATUS
#define GLITE_CE_CREAM_CLI_STATUS

#include "cli_service.h"
#include "glite/ce/cream-client-api-c/JobStatusWrapper.h"
#include "glite/ce/cream-client-api-c/JobInfoWrapper.h"
//#include "glite/ce/cream-client-api-c/job_statuses.h"

#include <string>
#include <vector>

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace api      = glite::ce::cream_client_api;

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_jobstatus : public cli_service {
	  
	  apiproxy::AbsCreamProxy::StatusArrayResult  *m_status_result;
	  apiproxy::AbsCreamProxy::InfoArrayResult    *m_info_result;
	  std::vector<std::string>                     m_states_filter;

	  int                                          m_level;
	  std::string  				       m_string_level;

	  bool                                         m_statusall;

	  std::vector<std::string>                     m_jobids;

	  int                                          m_from;
	  int                                          m_to;

	  std::string                                  m_inputfile;

	  bool                                         m_jobids_from_file;

	public:
	  virtual ~cli_service_jobstatus( ) throw( );
	  cli_service_jobstatus( const cli_service_common_options&, 
				 apiproxy::AbsCreamProxy::StatusArrayResult* status,
				 apiproxy::AbsCreamProxy::InfoArrayResult* info );

	  virtual int execute( void ) throw();
	  
          void insert_job_to_query( const std::string& job ) { m_jobids.push_back( job ); }
          
          void set_inputfile( const std::string& file ) { m_inputfile = file; m_jobids_from_file = true; }
          
          void set_status_all( ) { m_statusall = true; }

	  void set_verbosity_level( const std::string& level ) { m_string_level = level; }

	  void set_timerange( const time_t from, const time_t to ) { m_from = from; m_to = to; }

	  void set_state_filter( const std::vector<std::string>& states ) { m_states_filter = states; }
	  
	  std::string get_verbosity_level( void ) const { return m_string_level; }
	};
      
      }
      }
      }
      }

#endif
