#ifndef GLITE_CE_CREAM_CLI_ALLOWEDSUB
#define GLITE_CE_CREAM_CLI_ALLOWEDSUB

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_allowed_submission : public cli_service {
	  
	  bool m_is_allowed;
	  
	public:
	  virtual ~cli_service_allowed_submission( ) throw( ) {}
	  cli_service_allowed_submission( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  bool is_submission_allowed( ) const { return m_is_allowed; }
	  
	};
      
      }
      }
      }
      }

#endif
