#include "cli_service_jobcancel.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/EventWrapper.h"
#include "glite/ce/cream-client-api-c/job_statuses.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

#include "util/cliUtils.h"

#include "boost/lexical_cast.hpp"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//______________________________________________________________________________
cli_service_jobcancel::cli_service_jobcancel( const cli_service_common_options& opt, map<string, string>* target )
 : cli_service( opt ), m_jobs_to_cancel( vector<string>() ), m_inputfile( "" ), m_cancelall( false ), m_fail_result( target ),
 m_joblist_from_file( false )
{
}

//______________________________________________________________________________
cli_service_jobcancel::~cli_service_jobcancel( ) throw( )
{
}

//______________________________________________________________________________
int cli_service_jobcancel::execute( void ) throw( )
{
  string VO = "";
  long int i;
  if(!this->checkProxy( VO, i, m_execution_fail_message ) ) {
     return 1;
  }

  if(!this->initConfigurationFile( VO, m_execution_fail_message ) ) {
    return 1;
  }

  this->set_logfile( "CANCEL_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-job-cancel" );
  

  if( !m_endpoint.empty( ) ) {
    if( !cliUtils::checkEndpointFormat( m_endpoint ) )
    {
      m_execution_fail_message = "Endpoint empty or not specified in the right format: should be <host>[:tcpport]. Stop.";
      return(1);
    }
    try { 
      if(!cliUtils::containsTCPPort(m_endpoint)) {
        m_endpoint = m_endpoint + ":" + this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443");
      }
    } catch(internal_ex& ex) {
      m_execution_fail_message = ex.what();
      return(1);
    }
  } // endpoint empty

  
  
  

  string serviceAddress = "";
  unsigned int jcounter = 0;
  vector<string> jobnumbers_to_cancel;
  bool cancel_all_jobs_from_file = false;
  
  if(m_joblist_from_file) {
    
    if(!cliUtils::interactiveChoice("Cancel", 
    				    m_inputfile.c_str(), 
				    m_noint, 
				    m_debug, 
				    m_nomsg, 
				    cancel_all_jobs_from_file, 
				    jobnumbers_to_cancel,
				    m_jobs_to_cancel, 
				    m_execution_fail_message) ) 
    {
      return 1;
    }


  }

  vector<string> all_jobids_to_actually_cancel;
  map <string, vector<string> > url_jobids;
  
  if(!m_cancelall) {
    char confirm;
    if(!m_noint) {
      printf("\nAre you sure you want to cancel specified job(s) [y/n]: ");
      cin >> confirm;
    } else confirm ='y';

    if(confirm != 'y') {
      printf("Cancel aborted. Bye.\n");
      m_execution_fail_message="";
      return 1;
    }

    /**
     *
     * Filters out the job IDs the user didn't select
     *
     */
    for(jcounter=0; jcounter<m_jobs_to_cancel.size(); jcounter++) {
      if(m_joblist_from_file) {
        if(!cancel_all_jobs_from_file) {
	  bool isthere = false;
	  for(
	      vector<string>::iterator it = jobnumbers_to_cancel.begin();
	      it!=jobnumbers_to_cancel.end();
	      it++)
	    if( jcounter == (unsigned int)atoi( (*it).c_str()) ) { isthere = true; break; }
	  if(!isthere) continue;
        }
//	if(m_logging)
	  this->getLogger()->debug( string("Will cancel job [")+m_jobs_to_cancel.at(jcounter)+"]" );
	  
	all_jobids_to_actually_cancel.push_back(m_jobs_to_cancel.at(jcounter));
      }
      
      else all_jobids_to_actually_cancel.push_back( m_jobs_to_cancel.at(jcounter) );
    
    }
    /**
     *
     * Create an hash map CEId -> JobIDs
     *
     */
    vector<string> pieces;
    pieces.reserve(10);

    for(vector<string>::const_iterator jit = all_jobids_to_actually_cancel.begin();
	jit != all_jobids_to_actually_cancel.end();
	jit++) { //unsigned int j=0; j<all_jobids_to_actually_cancel.size(); j++) {
      if( !m_endpoint.empty() ) {
	string tmp = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
	  m_endpoint+"/"+this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
        url_jobids[tmp].push_back( *jit );
      }
      else {
        pieces.clear();
	
	try { apiutil::CEUrl::parseJobID( *jit, pieces, this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443")); }
        catch(apiutil::CEUrl::ceid_syntax_ex& ex) {
//	  if(m_logging) {
            this->getLogger()->error( ex.what() );
            this->getLogger()->error( "This JobID doesn't contain the CREAM service address. Skipping it..." );
//	  }
	  continue;
        }
	string tmp = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
	  pieces[1]+":"+pieces[2]+"/"+
	  this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
	url_jobids[tmp].push_back( *jit );
      }
    }
  } else {
    serviceAddress = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO);
    serviceAddress = serviceAddress + m_endpoint + "/" 
      + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
  }
  
  try {
    
    if(m_cancelall && !m_noint) {
      char confirm;
      printf("\nGoing to cancel all job from CE [%s]. Do you want to proceed [y/n]? ",
	     serviceAddress.c_str());
      cin >> confirm;
      if(confirm!='y') {
        printf("Cancel aborted. Bye.\n\n");
	m_execution_fail_message = "";
        return 1;
      }
    }
    
    apiproxy::JobFilterWrapper *wr = NULL;
    apiproxy::ResultWrapper result;

    if(m_cancelall) {
//      if(m_logging)
        this->getLogger()->info( string("Cancel all jobs on [")+serviceAddress+"]..." );
	
      wr = new apiproxy::JobFilterWrapper(vector<apiproxy::JobIdWrapper>(), vector<string>(), -1, -1, "", "");
      
      m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyCancel( wr, &result, m_soap_timeout );
      
      if(!m_creamClient)
	{
	  m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
	  return 1;
	}
	
     try {
    m_creamClient->setCredential( m_certfile );
  } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    //this->getLogger()->fatal( ex.what( ) );
    m_execution_fail_message = ex.what();
    return 1;
  }
      m_creamClient->execute( serviceAddress );
      
      //cliUtils::printResult( result );
      
      cliUtils::processResult( result, *m_fail_result );
      
      //creamClient->garbage_collect();
      
      delete wr;
    }
    else {
      for(map<string, vector<string> >::iterator it=url_jobids.begin(); it!=url_jobids.end(); it++) {
//	if(m_logging)
	  this->getLogger()->debug( string("Cancel selected jobs on [")+(*it).first+"]..." );
	  
	vector<apiproxy::JobIdWrapper> target;
	stripCreamURL strip( &target, this->getConfMgr() );
	for_each(it->second.begin(), it->second.end(), strip);
	wr = new apiproxy::JobFilterWrapper(target, vector<string>(), -1, -1, "", "");       
	m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyCancel( wr, &result, m_soap_timeout );
	if(!m_creamClient)
	  {
	    m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy object! STOP!";
	    return 1;
	  }
	  
	//boost::scoped_ptr< apiproxy::AbsCreamProxy > tmpClient;
	//tmpClient.reset( creamClient );
	
     try {
    m_creamClient->setCredential( m_certfile );
  } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    //this->getLogger()->fatal( ex.what( ) );
    m_execution_fail_message = ex.what();
    return 1;
  }
	m_creamClient->execute( (*it).first.c_str() );
	
	//cliUtils::printResult( result );
	
	cliUtils::processResult( result, *m_fail_result );
	
	//creamClient->garbage_collect();
	delete wr;
      }
    }
   
  } catch(BaseException& s) {
    m_execution_fail_message = s.what();
    return 1;
  } catch(exception& s) {
    m_execution_fail_message = s.what();
    return 1;
  } 
  
  
  
  return 0;
}
