#ifndef GLITE_CE_CREAM_CLI_LEASE
#define GLITE_CE_CREAM_CLI_LEASE

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_joblease : public cli_service {
	  
	  std::string   m_lease_id;
	  time_t        m_lease_time;
	  time_t        m_negotiated_lease_time;
	  
	public:
	  virtual ~cli_service_joblease( ) throw( ) { }
	  cli_service_joblease( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  void set_lease( const std::string& ID, const time_t T ) { m_lease_id = ID; m_lease_time = T; }
	  
	  time_t get_negotiated_lease_time( void ) const { return m_negotiated_lease_time; }
	};
      
      }
      }
      }
      }

#endif
