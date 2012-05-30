#ifndef GLITE_CE_CREAM_CLI_DISABLESUB
#define GLITE_CE_CREAM_CLI_DISABLESUB

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_disable_submission : public cli_service {
	  
	public:
	  virtual ~cli_service_disable_submission( ) throw( ) { }
	  cli_service_disable_submission( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  //bool get_result( void ) const { return m_result; }
	  
	};
      
      }
      }
      }
      }

#endif
