#include "cli_service_jobstatus.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/EventWrapper.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

#include "util/cliUtils.h"

#include "boost/lexical_cast.hpp"
#include <boost/regex.hpp>

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//______________________________________________________________________________
cli_service_jobstatus::cli_service_jobstatus( const cli_service_common_options& opt, 
					      apiproxy::AbsCreamProxy::StatusArrayResult* status,
					      apiproxy::AbsCreamProxy::InfoArrayResult* info )
  : cli_service( opt ), m_status_result( status ), m_info_result( info ), m_from( -1 ), m_to( -1 ),
  m_jobids_from_file( false ), m_statusall(false ), m_level( 0 ), m_inputfile (""), m_jobids( vector<string>() ),
  m_string_level( "" ), m_states_filter( vector<string>() )
{
}

//______________________________________________________________________________
cli_service_jobstatus::~cli_service_jobstatus( ) throw( )
{
}

//______________________________________________________________________________
int cli_service_jobstatus::execute( void ) throw( )
{
  string VO = "";
  long int i;
  if(!this->checkProxy( VO, i, m_execution_fail_message ) ) {
     return 1;
  }

  if(!this->initConfigurationFile( VO, m_execution_fail_message ) ) {
    return 1;
  }


  

  this->set_logfile( "STATUS_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-job-status" );
  
  if( !m_endpoint.empty() ) {
    if( !cliUtils::checkEndpointFormat( m_endpoint ) )
      {
	m_execution_fail_message = "Endpoint empty or not specified in the right format: should be <host>[:tcpport]. Stop.";
	return(1);
      }
    
    /**
     *
     */
    if(!cliUtils::containsTCPPort( m_endpoint )) {
      m_endpoint = m_endpoint + ":" + this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443");
    }
  }
  
  string serviceAddress = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
    m_endpoint+"/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);
  
//  if ( m_logging && !m_endpoint.empty()) {
    this->getLogger()->debug( "Service address=[%s]", serviceAddress.c_str() );
//  }
  


  if(m_jobids_from_file) {
    bool isjoblist = true;
    int f;
    
//    if(m_logging)
      this->getLogger()->debug( string("Getting job list from file [") + m_inputfile + "]" );
    
    if((f=open(m_inputfile.c_str(), O_RDONLY))==-1) {
      int saveerr = errno;
      m_execution_fail_message = "Error accessing file [" + m_inputfile + "]: " + strerror(saveerr) + ". Stop.";
      return 1;
    }
    close(f);
    
    isjoblist = cliUtils::isACreamJobListFile(m_inputfile.c_str());
    if(!isjoblist) {
      m_execution_fail_message = string("File [") + m_inputfile + "] is not a CREAM job list file. Stop.";
      return 1;
    }
    cliUtils::getJobIDFromFile(m_jobids, m_inputfile.c_str());
  }






  if( m_string_level.empty( ) ) {
  
    m_string_level = this->getConfMgr()->getProperty("STATUS_VERBOSITY_LEVEL", "0" );
  
  }
  
  boost::regex pattern;
  pattern = "^[0-2]\\b";
  boost::cmatch what;
  if( !boost::regex_match( m_string_level.c_str(), what, pattern) ) {
    
    m_execution_fail_message = "Specified output level is wrong: must be a number between 0 and 2. Stop.";
    return 1;
  }
  
  m_level =  boost::lexical_cast<int>( m_string_level );





  
  map <string, vector<string> > url_jobids;

  for(vector<string>::const_iterator jit = m_jobids.begin();
      jit != m_jobids.end();
      jit++) 
    {
      //cout << "jobid=" << *jit << endl;
    
      string os = string("Processing job [") + *jit + "]...";
//      if(m_logging)
	this->getLogger()->debug( os );
      
      if( !m_endpoint.empty() ) {
	string tmp = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
	  m_endpoint+"/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
	
	
	//stripCreamURL stripper( &confMgr);
	url_jobids[tmp].push_back( *jit );
      }
      else {
	vector<string> pieces;
	try { apiutil::CEUrl::parseJobID( *jit , pieces, this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443")); }
	catch(apiutil::CEUrl::ceid_syntax_ex& ex) {
//	  if(m_logging) {
	    this->getLogger()->error( ex.what() );
	    this->getLogger()->error( "This JobID doesn't contain the CREAM service address. Skipping it..." );
//	  }
	  continue;
	} 
	string tmp = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
	  pieces[1]+':'+pieces[2]+
	  "/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
	url_jobids[tmp].push_back( *jit );
      }
    }
  
  

  apiproxy::AbsCreamProxy::StatusArrayResult Sresult;//std::map< std::string, std::pair<bool, StatusResult > >
  apiproxy::AbsCreamProxy::InfoArrayResult Iresult;//std::map< std::string, std::pair<bool, InfoResult > >


 // cout << "STATUSALL=" << m_statusall << " - level=" << m_level << endl;


  if(m_statusall) { // asks for ALL jobs
    
    if(m_level>0) { //invokes the JobInfo (the heavy one)
      std::vector< apiproxy::JobIdWrapper> empty;
      apiproxy::JobFilterWrapper filter(std::vector< apiproxy::JobIdWrapper>(), m_states_filter, m_from, m_to, "", "");
      
      m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyInfo( &filter, m_info_result, m_soap_timeout );
      if(!m_creamClient)
	{
	  m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
	  return 1;
	}

      m_endpoint = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
	m_endpoint + "/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
      
      string result;

      m_creamClient->setCredential( m_certfile );

      try {

//	if(m_logging)
	  this->getLogger()->debug( string("Contacting service [") + m_endpoint + "]");

	m_creamClient->execute( m_endpoint );
	
      } catch(BaseException& ex) {
	
	m_execution_fail_message = ex.what();
	return 1;
	
      } catch(exception& ex) {
	
	m_execution_fail_message = ex.what();
	return 1;
	
      }
      
      
      //if( execute(creamClient, endpoint, result) ) {
	
      //	printInfoResult printInfo( oLevel );
      //for_each(Iresult.begin(), Iresult.end(), printInfo);//( result );
      //}else {
      //	creamApiLogger::instance()->getLogger()->error( result );
      //}
      
    } else { //invokes the JobStatus (the light one)
      
      std::vector< apiproxy::JobIdWrapper> empty;
      apiproxy::JobFilterWrapper filter(std::vector< apiproxy::JobIdWrapper>(), m_states_filter, m_from, m_to, "", "");
      m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyStatus( &filter, m_status_result, m_soap_timeout );

      if(!m_creamClient)
	{
	  m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
	  return 1;
	}
      
      m_creamClient->setCredential( m_certfile );
      
      m_endpoint = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)
	+ m_endpoint + "/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);


      try {
	
//	if(m_logging)
	  this->getLogger()->debug( string("Contacting service [") + m_endpoint + "]");

	m_creamClient->execute( m_endpoint );
	
      } catch(BaseException& ex) {
	
	m_execution_fail_message = ex.what();
	return 1;
	
      } catch(exception& ex) {
	
	m_execution_fail_message = ex.what();
	return 1;
	
      } 


    }
    
    /*******************************************
     * 
     * ASKS  THE  STATUS  OF  SELECTED  JOBS
     *
     ******************************************/
  } else { // asks for selected jobs
    
    if(m_level>0) { //invokes the JobInfo (the full one)
      for(map<string, vector<string> >::iterator it = url_jobids.begin();
	  it != url_jobids.end();
	  it++) // iterates over endpoints
	{
	  vector<apiproxy::JobIdWrapper> target;
	  
	  //cout << "it_first=[" << it->first << endl;
	  
	  stripCreamURL strip( &target, this->getConfMgr( ) );
	  for_each(it->second.begin(), it->second.end(), strip);
	  apiproxy::JobFilterWrapper filter(target, m_states_filter, m_from, m_to, "", "");
	  string url = it->first;
	  
	  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyInfo( &filter, m_info_result, m_soap_timeout );
	  if(!m_creamClient)
	    {
	      m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
	      return 1;
	    }

	  m_creamClient->setCredential( m_certfile );
	  string result;



	  try {
	    
//	    if(m_logging)
	      this->getLogger()->debug( string("Contacting service [") + url + "]");
	    
	    m_creamClient->execute( url );
	    
	  } catch(BaseException& ex) {
	    
	    m_execution_fail_message = ex.what();
	    return 1;
	    
	  } catch(exception& ex) {
	    
	    m_execution_fail_message = ex.what();
	    return 1;
	    
	  } 


	}
	
      
    } else {//invokes the JobStatus (the light one)
      
      for(map<string, vector<string> >::iterator it = url_jobids.begin();
	  it != url_jobids.end();
	  it++) // iterates over endpoints
	{
//	  if ( /*log_dev->isInfoEnabled()*/ m_logging ) {
//
	    this->getLogger()->info( string("Sending JobStatus request to [")+(*it).first+"]" );
//	  }	  
	  vector<apiproxy::JobIdWrapper> target;
	  
	  stripCreamURL strip( &target, this->getConfMgr( ) );
	  for_each(it->second.begin(), it->second.end(), strip);	  
	  apiproxy::JobFilterWrapper filter(target, m_states_filter, m_from, m_to, "", "");	  
	  string url = it->first;
	  
	  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyStatus( &filter, m_status_result, m_soap_timeout );
	  if(!m_creamClient)
	    {
	      m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
	      return 1;
	    }

	  m_creamClient->setCredential( m_certfile );



	  try {
	    
//	    if(m_logging)
	      this->getLogger()->debug( string("Contacting service [") + url + "]");
	    
	    m_creamClient->execute( url );
	    
	  } catch(BaseException& ex) {
	    
	    m_execution_fail_message = ex.what();
	    return 1;
	    
	  } catch(exception& ex) {
	    
	    m_execution_fail_message = ex.what();
	    return 1;
	    
	  } 
	  


	}
      
    }
    
    //return 0;
  }
  
  return 0;
}
