#ifndef GLITE_CE_CREAM_CLI_JOBSUBMIT
#define GLITE_CE_CREAM_CLI_JOBSUBMIT

#include "cli_service.h"
//#include "glite/ce/cream-client-api-c/EventWrapper.h"

#include <string>

namespace apiproxy = glite::ce::cream_client_api::soap_proxy;

class jdlHelper;

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {
      
        class cli_service_jobsubmit : public cli_service {
          

	  
	  bool                  m_fake;
	  std::string           m_ceid;
	  std::string           m_outputfile;
	  bool                  m_automatic_delegation;
	  std::string           m_delegation_id;
	  std::string           m_VO;
	  std::string           m_user_specified_VO;
	  int                   m_ftp_num_streams;
	  std::string           m_lease_id;
	  std::list<std::string> m_jdlfiles;
	  bool                  m_jobid_on_file;
	  
	  std::vector<std::string>           m_jobids;

	  std::vector<jdlHelper*> m_helpers;

	  bool make_delegation( const std::string& deleg_service ) throw();

	  //void freeHelpers( void );
	  //void freeRequests( std::list<apiproxy::JobDescriptionWrapper*>& );

	  std::vector<jdlHelper*> process_ISB( const std::vector<jdlHelper*> source );
	  std::vector<jdlHelper*> process_ISB2( const std::vector<jdlHelper*> source );
	  std::vector<jdlHelper*> process_JDL( const std::vector<jdlHelper*>& source, bool, const std::vector<std::string>& );
	  std::vector<jdlHelper*> check_VO_JDL( const std::vector<jdlHelper*>& source );

        public:
          virtual ~cli_service_jobsubmit( ) throw( ) {}
          cli_service_jobsubmit( const cli_service_common_options& );
          virtual int execute( void ) throw();
          
          
	  void set_user_specified_vo( const std::string& vo ) { m_user_specified_VO = vo; }

	  void add_jdl_file( const std::string& jdlfile ) { m_jdlfiles.push_back( jdlfile ); }

	  void set_delegation( const bool automatic, 
			       const std::string& delid ) 
	  { 
	    m_automatic_delegation = automatic; 
	    m_delegation_id = delid; 
	  }

	  void set_lease_id( const std::string& leaseid ) { m_lease_id = leaseid; }
	  
	  void set_outputfile( const std::string& outfile ) { m_jobid_on_file = true; m_outputfile = outfile; }
	  
	  void set_ceid( const std::string& ceid ) { m_ceid = ceid; }
	  
	  std::vector<std::string> get_jobids( void ) const  { return m_jobids; }
	  
	  void set_fake_run( void ) { m_fake = true; }

        };
      
      }
      }
      }
      }

#endif
