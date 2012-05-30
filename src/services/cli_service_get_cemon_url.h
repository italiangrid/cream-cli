#ifndef GLITE_CE_CREAM_CLI_GETCEMONURL
#define GLITE_CE_CREAM_CLI_GETCEMONURL

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_get_cemon_url : public cli_service {
	  
	  std::string m_url;
	  
	public:
	  virtual ~cli_service_get_cemon_url( ) throw( ) { }
	  cli_service_get_cemon_url( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  std::string get_url( void ) const { return m_url; }
	};
      
      }
      }
      }
      }

#endif
