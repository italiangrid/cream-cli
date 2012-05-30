#ifndef GLITE_CE_CREAM_CLI_SERVICEINFO
#define GLITE_CE_CREAM_CLI_SERVICEINFO

#include "cli_service.h"

#include "glite/ce/cream-client-api-c/ServiceInfoWrapper.h"

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_service_info : public cli_service {
	  
	  apiproxy::ServiceInfoWrapper	*m_service_info;
	  
	  int				 m_verbosity;
	  
	public:
	  virtual ~cli_service_service_info( ) throw( ) {}
	  cli_service_service_info( const cli_service_common_options&, apiproxy::ServiceInfoWrapper *info );
	  virtual int execute( void ) throw();
	  
	  //apiproxy::ServiceInfoWrapper* get_info( void ) { return &m_service_info; }
	  
	  void set_verbosity( const int verb ) { m_verbosity = verb; }
	  
	  
	};
      
      }
      }
      }
      }

#endif
