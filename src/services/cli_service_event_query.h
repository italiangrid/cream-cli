#ifndef GLITE_CE_CREAM_CLI_EVTQRY
#define GLITE_CE_CREAM_CLI_EVTQRY

#include "cli_service.h"
#include "glite/ce/cream-client-api-c/EventWrapper.h"

#include <string>

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_event_query : public cli_service {
	  
	  int 							m_nevents;
	  std::vector<std::pair<std::string, std::string> >	m_status;
	  std::string						m_evtStart;
	  std::string						m_evtEnd;
	  
	  std::list<apiproxy::EventWrapper*>                   *m_result;
	  
	public:
	  virtual ~cli_service_event_query( ) throw( );// {}
	  cli_service_event_query( const cli_service_common_options&, std::list<apiproxy::EventWrapper*>* );
	  virtual int execute( void ) throw();
	  
	  void set_events( const std::string&, const std::string& );
	  void set_num_events( const int );
	  void set_status( const std::vector<std::pair<std::string, std::string> >& );
	  
	  //std::list<apiproxy::EventWrapper*> *get_events( void ) { return &m_result; }
	};
      
      }
      }
      }
      }

#endif
