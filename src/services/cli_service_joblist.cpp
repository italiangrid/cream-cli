#include "cli_service_joblist.h"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"

//#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"

#include "util/cliUtils.h"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;

//______________________________________________________________________________
cli_service_joblist::cli_service_joblist( const cli_service_common_options& opt )
 : cli_service( opt )
{
  m_jobid_on_file = false;
  m_outputfile = "";
}

//______________________________________________________________________________
void cli_service_joblist::set_output( const string& outputfile ) 
{
  m_jobid_on_file = true;
  m_outputfile    = outputfile;
}

//______________________________________________________________________________
int cli_service_joblist::execute( ) throw( ) {

  string VO = "", errmex = "";
  long int i;
  if(!this->checkProxy( VO, i, errmex ) ) {
    m_execution_fail_message = errmex;
    return 1;
  }
  
  if(!this->initConfigurationFile( VO, errmex ) ) {
    m_execution_fail_message = errmex;
    return 1;
  }
  
  this->set_logfile( "LIST_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-job-list" );
  

  if( !cliUtils::checkEndpointFormat( m_endpoint ) )
    {
      m_execution_fail_message = "Endpoint not specified in the right format: should be <host>[:tcpport]. Stop.";
      return(1);
    }

  /**
   *
   */
  if(!cliUtils::containsTCPPort( m_endpoint )) {
    m_endpoint = m_endpoint + ":" + this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT", "8443");
  }

  string serviceAddress = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
    m_endpoint+"/" + this->getConfMgr()->getProperty("CREAM_URL_POSTFIX",DEFAULT_CEURL_POSTFIX);

//  if ( m_logging )
    this->getLogger()->debug( "Service address=[%s]", serviceAddress.c_str() );
 
  
  vector<apiproxy::JobIdWrapper> jobIdWrapperVetor;
  
  m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyList(&jobIdWrapperVetor, m_soap_timeout );
  
  if(!m_creamClient)
    {
      m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy OBJECT! STOP!";
      return 1;
    }

  m_creamClient->setCredential( m_certfile );
  
  try {

     m_creamClient->execute(serviceAddress);
     
   } catch ( BaseException& ex) {
     m_execution_fail_message = ex.what();
     return 1;
   } catch (exception& ex) {
     m_execution_fail_message = ex.what();
     return 1;
   }


   string prefixJobId = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+ m_endpoint + "/";

   if(m_jobid_on_file) {
     bool exists = false;

     if((exists=cliUtils::fileExists(m_outputfile.c_str()))==true) {
	if(!cliUtils::fileIsWritable(m_outputfile.c_str())){
	  m_execution_fail_message = string("Output file [") + m_outputfile + "] exists but is not accessible. Stop";
	  return(1);
	}
     }
     // OK, the output file does exist or doesn't
     // if it does let's check if the first line has the magic string
     int f;
   
     if(exists) {
        bool isjoblist = cliUtils::isACreamJobListFile(m_outputfile.c_str());
	if(!isjoblist) {
	
	if(!m_noint) {
	   printf("\nWARNING: file [%s] already exists and is not a CREAM job list file. Overwrite it (y/n) ? ", m_outputfile.c_str());
	   char c; cin >> c;
	   if(c!='y') {
	     printf("\nListing aborted. Bye!\n"); return(0);
	   }
         }
	 // Is NOT a JobList => opens file and truncs it
	 try { f=cliUtils::openJobListFile(m_outputfile.c_str(), true); }
	 catch(apiutil::file_ex& fex) {
	     m_execution_fail_message = fex.what();
             return(1);
	 }
        } else {  // if(!isjoblist)
	  // opens file in append mode
	  try { f=cliUtils::appendToJobListFile(m_outputfile.c_str()); }
	  catch(apiutil::file_ex& fex) {
	    m_execution_fail_message = fex.what();
	    return(1);
	  }
	}
     } // if(exists)
     else { //if(exist)
     	// create file from scratch
	try { f=cliUtils::openJobListFile(m_outputfile.c_str(), false); }
	catch(apiutil::file_ex& fex) {
	    m_execution_fail_message = fex.what();
	    return(1);
	}
     }
     // Writes JobIDs into file; but before doing that
     // retrieves the list of jobs already in the file to prevent
     // duplications
     vector<string> alreadyThere;
     alreadyThere.reserve(1000);
     cliUtils::getJobIDFromFile(alreadyThere, m_outputfile.c_str());
     int count = 0;
     for(vector<apiproxy::JobIdWrapper>::const_iterator rit = jobIdWrapperVetor.begin();
         rit != jobIdWrapperVetor.end();
	 rit++) {
       
       string creamJobId = (*rit).getCreamURL();
       string::size_type loc = creamJobId.find(this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX));
       if( loc != string::npos) {
         creamJobId = creamJobId.substr(0,loc);
        }
       creamJobId += "/";
       creamJobId.append((*rit).getCreamJobID());
       if( find(alreadyThere.begin(), alreadyThere.end(), creamJobId ) != alreadyThere.end()){
           continue;
        }
	try { cliUtils::writeJobID(f, creamJobId ); }
	catch(apiutil::file_ex& fex) {
	  m_execution_fail_message = fex.what();
	  close(f);
	  return(1);
	}
	count++;
     }
  } else {
    for(vector<apiproxy::JobIdWrapper>::const_iterator rit = jobIdWrapperVetor.begin();
        rit != jobIdWrapperVetor.end();
	rit++) { //unsigned int j=0; j<result.size(); j++) {
        string creamJobId = (*rit).getCreamURL();
        string::size_type loc = creamJobId.find(this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX));
        if( loc != string::npos) {
          creamJobId = creamJobId.substr(0,loc);
        }
	creamJobId += "/";
        creamJobId.append((*rit).getCreamJobID());  
// 	if(m_logging)
//           cout << creamJobId << endl;
	m_joblist.push_back( creamJobId );

    }
   }
   return 0;
}
