/* LICENSE:
Copyright (c) Members of the EGEE Collaboration. 2010.
See http://www.eu-egee.org/partners/ for details on the copyright
holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.
See the License for the specific language governing permissions and
limitations under the License.

END LICENSE */

#include <fstream>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

#include "cliUtils.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"

extern int h_errno;

using namespace std;
using namespace glite::ce::cream_client_api::soap_proxy;
using namespace glite::ce::cream_client_api::util;
using namespace glite::ce::cream_client_api;

#define CREAMJOBFILETAG "##CREAMJOBS##\n"

void cliUtils::processResult( const ResultWrapper& result,
		               map<string, string>& target ) throw()
{
  list<pair<JobIdWrapper,string> > notExistingJobs, notMatchingStatusJobs, notMatchingDateJobs, notMatchingProxyDidJobs,  notMatchingLeaseJobs;//           notfound, status_not_compatible;
  
  result.getNotExistingJobs( notExistingJobs );
  result.getNotMatchingStatusJobs( notMatchingStatusJobs );
  result.getNotMatchingDateJobs( notMatchingDateJobs );
  result.getNotMatchingProxyDelegationIdJobs( notMatchingProxyDidJobs );
  result.getNotMatchingLeaseIdJobs( notMatchingLeaseJobs );
  
  list<pair<JobIdWrapper, string> >::const_iterator it;
  for( it= notExistingJobs.begin(); it != notExistingJobs.end(); ++it ){
    target[it->first.id] = string("This job has not been found on the CREAM server. Server error message is [") + it->second + "]";
  }
  
  for( it=notMatchingStatusJobs.begin(); it != notMatchingStatusJobs.end(); ++it ){
    target[it->first.id] = string("This job was not matching the status or had a status incompatible for operation. Server error message is [") + it->second + "]";
  }

  for( it=notMatchingDateJobs.begin(); it != notMatchingDateJobs.end(); ++it ){
    target[it->first.id] = string("This job was not matching the --from/--to filter. Server error message is [") + it->second + "]";
  }

  for( it=notMatchingProxyDidJobs.begin(); it != notMatchingProxyDidJobs.end(); ++it ){
    target[it->first.id] = string("This job was not matching the proxy deleg. Server error message is [") + it->second + "]";
  }

  for( it=notMatchingLeaseJobs.begin(); it != notMatchingLeaseJobs.end(); ++it ){
    target[it->first.id] = string("This job was not matching the lease ID. Server error message is [") + it->second + "]";
  }
}

//______________________________________________________________________________
void cliUtils::printResult( const ResultWrapper& result ) throw()
{
  list<pair<JobIdWrapper,string> > notExistingJobs, 
    notMatchingStatusJobs, 
    notMatchingDateJobs, 
    notMatchingProxyDidJobs,  
    notMatchingLeaseJobs;
  
  result.getNotExistingJobs( notExistingJobs );
  result.getNotMatchingStatusJobs( notMatchingStatusJobs );
  result.getNotMatchingDateJobs( notMatchingDateJobs );
  result.getNotMatchingProxyDelegationIdJobs( notMatchingProxyDidJobs );
  result.getNotMatchingLeaseIdJobs( notMatchingLeaseJobs );
  
  list<pair<JobIdWrapper, string> >::const_iterator it;
  for( it= notExistingJobs.begin(); it != notExistingJobs.end(); ++it )
    creamApiLogger::instance()->getLogger()->error("This job has not been found on the CREAM server: [%s] - Server error Message is: [%s]", it->first.id.c_str(), it->second.c_str());
  
  for( it=notMatchingStatusJobs.begin(); it != notMatchingStatusJobs.end(); ++it )
    creamApiLogger::instance()->getLogger()->error("This job was not matching the status or had a status incompatible for operation: [%s] - Server error Message is: [%s]", it->first.id.c_str(), it->second.c_str()); 

  for( it=notMatchingDateJobs.begin(); it != notMatchingDateJobs.end(); ++it )
    creamApiLogger::instance()->getLogger()->error("This job was not matching the --from/--to filter: [%s] - Server error Message is: [%s]", it->first.id.c_str(), it->second.c_str()); 

  for( it=notMatchingProxyDidJobs.begin(); it != notMatchingProxyDidJobs.end(); ++it )
    creamApiLogger::instance()->getLogger()->error("This job was not matching the proxy deleg. ID: [%s] - Server error Message is: [%s]", it->first.id.c_str(), it->second.c_str()); 

  for( it=notMatchingLeaseJobs.begin(); it != notMatchingLeaseJobs.end(); ++it )
    creamApiLogger::instance()->getLogger()->error("This job was not matching the lease ID: [%s] - Server error Message is: [%s]", it->first.id.c_str(), it->second.c_str()); 
}

//______________________________________________________________________________
void cliUtils::getJobIDFromFile(vector<string>& target, const char* filename)
{
  ifstream is( filename , ios::in );
  is.seekg( strlen(CREAMJOBFILETAG) );
  string buffer = "";
  
  while(is.peek()!=EOF) {
    std::getline(is, buffer, '\n');
    if(!buffer.length()) continue;
    target.push_back(buffer);
  }
}

//______________________________________________________________________________
bool cliUtils::isACreamJobListFile(const char* filename) throw(file_ex&)
{
  bool it_is = true;

  ifstream is(filename, ios::in);
  if(!is)
    throw file_ex(string("error opening ")+filename);

  string magic;

  std::getline(is, magic, '\n');
  magic += '\n';

  if(magic!=CREAMJOBFILETAG) it_is=false;

  return it_is;
}

//______________________________________________________________________________
void cliUtils::removeJobIDFromFile(const vector<string>& toRemove,
				    const char* filename) throw(file_ex&)
{
  ifstream is( filename , ios::in );
  is.seekg(strlen(CREAMJOBFILETAG));
  string buffer;
  vector<string> toLeave;
  toLeave.reserve(1000);
  bool found = false;
  while(is.peek()!=EOF) {
    found = false;
    std::getline(is, buffer, '\n');
    if( !buffer.length() ) continue;
    for(vector<string>::const_iterator it = toRemove.begin();
	it!=toRemove.end(); it++)
      if( *(it) == buffer ) found=true;
    if(!found) toLeave.push_back(buffer);
  }
  is.close();
  ofstream os( filename, ios::trunc);

  os << CREAMJOBFILETAG<<flush;

  for(vector<string>::iterator it=toLeave.begin();
      it!=toLeave.end(); it++)//unsigned int j=0; j<toLeave.size(); j++)
    os << /*toLeave[j]*/*(it) << endl;

}

//______________________________________________________________________________
int cliUtils::openJobListFile(const char* filename, const bool& trunc)
  throw(file_ex&)
{
  int fd, flag = O_RDWR|O_LARGEFILE;
  if(trunc) flag |= O_TRUNC;
  else flag |= O_CREAT;
  fd = open(filename, flag);
  if(fd==-1) throw file_ex(strerror(errno));

  if(-1==write(fd, (const void*)CREAMJOBFILETAG, strlen(CREAMJOBFILETAG)))
    throw file_ex(strerror(errno));
  if(-1 == fchmod(fd, S_IRUSR|S_IWUSR)) {
    close(fd);
    throw file_ex(strerror(errno));
  }
  return fd;
}

//______________________________________________________________________________
int cliUtils::appendToJobListFile(const char* filename) throw(file_ex&)
{
  int fd, flag = O_RDWR|O_LARGEFILE|O_APPEND;
  fd = open(filename, flag);
  if(fd==-1) throw file_ex(strerror(errno));
  return fd;
}

//______________________________________________________________________________
void cliUtils::writeJobID(const int& fd, const string& JID) throw(file_ex&) {
  //JID.append("\n");
  if(-1==write(fd, (const void*)((JID+"\n").c_str()), JID.length()+1))
    throw file_ex(strerror(errno));
}


//______________________________________________________________________________
string cliUtils::getFileName(const string& pathname)
{
  if(boost::ends_with(pathname, "/")) return "";
  int pos = boost::find_last(pathname, "/").begin() - pathname.begin();
  return pathname.substr(pos+1, pathname.length()-pos);
}

//______________________________________________________________________________
string cliUtils::getCompleteHostName(const string& host) {
  struct hostent *H = gethostbyname2( host.c_str(), AF_INET6 );
  if(!H) return host;
  return string(H->h_name);
}

//______________________________________________________________________________
bool cliUtils::fileExists(const char* filename) throw(file_ex&) {
  struct stat buf;
  if(-1==stat(filename, &buf)) {
    int saveerr = errno;
    switch(saveerr) {
      // the output file doesn't exist. OK, let's create a new one
    case ENOENT:
      return false;
      break;
    default:
      // the output file already exists but is not readable
      // and/or writable or some other serious error occurred. STOP!
      // ERROR
      throw file_ex(strerror(saveerr));
      break;
    }
  }
  return true;
}

//______________________________________________________________________________
bool cliUtils::fileIsReadable(const char* filename) throw(file_ex&) {
  struct stat buf;
  if(-1==stat(filename, &buf))
    throw file_ex(strerror(errno));
  return (bool)((buf.st_mode & S_IRUSR) | (buf.st_mode & S_IRGRP) |
		(buf.st_mode & S_IROTH));
}

//______________________________________________________________________________
bool cliUtils::fileIsWritable(const char* filename) throw(file_ex&) {
  struct stat buf;
  if(-1==stat(filename, &buf))
    throw file_ex(strerror(errno));
  return (bool)(buf.st_mode&S_IWUSR);
}

//______________________________________________________________________________
vector<string> cliUtils::getConfigurationFiles(const string& VO, 
					       const string& user_specified_cnf,
					       const string& _default)
  throw (no_user_confile_ex&)
{
  vector<string> files;
  files.reserve(32);
  bool user_spec_ok=false;

  /**
   * Pushes user's specified conf file
   *
   */
  if(user_specified_cnf!="") {
    if((!cliUtils::fileExists(user_specified_cnf.c_str())) ||
       (!cliUtils::fileIsReadable(user_specified_cnf.c_str())))
      throw no_user_confile_ex("the user specified configuration file isn't there or it is not readable");
    files.push_back(user_specified_cnf);
    user_spec_ok=true;
  }

  char*  tmp;
  string glite_file="";
  string home = "";
  
  if((tmp=getenv("GLITE_CREAM_CLIENT_CONFIG"))!=NULL)
    glite_file = tmp;
  
  if((tmp=getenv("HOME"))!=NULL)
    home = tmp;
  
  /**
   * Pushes user specific conf files
   *
   */
  try {
    if(!user_spec_ok) {
      // the user didn't specify any custom conf file
      if((!cliUtils::fileExists(glite_file.c_str())) ||
	 (!cliUtils::fileIsReadable(glite_file.c_str()))) {
	// trying the HOME's VO specific one
	string home_vo = home+"/.glite/"+VO+"/glite_cream.conf";
	if((!cliUtils::fileExists(home_vo.c_str())) ||
	   (!cliUtils::fileIsReadable(home_vo.c_str())))
	  {
	    // the HOME's VO specific one isn't there
	    // trying the HOME's specific
	    string home_novo = home+"/.glite/glite_cream.conf";
	    if((!cliUtils::fileExists(home_novo.c_str())) ||
	       (!cliUtils::fileIsReadable(home_novo.c_str()))) {
	      ;
	    } else {
	      if(files.size())
		files.insert(files.begin(), home_novo);
	      else
		files.push_back(home_novo);
	    }
	  } else {
	  if(files.size())
	    files.insert(files.begin(), home_vo);
	  else
	    files.push_back(home_vo);
	}
      } else {
	if(files.size())
	  files.insert(files.begin(), glite_file);
	else
	  files.push_back(glite_file);
      }
    }
     
    /**
     * pushes VO specific conf files
     *
     */
    string glite_loc_VO_file = "/etc/"+VO+"/glite_cream.conf";
    
    if(cliUtils::fileExists(glite_loc_VO_file.c_str()) &&
       cliUtils::fileIsReadable(glite_loc_VO_file.c_str())) {
      if(files.size())
	files.insert(files.begin(), glite_loc_VO_file);
      else
	files.push_back(glite_loc_VO_file);
    } else {
    }

    /**
     * pushes generic conf files
     *
     */
    string glite_loc_file = "/etc/glite_cream.conf";
    
    if(cliUtils::fileExists(glite_loc_file.c_str()) &&
       cliUtils::fileIsReadable(glite_loc_file.c_str())) {
      if(files.size())
	files.insert(files.begin(), glite_loc_file);
      else
	files.push_back(glite_loc_file);
    } else {
    }


  } catch(file_ex&) {
    /**
     * this exception is raised if the stat function generate an error
     * different than "file not found"
     */
  }

  return files;
}

//______________________________________________________________________________
string cliUtils::getProxyCertificateFile(void)
  throw (glite::ce::cream_client_api::soap_proxy::auth_ex&)
{
  const char* _cert;
  string certfile;
  if(!(_cert = getenv("X509_USER_PROXY"))) {
    string tmp = "/tmp/x509up_u";
    certfile = (string&)tmp + boost::str( boost::format("%d") % ::getuid() );
  } else certfile = _cert;

  struct stat buf;
  if(-1 == stat(certfile.c_str(), &buf)) {
    throw glite::ce::cream_client_api::soap_proxy::auth_ex(("Certificate file "
		   + certfile
		   + " is not there or is not accessible").c_str());
  }
  if(!(buf.st_mode & S_IRUSR)) {
    throw glite::ce::cream_client_api::soap_proxy::auth_ex(("Certificate file [" + certfile + "] is not readable").c_str());
  }

  return certfile;
}

//______________________________________________________________________________
string cliUtils::getPath(const string& pathfile)
{
  if(boost::ends_with(pathfile, "/")) return pathfile;
  int pos = boost::find_last(pathfile, "/").begin() - pathfile.begin();
  return pathfile.substr(0, pos);
}

//______________________________________________________________________________
void cliUtils::mkdir(const string& path)
{
  vector<string> pieces;
  boost::split(pieces, path, boost::is_any_of("/"));
  string pathToCreate("");
  for(vector<string>::const_iterator pit = pieces.begin(); pit!=pieces.end(); pit++)
  {
    pathToCreate += "/" + *pit;
    try{boost::filesystem::create_directory( pathToCreate );}
    catch(exception&){}
  }
}

//______________________________________________________________________________
bool cliUtils::checkEndpointFormat(const string& endpoint)
  throw(internal_ex&)
{
  const boost::regex match("^([^:])+(:[0-9]{1,5})?$");
  try {
    if( !boost::regex_match( endpoint.c_str(), match) ) {
      return false;
    }
  } catch(exception& ex) { 
    throw internal_ex(string("boost::regex raised an exception matching string [")+endpoint+"]. Error is: "+ex.what()+". Stop");
  }
  if( endpoint.find("/", 0) != string::npos ) {
    return false;
  }
  unsigned pos = endpoint.find(":", 0);
  if(pos != string::npos) {
    string port = endpoint.substr(pos+1, endpoint.length()-1);
    if(atoi(port.c_str()) > 65535) return false;
  }
  return true;
}

//______________________________________________________________________________
bool cliUtils::containsTCPPort(const string& endpoint)
  throw(internal_ex&)
{
  const boost::regex tmp( "^([^:])+:([0-9]{1,5})$" );
  boost::smatch what;
  try {
    if( !boost::regex_match( endpoint, what, tmp ) ) return false;
  } catch(exception& ex) {
    throw( internal_ex(string("boost::regex raised an exception matching string [")+
		       endpoint+"]. Error is: "+ex.what()+". Stop") );
  } 
  return true;
}

//______________________________________________________________________________
vector<string> cliUtils::expandWildcards(const vector<string>& files)
  throw(internal_ex&)
{
  vector<string> expanded;

  for(vector<string>::const_iterator fit = files.begin();
      fit != files.end();
      ++fit)
    {
      if( (fit->find("*", 0) == string::npos) 
          && (fit->find("?", 0) == string::npos)) 
	{
	  expanded.push_back(*fit);
	  continue;
	}
      cliUtils::expand(*fit, expanded);
    }
  return expanded;
}

//______________________________________________________________________________
void cliUtils::expand( const string& file, vector<string>& target )
  throw(internal_ex&)
{
  
  string cmd = "/bin/ls -1 " + file + "  2>/dev/null";
  FILE *list = ::popen( cmd.c_str(), "r" );
  if( !list ) {
    throw( internal_ex(string("A severe error occurred while resolving [")
		       +file+"] files: "+strerror(errno)));
  }

  string output("");
  char c;
  while(!feof(list)) {
    c = fgetc(list);
    if( (c=='\n') && (output.length()) ) {
      boost::trim_if( output, boost::is_any_of("\n")); 
      target.push_back( output );
      output = "";
    }
    output += c;
  }
  if( -1 == pclose(list) ) // the command failed for some reason
    {
      throw( internal_ex(string("A severe error occurred while resolving [")
			 +file+"] files: "+strerror(errno)));
    }
}


//______________________________________________________________________________
bool cliUtils::interactiveChoice( const char* commandName, 
				  const char* joblist_file,
				  const bool noint, 
				  const bool debug,
				  const bool nomsg,
				  bool& process_all_jobs_from_file,
				  vector<string>& target, 
				  vector<string>& source,
				  string& errMex ) throw()
{
  int f, jcounter = 0;
  if(debug) {
    string os = string("Getting job list from file [") + joblist_file + "]";
    creamApiLogger::instance()->log(log4cpp::Priority::INFO, os, true, true, !nomsg);
  }
  
  if((f=open(joblist_file, O_RDONLY))==-1) {
    int saveerr = errno;
    errMex = string("Error accessing file [") + joblist_file + "]: " + strerror(saveerr) + ". Stop.";
    return false;
  }
  close(f);
  
  bool isjoblist = cliUtils::isACreamJobListFile(joblist_file);
  if(!isjoblist) {
    errMex = string("File [") + joblist_file + "] is not a CREAM job list file. Stop.";
    return false;
  }

  cliUtils::getJobIDFromFile(source, joblist_file );
  
  if(!noint) {
    string os = string("\nFound the following jobs in [") + joblist_file + "]: ";
    creamApiLogger::instance()->log(log4cpp::Priority::INFO, os, false, false, true);
  }

  string whichone = "";
  if(!noint) {
    printf("\n");
    for(vector<string>::const_iterator jit = source.begin();
	jit != source.end();
	jit++) {
      printf("%s\n", (boost::str( boost::format("%d") % jcounter++) + ". " + *jit).c_str());
    }
    printf("a. %s all jobs\n", commandName);
    printf("q. Quit\n");
    printf("\nYour choice ? (multiple choices separated by ',') ");
    cin >> (string&)whichone;
    if( whichone == "q" ) {
      printf("%s aborted. Bye.\n", commandName);
      return false;
    }
    
    /**
     * must check the format of whichone
     */
    if(whichone != "a") {
      /**
       * check for a format like \d[,\d]+
       */
      boost::cmatch what;
      const boost::regex pattern("[0-9]+(,[0-9]+)*");
      if(!boost::regex_match(whichone.c_str(), what, pattern) ) {
	errMex = "Response must be \"a\" or \"q\" or numbers ','-separated. Stop";
	return false;
      }
    }
    
  } else whichone = "a";
  if( whichone == "a" ) process_all_jobs_from_file=true;
  boost::split(target, whichone, boost::is_any_of(","));
  return true;
}

//______________________________________________________________________________
void stripCreamURL::operator()( const string& jid ) 
{
  vector<string> pieces;
  string tmpJid;
  
  try { 
    CEUrl::parseJobID( jid , pieces, m_confMgr->getProperty("DEFAULT_CREAM_TCPPORT", "8443")); 
  } catch(CEUrl::ceid_syntax_ex& ex) {
    creamApiLogger::instance()->getLogger()->error( ex.what() );
    creamApiLogger::instance()->getLogger()->error( "This JobID doesn't contain the CREAM service address. Skipping it..." );
    return;
  } 
  string creamURL = m_confMgr->getProperty("CREAM_URL_PREFIX", DEFAULT_PROTO)+
    pieces[1]+':'+pieces[2]+ "/"+m_confMgr->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX);
  
  string::size_type loc = pieces[3].find(m_confMgr->getProperty("CREAM_URL_POSTFIX", DEFAULT_CEURL_POSTFIX));
  if( loc != string::npos )
    tmpJid = pieces[3].substr(loc+1, pieces[3].length()-loc);
  else 
    tmpJid = pieces[3];
  
  JobIdWrapper tmp(tmpJid, creamURL, std::vector<JobPropertyWrapper>());
  
  m_jobids->push_back( tmp );
  
}

