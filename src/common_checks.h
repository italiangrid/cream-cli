#ifndef GLITE_ES_CLI_COMMON_CHECKS_H_
#define GLITE_ES_CLI_COMMON_CHECKS_H_

#include <string>

#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>

using namespace glite::ce::cream_client_api::soap_proxy;
using namespace std;

/**
 *
 *
 * All common part of the glite-es checks:
 * - non empty-ness of endpoint
 * - correct format of endpoint (<hostname>[:port])
 * - existance and validity of proxyfile, and exp time extraction
 * - 
 *
 */		   
bool common_check( string&        endpoint,
		   bool           useInputFile,
		   const char*    certfile,
		   const bool     verify_ac,
		   time_t&        expirationtime,
		   string&        error )
{

  /**
   *
   *
   *
   * Check endpoint
   *
   *
   *
   *
   */
  if(!useInputFile) {
    if(endpoint.empty( ) ) {
      error = "Must specify an endpoint in the format <hostname>[:<tcp_port_number>] with option --endpoint|-e. Stop.";
      return false;
    }
  }
  

  /**
   *
   *
   *
   * Check endpoint's format goodness if not set inputFile
   *
   *
   *
   *
   */
  if( !useInputFile ) {
  
    boost::regex pattern;
    boost::cmatch what;
    pattern = "^([^:]+)://.+";
  
    if( boost::regex_match(endpoint.c_str(), what, pattern) ) {
      error = "Protocol ["+what[1]+"] not allowed in the endpoint";
      return false;
    }
  
//     if(boost::starts_with(endpoint, "https://")) {
//       error = "endpoint must not start with protocol https://";
//       return false;
//     }
//     
//     if(boost::starts_with(endpoint, "http://")) {
//       error = "endpoint must not start with protocol http://";
//       return false;
//     }
  
    vector<string> endpoint_pieces;
    boost::split( endpoint_pieces, endpoint, boost::is_any_of(":"));
    if(endpoint_pieces.size()<=1)
      endpoint += ":8443";
    else {
      int tcpport = ::atoi( endpoint_pieces[1].c_str() );
      if(!tcpport) {
	error = string("Specified TCP port number [")
	  + endpoint_pieces[1] 
	  + "] is not valid. Stop";
	return false;
      }
      if(endpoint_pieces[0].empty( ))
	{
	  error = "Must specify an endpoint in the format <hostname>[:<tcp_port_number>] with option --endpoint|-e. Stop.";
	  return false;
	}
    }
  }

  /**
   *
   *
   *
   * Check cert proxy file existance, goodness and validity
   *
   *
   *
   *
   */
  if(!certfile) // do not want to check proxy because we're using cert/key couple and not a proxy
    return true;
  if(!boost::filesystem::exists( boost::filesystem::path(certfile, boost::filesystem::native))) {
    error = string("Proxy file [") 
      + certfile + "] does not exist. Stop";
    return false;
  }

  if(!::getenv("ES_CLI_NOAUTH")) {
    try {
      
      VOMSWrapper V( certfile, verify_ac );
      if( !V.IsValid( ) ) {
	error = string("There's a problem with proxyfile [")
	  + certfile 
	  + "]: " + V.getErrorMessage();
	return false;
      }    
      
      if(V.getProxyTimeEnd() < time(0)+5)
	{
	  error = string("Proxy file [")
	    + certfile
	    + "] is expired or going to expire in 5 seconds";
	  return false;
	}
      expirationtime = V.getProxyTimeEnd();
      
    } catch(exception& s) {
      error = string("There's a problem with proxyfile [")
	+ certfile 
	+ "]: " 
	+ s.what();
      return false;
    }
  }

  return true;
}









bool check_input_format( const std::string& inputFile,
			 std::string& endpoint,
			 std::vector<std::string>& IDS,
			 std::string& error )
{
  vector<string> pieces;

  /**
   *
   * Check if input file exists
   *
   */
  if(!boost::filesystem::exists(inputFile)) {
    error = string("File [") + inputFile + "] does not exist";
    return false;
  }

  /**
   *
   * Check if input can be read
   *
   */
  ifstream input( inputFile.c_str(), ios::in );
  if(!input) {
    error = string("Couldn't open input file [") + inputFile + "]; check file's permissions";
    return false;
  }

  /**
   *
   * Load first line of input file
   *
   */
  string firstLine;
  input >> firstLine;

  boost::trim_right_if(firstLine, boost::is_any_of("\n"));
  boost::trim_right_if(firstLine, boost::is_any_of("#"));
  boost::trim_left_if(firstLine, boost::is_any_of("#"));
  
  /**
   *
   * Check if first line of input file
   * indicates it is a correct input file of activity ids
   *
   */
  boost::split(pieces, firstLine, boost::is_any_of("@"));
  if(pieces.size()!=2 || pieces[1].empty())
    {
      error = string("File [") + inputFile + "] is not a regular input file";
      return false;
    }

  endpoint = pieces[1];
  
  vector<string> pieces_endp;
  boost::split(pieces_endp, endpoint, boost::is_any_of(":"));
  if(pieces_endp.size()!=2 || pieces_endp[1].empty() || !atoi(pieces_endp[1].c_str()))
    {
      error = string("endpoint [") + endpoint + "] from input file is not in correct format; must be <hostname>:<TCP_PORT>";
      return false;
    }
  
  while(!input.eof())
    {
      firstLine = "";
      input >> firstLine;
      
      boost::trim_right_if(firstLine, boost::is_any_of("\n"));
      
      //cout << "check - ADD " << firstLine << endl;
      if(!firstLine.empty())
	IDS.push_back( firstLine );
    }

  return true;
}

#endif
