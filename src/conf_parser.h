#ifndef GLITE_ES_CLI_CONF_PARSER_H_
#define GLITE_ES_CLI_CONF_PARSER_H_

#include <string>

#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

//#include <utility>
#include <iostream>
#include <map>

using namespace std;

class OperationEndpoint {
  
  friend OperationEndpoint* get_OperationEndpoint( const string&,
						   const string&,
						   string& error);
  
  string m_endpoint;

 protected:
  map<string,      string> m_op_epr_map;
  
  OperationEndpoint( const string& endpoint ) : m_endpoint(endpoint), m_op_epr_map() {}

 public:

  string get_epr( const string& op ) {
    map<string, string>::const_iterator it;
    it = m_op_epr_map.find( op );
    if( it == m_op_epr_map.end() ) return "";
    return it->second;
  }
  
};

OperationEndpoint* get_OperationEndpoint( const string& confile, 
					  const string& ep, 
					  string& error) 
{
  if(!boost::filesystem::exists( confile ))
    {
      error = string("Conf file ") + confile + " does not exist";
      return 0;
    }
    
  if(boost::filesystem::is_directory( confile )) {
    error = string("Conf file ") + confile + " is a directory";
    return 0;
  }
    
  ifstream conf_file(confile.c_str());
  if(!conf_file) {
    error = string("Conf file ") + confile + " is there but it is not readable";
    return 0;
  }
  
  string firstLine;
  map<string, string > op_map;
  while(!conf_file.eof())
    {
      firstLine = "";
      conf_file >> firstLine;

      //cout << "firstLine="<< firstLine << endl;
      
      boost::trim_right_if(firstLine, boost::is_any_of("\n"));
      boost::trim_right_if(firstLine, boost::is_any_of("\t"));
      boost::trim_left_if(firstLine, boost::is_any_of("\t"));
      boost::trim_left_if(firstLine, boost::is_any_of(" "));
      boost::trim_right_if(firstLine, boost::is_any_of(" "));
      
      vector<string> pieces;
      boost::split(pieces, firstLine, boost::is_any_of(","));
      if(pieces.size()!= 3)
	continue;

      //cout << "pieces[0]=" << pieces[0] << endl;

      if( pieces[0] != ep ) {
	//cout << "Skipping..." << endl;
	continue;
      }
      
      //cout << "Mapping " << pieces[1] << " -> " << pieces[2] << endl;

      op_map[pieces[1]] = pieces[2];
      
    }

  if(op_map.size()) {
    
    OperationEndpoint *tmp = new OperationEndpoint( ep ); 
    tmp->m_op_epr_map = op_map;
    return tmp;
  }
  error = string("Configuration file is empty or doesn't contain information about endpoint ")+ep;
  return 0;
}

#endif
