#include "cli_service_jobsubmit.h"

#include "util/cliUtils.h"
#include "util/jdlHelper.h"
#include "ftpclient/gridftpClient.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/format.hpp"
#include "boost/regex.hpp"

#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/JobDescriptionWrapper.h"

#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/soap_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

using namespace std;
using namespace glite::ce::cream_cli::services;
namespace apiproxy = glite::ce::cream_client_api::soap_proxy;
namespace apiutil = glite::ce::cream_client_api::util;
namespace fs = boost::filesystem;

//______________________________________________________________________________
class helper_cleaner {
  vector<jdlHelper*> m_helpers;

public:
  helper_cleaner( vector<jdlHelper*> helpers) : m_helpers( helpers ) {}
  ~helper_cleaner( ) throw() {
    for(vector<jdlHelper*>::iterator it = m_helpers.begin();
	it != m_helpers.end();
	++it)
      {
	delete *it;
      }
  }
};


//______________________________________________________________________________
class request_cleaner {
  list<apiproxy::JobDescriptionWrapper*> m_reqs;

public:
  request_cleaner( list<apiproxy::JobDescriptionWrapper*> reqs) 
    : m_reqs( reqs ) {}

  ~request_cleaner( ) throw() {
    for(list<apiproxy::JobDescriptionWrapper*>::iterator it = m_reqs.begin();
	it != m_reqs.end();
	++it)
      {
	delete *it;
      }
  }
};


//______________________________________________________________________________
vector<jdlHelper*>
cli_service_jobsubmit::process_ISB( const vector<jdlHelper*> source )
{
  vector<jdlHelper*> target;
  vector<jdlHelper*>::const_iterator it;

  for( it = source.begin( ); it != source.end( ); ++it ) {
    
    try {
      (*it)->processISB( fs::current_path( ).string( )/*, isbbu */);
    } catch(exception& ex) {

      this->getLogger()->warn( string("ERROR processing JDL [")
			       + (*it)->getFileName() + "]: " + ex.what()
			       + ". Will NOT submit this JDL.");
      continue;
    }
    target.push_back( *it );
  }
  return target;
}

//______________________________________________________________________________
vector<jdlHelper*>
cli_service_jobsubmit::process_ISB2( const vector<jdlHelper*> source )
{
  vector<jdlHelper*> target;
  vector<jdlHelper*>::const_iterator it;

  for( it = source.begin( ); it != source.end( ); ++it ) {
    
    set<string> paths;
    (*it)->getAbsolutePath( paths );
    set<string>::const_iterator path_it = paths.begin();
    
    bool good = true;
    for( ; path_it != paths.end(); ++path_it)
      {
	ifstream my_file( path_it->c_str() );
	if (!my_file.good())
	  {
	    this->getLogger()->warn( string("ERROR in JDL: ")
				     + (*it)->getFileName()
				     + ": the file ["
				     + *path_it + "] is not there or is not accessible. "
				     + "Will NOT submit this JDL.");
	    good = false;
	    break;                                                                                                   
	  }
      }

    if(good==true) {
      target.push_back(*it);
    }
  }

  return target;
}

//______________________________________________________________________________
vector<jdlHelper*>
cli_service_jobsubmit::process_JDL( const vector<jdlHelper*>& source, bool mustcheckVO_in_JDL, const  vector<string>& ceid_pieces )
{                                                                                                                                           
  vector<jdlHelper*>::const_iterator it;                                                                                                      
  vector<jdlHelper*> target;

  for( it = source.begin( ); it != source.end( ); ++it ) {                                                          
    
    try {
      if(mustcheckVO_in_JDL) // we already checked the JDL contains the VO                                                                    
	(*it)->process(ceid_pieces[3], ceid_pieces[2], "");                                                                                   
      else // we have to add the user/proxy VO to JDL overwriting it if exists                                                                
	(*it)->process(ceid_pieces[3], ceid_pieces[2], m_VO);                                                                                 
    } catch(JDLEx& ex) {                                                                                                                      
      this->getLogger()->warn( string("ERROR processing JDL [")                                                                               
			       + (*it)->getFileName() + "]: " + ex.what());                                                                   
      continue;                                                                                                                               
    } catch(exception& ex) {                                                                                                                  
      this->getLogger()->warn( string("ERROR processing JDL [")                                                                               
			       + (*it)->getFileName() + "]: " + ex.what());                                                                   
      continue;                                                                                                                               
    }                                                                                                                                         
    target.push_back( *it );                                                                                                  
  }                                                                                                                                           
  return target;
} 

//______________________________________________________________________________
vector<jdlHelper*> 
cli_service_jobsubmit::check_VO_JDL( const vector<jdlHelper*>& source ) 
{
  vector<jdlHelper*> target;
  
  vector<jdlHelper*>::const_iterator it;                                                                                                      
  
  for( it = source.begin(); it != source.end(); ++it ) {                                                                                
    if(!(*it)->hasVO()) {                                                                                                                     
      this->getLogger()->warn( string("VirtualOrganisation not specified in the JDL file [")                                                  
			       + (*it)->getFileName() + "]. Will not submit it.");                                                            
      continue;                                                                                                                               
    }                                                                                                                                         
    target.push_back( *it );                                                                                                     
  }
  return target;
}

//______________________________________________________________________________
bool goodJDL( const string& file ) {

  ifstream myfile ( file.c_str() );

  string line(""), buf("");

  if (myfile.is_open())
  {
    while ( myfile.good() )
    {
      getline (myfile,line);
      
      boost::trim( line );
      boost::trim_left_if( line, boost::is_any_of("\t") );
      //boost::trim_left_if( line, boost::is_any_of("#") );
      
      if(boost::starts_with( line, "#"))
        continue;
	
      if(boost::starts_with( line, "//" ))
        continue;
      
      buf+=line;
    }
    myfile.close();

  }
  
  classad::ClassAdParser parser;
  classad::ClassAd *jdlAd = parser.ParseClassAd( buf );
  
  if ( 0 == jdlAd ) 
    return false;
  
  delete jdlAd;
  return true;
}

//______________________________________________________________________________
cli_service_jobsubmit::cli_service_jobsubmit( const cli_service_common_options& opt )
  : cli_service( opt ), m_fake(false), m_ceid(""),m_outputfile(""),
    m_automatic_delegation(false), m_delegation_id(""),m_VO(""),
    m_user_specified_VO( "" ), m_ftp_num_streams( 2 ),
    m_lease_id(""), m_jdlfiles( list<string>() ), m_jobid_on_file( false ), 
    m_jobids( vector<string>()),
    m_helpers( vector<jdlHelper*>() )
{
}

//______________________________________________________________________________
int cli_service_jobsubmit::execute( void ) throw( )
{
  
  list<string>::const_iterator it = m_jdlfiles.begin();
  
  for( ; it != m_jdlfiles.end(); ++it ) {

    if(!boost::filesystem::exists( boost::filesystem::path(*it, boost::filesystem::native) ) ) {
      this->getLogger()->error( "JDL file [" + *it + "] does not exist. Skipping..." );
      continue;
    }

    if(!goodJDL( *it )) {
      this->getLogger()->error( "Error while processing file ["
                                + *it + "]: Syntax error"
                                + ". Will not submit this JDL" );
      continue;
    }

    jdlHelper *jdlH = 0;
    try {
      jdlH = new jdlHelper( it->c_str( ) );
     } catch(exception& ex ) {

       this->getLogger()->error( "Error while processing file ["
                                + *it + "]: "+ex.what()
                                + ". Will not submit this JDL" );
       continue;

     } catch(...) {
 	this->getLogger()->error( "Error while processing file ["
                                + *it + "]: Unknown exception"
                                + ". Will not submit this JDL" );
       continue;
     }


    m_helpers.push_back( jdlH );
  }

  if(m_helpers.empty() ) return 0;
  
  /**
   *
   * As the scope quits the execute method, all the helpers are deleted
   * automaticalli by the hcleaner's DTOR
   *
   */
  helper_cleaner hcleaner( m_helpers ); 
  

  string VO_from_cert = "";
  
  /**
   *
   * Try to get VO name from certificate
   *
   */
  long int i;
  if(!this->checkProxy( VO_from_cert, i, m_execution_fail_message ) ) {
    //this->freeHelpers();
    return 1;
  }
  bool mustcheckVO_in_JDL = false;
  if ( !VO_from_cert.empty( ) ) {
    m_VO = VO_from_cert;
//    if(m_logging)
      this->getLogger()->debug( string("VO from certificate=[") + VO_from_cert + "]" );

    if(!m_user_specified_VO.empty( ) )
//      if(m_logging)
	this->getLogger()->warn( string("Specified VO [") 
				 + m_user_specified_VO 
				 + "] as command line argument"
				 + " has been overrided by VO from certificate [" 
				 + VO_from_cert + "]") ;

  } else {
    // NO VO in cert
    if( !m_user_specified_VO.empty( ) )
      {
	// VO specified by user
	m_VO = m_user_specified_VO;
      } else {
      mustcheckVO_in_JDL = true;
    }
  }

  /** 
   * 
   *Create a jdlHelper object that
   * - checks if the JDL file specified by the user is there
   * - check syntax correctness using WMS.JDL's API
   * - tries to extract VO from JDL if not 
   *
   */

  vector<jdlHelper*> good_helpers;

  if(mustcheckVO_in_JDL) {
    good_helpers = this->check_VO_JDL( m_helpers );
  } else {
	good_helpers = m_helpers;
  }

  if(good_helpers.empty()) {
    return 0;
  }

  m_VO = (*good_helpers.begin())->getVO();

  /**
   * Configuration initialization
   */
  if(!this->initConfigurationFile( m_VO, m_execution_fail_message ) ) {
    //    this->freeHelpers();
    return 1;
  }

  this->set_logfile( "SUBMIT_LOG_DIR", "/tmp/glite_cream_cli_logs", "glite-ce-job-submit" );

  /* ****************************************************************************
   *
   * Will check if the CEURL has the correct
   * format that must be host:port/cream-<batchsystem>-<queuename>
   *
   */
  vector<string> ceid_pieces;
  try {
    apiutil::CEUrl::parseCEID(m_ceid, 
			      ceid_pieces, 
			      this->getConfMgr()->getProperty("DEFAULT_CREAM_TCPPORT","8443")
			      );

  } catch(apiutil::CEUrl::ceid_syntax_ex& ex) {
    // ERROR
    m_execution_fail_message = ex.what();
    //this->freeHelpers();
    return 1;
  }

  good_helpers = this->process_JDL( good_helpers, mustcheckVO_in_JDL, ceid_pieces );
  if( good_helpers.empty() )
    return 0;

  good_helpers = this->process_ISB( good_helpers );
  if(good_helpers.empty())
    return 0;

  good_helpers = this->process_ISB2( good_helpers );
  if(good_helpers.empty())
    return 0;

  /**
   *
   * Gets queue name and batch system name
   * from the CEURL command line argument and
   * appends them to the service URL name. Then
   * append the actual path context of the
   * deployed cream service.
   *
   */
   
  string endpoint = ceid_pieces[0];
   
  string serviceAddress = this->getConfMgr()->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
    cliUtils::getCompleteHostName(ceid_pieces[0]) + 
    ":" + 
    ceid_pieces[1]+
    this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
   
   
  string delService = this->getConfMgr()->getProperty("CREAMDELEGATION_URL_PREFIX", DEFAULT_PROTO)+
    cliUtils::getCompleteHostName(ceid_pieces[0]) + 
    ":" + 
    ceid_pieces[1]+
    this->getConfMgr()->getProperty("CREAMDELEGATION_URL_POSTFIX", DEFAULT_CEURLDELEGATION_POSTFIX);
   
  if(m_fake) {
    //    this->freeHelpers();
    return 0;
  }

  int f = - 1;
  if(m_jobid_on_file) {
     
    bool exists = false;

    if((exists=cliUtils::fileExists(m_outputfile.c_str()))==true) {
      if(!cliUtils::fileIsWritable(m_outputfile.c_str())){
	m_execution_fail_message = "Output file exists but is not accessible. Stop";
	return 1;
      }
    }

    if(exists) {
      bool isjoblist = cliUtils::isACreamJobListFile(m_outputfile.c_str());
      if(!isjoblist) {
	if(!m_noint) {
	  printf("\nWARNING: file [%s] already exists and is not a CREAM job list file. Overwrite it (y/n) ? ", m_outputfile.c_str());
	  char c;
	  cin >> c;
	  if(c!='y') {
	    printf("\nSubmission aborted. Bye!\n");
	    //	    this->freeHelpers();
	    return 0;
	  }
	}
	// Is NOT a JobList => opens file and truncs it
	try { f=cliUtils::openJobListFile(m_outputfile.c_str(), true); }
	catch(apiutil::file_ex& fex) {
	  m_execution_fail_message = fex.what();
	  //	  this->freeHelpers();
	  return(1);
	}
      } else {
	// IS a JobList file => opens file in append mode
	try { f=cliUtils::appendToJobListFile(m_outputfile.c_str()); }
	catch(apiutil::file_ex& fex) {
	  m_execution_fail_message = fex.what();
	  //	  this->freeHelpers();
	  return(1);
	}
      }
    } else {  
      // DOESN'T EXIST => create file from scratch
      try { f=cliUtils::openJobListFile(m_outputfile.c_str(), false); }
      catch(apiutil::file_ex& fex) {
	m_execution_fail_message = fex.what();
	//	this->freeHelpers();
	return(1);
      }
    }
  }// if(jobid_on_file)









  string jID;
  string ISB_upload_url;
   
  bool ISB_to_send = false;   
  // Check if at least one JDL has ISB to send
  for( vector<jdlHelper*>::const_iterator it = good_helpers.begin( ); it != good_helpers.end( ); ++it ) {
    if( (*it)->getAbsolutePathSize( )) {
      ISB_to_send = true;
      break;
    }
  }
  
   
  string jdlExtras = this->getConfMgr()->getProperty("JDL_DEFAULT_ATTRIBUTES","");

  {
    // Check if at least one JDL has ISB to send
    vector<jdlHelper*>::const_iterator it;
    vector<jdlHelper*> to_submit_helpers;
    for( it = good_helpers.begin( ); it != good_helpers.end( ); ++it ) {
      
      try {
	(*it)->addFrontClassAd( jdlExtras );
      }
      catch(JDLError& ex) {
	this->getLogger()->warn( string("ERROR in JDL [")
				 + (*it)->getFileName()
				 + "]: " + ex.what() 
				 + ". Will NOT submit this JDL.");
	continue;
      }
      catch(JDLEx& ex) {
	this->getLogger()->warn( string("ERROR in JDL [")
				 + (*it)->getFileName()
				 + "]: " + ex.what() 
				 + ". Will NOT submit this JDL.");
	continue;
      }
      catch(exception& ex) {
	this->getLogger()->warn( string("ERROR in JDL [")
				 + (*it)->getFileName()
				 + "]: " + ex.what() 
				 + ". Will NOT submit this JDL.");
	continue;
      }
      catch(...) {
	this->getLogger()->warn( string("ERROR in JDL [")
				 + (*it)->getFileName()
				 + "]: UNKNOWN EXCEPTION CATCHED" 
				 + ". Will NOT submit this JDL.");
	continue;
      }
      
      if( !(*it)->checkPrologue() )
	{
	  this->getLogger()->warn( string("ERROR in JDL [")
				   + (*it)->getFileName()
				   + "]: " 
				   + "specified 'PrologueArguments' attribute without 'Prologue'"
				   + ". Will NOT submit this JDL.");
	  continue;
	}
      
      if( !(*it)->checkEpilogue() )
	{
	  
	  this->getLogger()->warn( string("ERROR in JDL [")
				   + (*it)->getFileName()
				   + "]: " 
				   + "specified 'EpilogueArguments' attribute without 'Epilogue'"
				   + ". Will NOT submit this JDL.");
	  
	  continue;
	}
      
      to_submit_helpers.push_back(*it);
      
    }

    if( to_submit_helpers.empty() )
      return 0;

    good_helpers.clear();
    good_helpers = to_submit_helpers;
  }


  if (m_automatic_delegation) {
    if(!this->make_delegation( delService ) ) {
      //      this->freeHelpers();
      return 1;
    }
  }

  //apiproxy::AbsCreamProxy::RegisterArrayRequest reqs;
  list<apiproxy::JobDescriptionWrapper*> reqs;

  

  {
    /************************************************
     *
     * Build up the array of request from the JDLs
     *
     */
    vector<jdlHelper*>::const_iterator it;
    for( it = good_helpers.begin( ); it != good_helpers.end( ); ++it ) {
       
      string delegationProxy = "";

      string lease_id = "";

      apiproxy::JobDescriptionWrapper* jd 
	= new apiproxy::JobDescriptionWrapper((*it)->toString( ), 
					      m_delegation_id, 
					      delegationProxy, 
					      lease_id, 
					      false, 
					      (*it)->getJobName() );
      reqs.push_back( jd );
      if(m_debug) {
	string os = string("Registering to [") + serviceAddress 
	  + "] JDL=" + (*it)->toString() + " - JDL File=["
	  + (*it)->getFileName() + "]";

//	if(m_logging)
	  this->getLogger( )->debug( os );
      }
    }
  }

  
  /**
   *
   * As the scope quits the execute method all the JobDescriptionWrapper objects
   * added to reqs will be automaticalli deleted by the rcleaner's DTOR
   *
   */
  request_cleaner rcleaner( reqs );



  /**
   * Real submission
   *
   */

  vector<string> jIDs_complete;

  try {
     
    map< string, string > properties;
    string localCreamJID, creamURL;

    apiproxy::AbsCreamProxy::RegisterArrayResult resp;
     
    m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyRegister( &reqs, &resp, m_soap_timeout );
     
    if(!m_creamClient) 
      {
	m_execution_fail_message = "FAILED TO CREATE AN AbsCreamProxy OBJECT! STOP!";
	//	this->freeHelpers();
	return 1;
      }

      try {
    m_creamClient->setCredential( m_certfile );
  } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    //this->getLogger()->fatal( ex.what( ) );
    m_execution_fail_message = ex.what();
    return 1;
  }

    m_creamClient->execute( serviceAddress ); // can raise something
     
     
    /**
     *
     * MUST clean heap allocated reqs
     *
     */
    vector<jdlHelper*>::const_iterator it = good_helpers.begin();

    vector<string> jIDs;

    for(; it!= good_helpers.end(); ++it ) {
       
      boost::tuple<bool, apiproxy::JobIdWrapper, string> thisRegRes = resp[(*it)->getJobName()];
       
      if(apiproxy::JobIdWrapper::OK != thisRegRes.get<0>()) {
	m_execution_fail_message = thisRegRes.get<2>();
	this->getLogger()->warn( string("ERROR submitting JDL [")
				 + (*it)->getFileName()
				 + "]: " + thisRegRes.get<2>()
				 + "."); 
	continue;
	//	return 1;
      }

      creamURL = thisRegRes.get<1>().getCreamURL();
       
      string::size_type loc = creamURL.find(this->getConfMgr()->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX));
       
      if( loc != string::npos)
	{
	  creamURL = creamURL.substr(0,loc);
	}
       
      localCreamJID = thisRegRes.get<1>().getCreamJobID();
      jID = creamURL + "/" + localCreamJID; 

      thisRegRes.get<1>().getProperties( properties );
       
      if( ISB_to_send ) {
	ISB_upload_url = properties["CREAMInputSandboxURI"];
	string os = string("JobID=[") + jID + "]";
//	if( m_logging) {
	  this->getLogger( )->debug( os );
	  os = string("UploadURL=[") + ISB_upload_url + "]";
	  this->getLogger( )->debug( os );
//	}
	 
	{
	  ftpclient ftp( (string("gsiftp://")+endpoint).c_str(), m_ftp_num_streams );
	   
	  set<string> absPaths;
	  (*it)->getAbsolutePath( absPaths );
	   

	  bool thisjob_good = true;

	  for(set<string>::const_iterator fit = absPaths.begin();
	      fit != absPaths.end();
	      fit++) 
	    {
	      string targetFname = cliUtils::getFileName( (string&)*fit );
//	      if(m_logging)
		this->getLogger()->info( string("Sending file [")+ISB_upload_url + "/" + targetFname+"]" );
	       
	      if(!ftp.put( fit->c_str( ),  (ISB_upload_url + "/" + targetFname).c_str() ) ) {
		//m_execution_fail_message = string("Error sending file [") + *fit + "]. Stop.";
		this->getLogger()->warn( string("ERROR sending file [")
					 + *fit 
					 + "] for JDL [" 
					 + (*it)->getFileName()
					 +"]. Will NOT START this Job.");

					 
		// TODO: abortire la sottomissione solo del job corrente NON di tutti i job come adesso.
		thisjob_good = false;
		break;
	      }
	    }

	  if (thisjob_good == false ) {
	    continue; // skip to next JDL
	  }

	}// destroy the ftp object that closes connection with gridftp server

	/**
	 *
	 * if here the register has been OK, and the ISB send too!
	 * Then can put the JobID to the list of jobs to START
	 *
	 */
	jIDs.push_back( localCreamJID );
	jIDs_complete.push_back( jID );

      } else {//if( ISB_to_send)
	jIDs.push_back( localCreamJID );
	jIDs_complete.push_back( jID );
      }
    } // loop over the helpers  
     




    // Now proceed with JOBSTART
    vector< apiproxy::JobIdWrapper > param1;
    for(vector<string>::const_iterator it=jIDs.begin();
	it != jIDs.end();
	++it) 
      {
	 
//	if( m_logging )
	  this->getLogger( )->debug( "Will invoke JobStart for JobID [" + *it + "]" );
	 
	vector<apiproxy::JobPropertyWrapper> jpw_vector;
	apiproxy::JobIdWrapper jiw(*it, creamURL, jpw_vector);
	param1.push_back( jiw );
	 
      }
     
    apiproxy::ResultWrapper result;
    apiproxy::JobFilterWrapper jfw( param1, vector<string>(), -1, -1, m_delegation_id, m_lease_id );
     
    delete m_creamClient;
    m_creamClient = apiproxy::CreamProxyFactory::make_CreamProxyStart( &jfw, &result, m_soap_timeout );
    if(!m_creamClient)
      {
	m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy OBJECT! STOP!";
	//	this->freeHelpers();
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
     
     
  } catch(BaseException& ex) {

    //    this->freeRequests( reqs );

    m_execution_fail_message = ex.what();
    //    this->freeHelpers();
    return 1;
  } catch( exception& ex )  {

    //    this->freeRequests( reqs );

    m_execution_fail_message = ex.what();
    //    this->freeHelpers();
    return 1;
  }

  //  this->freeRequests( reqs );

  if(m_jobid_on_file) {
    for(vector<string>::const_iterator it = jIDs_complete.begin();
	it != jIDs_complete.end();
	++it)
      {
	try { cliUtils::writeJobID(f, *it); }
	catch(apiutil::file_ex& fex) {
	  m_execution_fail_message = fex.what();
	  close(f);
	  //	  this->freeHelpers();
	  return 1;
	} 
	close(f);
      }
  }

  m_jobids = jIDs_complete;//jID;
   
  //this->freeHelpers();

  return 0;
}


// //______________________________________________________________________________
// void cli_service_jobsubmit::freeHelpers( ) 
// {

//   for(vector<jdlHelper*>::iterator it = m_helpers.begin();
//       it != m_helpers.end();
//       ++it)
//     {
//       //cout << "Deleting JDL [" << (*it)->getFileName() << "]" << endl;
//       delete *it;
//       //cout << "Deleted !!" << endl;
//     }
//     m_helpers.clear();
// }

// //______________________________________________________________________________
// void cli_service_jobsubmit::freeRequests( std::list<apiproxy::JobDescriptionWrapper*>& reqs ) {

//   return;

//   apiproxy::AbsCreamProxy::RegisterArrayRequest::iterator it = reqs.begin();
//   for( ; it != reqs.end(); ++it ) {
//     delete (*it);
//   }
//   reqs.clear();
// }

//______________________________________________________________________________
bool cli_service_jobsubmit::make_delegation( const string& deleg_service ) throw()
{
  //  string delegID;
  glite::ce::cream_client_api::certUtil::generateUniqueID( m_delegation_id );
  apiproxy::AbsCreamProxy *creamDelegClient = apiproxy::CreamProxyFactory::make_CreamProxyDelegate( m_delegation_id, m_soap_timeout );

  if(!creamDelegClient)
    {
      m_execution_fail_message = "FAILED CREATION OF AN AbsCreamProxy OBJECT! STOP!";
      return false;
    }

  //  boost::scoped_ptr< AbsCreamProxy > tmpClient;
  //  tmpClient.reset( creamDelegClient );

  //cout << "make_delegation: deleg_service=[" << deleg_service << "]" << endl;

    try {
    creamDelegClient->setCredential( m_certfile );
  } catch( glite::ce::cream_client_api::soap_proxy::auth_ex& ex ) {
    //this->getLogger()->fatal( ex.what( ) );
    m_execution_fail_message = ex.what();
    return false;
  }

  try {
    creamDelegClient->execute( deleg_service );
    //creamDelegClient->garbage_collect();
  } catch(exception& ex) {
    m_execution_fail_message = ex.what();
    delete creamDelegClient;
    return false;
  }
  delete creamDelegClient;
  return true;
}
