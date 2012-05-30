#ifndef GLITE_CE_CREAM_CLI_ESJOBSUBMIT
#define GLITE_CE_CREAM_CLI_ESJOBSUBMIT

#include "cli_service.h"



#include <string>

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_esjobsubmit : public cli_service {

	  //	  bool make_delegation( const std::string& deleg_service ) throw();
	  
	  std::string           m_ceid;
	  std::string           m_xmlfile;
	  bool                  m_automatic_delegation;
	  std::string           m_delegation_id;
	  time_t                m_expiration_time;

	  bool make_delegation( const std::string& );

	public:
          virtual ~cli_service_esjobsubmit( ) throw( ) {}
          cli_service_esjobsubmit( const cli_service_common_options& );
          virtual int execute( void ) throw();
	  void set_ceid( const std::string& ceid ) { m_ceid = ceid; }
	  void set_xml_file( const std::string& xml ) { m_xmlfile = xml; }
	  void set_delegation( const bool _auto, const std::string& ID )
	  {
	    m_automatic_delegation = _auto;
	    m_delegation_id        = ID;
	  }

	};

      }
    }
  }
}

#endif
