#ifndef GLITE_CE_CREAM_CLI_SERVICE_COMMON
#define GLITE_CE_CREAM_CLI_SERVICE_COMMON

namespace glite {
  namespace ce {
    namespace cream_cli {
      namespace services {

struct cli_service_common_options {
	  bool			m_debug;
	  bool			m_nomsg;
	  bool			m_noint;
	  bool			m_verify_ac_sign;
	  bool			m_redir_out;
	  std::string		m_user_conf_file;
	  std::string		m_certfile;
	  std::string		m_logfile;
	  std::string		m_endpoint;
	  int			m_soap_timeout;
	  
	  //log4cpp::Category* 	m_log_dev;
  };
}
}
}
}
#endif
