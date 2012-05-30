#ifndef GLITE_CE_CREAM_CLI_ENABLESUB
#define GLITE_CE_CREAM_CLI_ENABLESUB

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_enable_submission : public cli_service {
	  
	  
	public:
	  virtual ~cli_service_enable_submission( ) throw( ) { }
	  cli_service_enable_submission( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  //bool get_result( void ) const { return m_result; }
	  
	};
      
      }
      }
      }
      }

#endif
