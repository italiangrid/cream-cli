#ifndef GLITE_CE_CREAM_CLI_PRXRENEW
#define GLITE_CE_CREAM_CLI_PRXRENEW

#include "cli_service.h"

#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_proxy_renew : public cli_service {
	  
	  std::string   m_delegation_id;
	  
	public:
	  virtual ~cli_service_proxy_renew( ) throw( ) { }
	  cli_service_proxy_renew( const cli_service_common_options& );
	  virtual int execute( void ) throw();
	  
	  void set_delegation_id( const std::string& D ) { m_delegation_id = D; }
	  
	};
      
      }
      }
      }
      }

#endif
