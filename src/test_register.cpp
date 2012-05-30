/*
  Copyright (c) Members of the EGEE Collaboration. 2004. 
  See http://www.eu-egee.org/partners/ for details on the copyright
  holders.  
  
  Licensed under the Apache License, Version 2.0 (the "License"); 
  you may not use this file except in compliance with the License. 
  You may obtain a copy of the License at 
  
      http://www.apache.org/licenses/LICENSE-2.0 
  
  Unless required by applicable law or agreed to in writing, software 
  distributed under the License is distributed on an "AS IS" BASIS, 
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
  See the License for the specific language governing permissions and 
  limitations under the License.
*/

/*
        glite-ce-job-submit: a command line interface to submit to new glite CE

        Authors: Alvise Dorigo <alvise.dorigo@pd.infn.it>
        Version info: $Id: test_register.cpp,v 1.3.8.1.2.1.2.1.2.2 2012/01/11 15:57:07 adorigo Exp $
*/

/*
purify g++ -o test_register -DHAVE_CONFIG_H -D_GSOAP_VERSION=0x020706 -D_GSOAP_WSDL2H_VERSION=0x020706 -DUSEGSOAP_2_6 -DUSEGSOAPWSDL2H_2_6 -UHAVE_CONFIG_H -I../../stage/include -I../../repository/vdt/globus/4.0.3-VDT-1.6.0/slc4_ia32_gcc346/include/gcc32dbgpthr -I../../repository/org.glite/org.glite.security.gsoap-plugin/1.5.2/slc4_ia32_gcc346/include -I../../repository/externals/gsoap/2.7.6b/slc4_ia32_gcc346/include -I../../org.glite.ce.cream-client-api-c/build/src/autogen test_register.cpp ../../org.glite.ce.cream-client-api-c/build/src/autogen/CREAM_CLIENTC.cpp ../../org.glite.ce.cream-client-api-c/build/src/autogen/CREAM_CLIENTClient.cpp ../../repository/externals/gsoap/2.7.6b/slc4_ia32_gcc346/src/stdsoap2.cpp -L../../stage/lib -lglite_security_gsoap_plugin_276b_gcc32dbgpthr
*/

/**
 *
 * SYSTEM C/C++ INCLUDE FILES
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>
#include <fstream>
#include <dlfcn.h>
#include <cstdio>
#include <cerrno>

#include "glite/ce/cream-client-api-c/CREAM_CLIENT.nsmap"
#include "glite/ce/cream-client-api-c/cream_client_soapH.h"
#include "glite/ce/cream-client-api-c/cream_client_soapStub.h"

extern "C" {
#undef IOV_MAX
#include "glite/security/glite_gsplugin.h"
};

/**
 *
 * CREAM CLIENT API C++ INCLUDE FILES
 *
 */
#include "glite/ce/cream-client-api-c/AbsCreamProxy.h"
using namespace std;

char* namespaces[] = {};
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  glite_gsplugin_Context m_ctx = 0;

  int i = 0;

  string m_certfile = "/tmp/x509up_u501";

  string serviceURL = "";

  char *port = "8443";
  if(argv[1])
    port = argv[1];

  int ii = 0;
  cin >> ii;

  while(1) {
    
    struct soap *SOAP = soap_new();
    
    SOAP->header = NULL;
    
    if(!SOAP) {
      cerr << "Error creating the gSOAP runtime" << endl;
      return 1;
    }
    
    SOAP->socket_flags = MSG_NOSIGNAL;
    soap_set_namespaces(SOAP, CREAM_CLIENT_namespaces);
    
    if(!::getenv("TEST_NO_AUTHN")) {
      if ( glite_gsplugin_init_context( &m_ctx ) ) {
	m_ctx = NULL;
	cerr << "gsplugin initialization failed" << endl;
	return 1;
      }
      if (glite_gsplugin_set_credential(m_ctx,m_certfile.c_str(),m_certfile.c_str())) {
        glite_gsplugin_free_context( m_ctx );
        m_ctx = NULL;
	cerr << "Cannot set credentials in the gsoap-plugin context" << endl;
	return 1;
      }

      struct timeval T;
      T.tv_sec = (time_t)300;
      T.tv_usec = 0;
      
      glite_gsplugin_set_timeout( m_ctx, &T );
    }
    
    if(!::getenv("TEST_NO_AUTHN")) {
      if ( soap_register_plugin_arg( SOAP, glite_gsplugin, m_ctx ) ) {
	m_ctx = NULL;
	cerr << "soap_register_plugin_arg FAILED" << endl;
	return 1;
      }
      serviceURL = "https";
    } else {
      serviceURL = "http";
    }
    
    serviceURL += "://localhost:" + string(port) + "/ce-cream/services/CREAM2";

    _CREAMTYPES__JobRegisterResponse RESP;
    _CREAMTYPES__JobRegisterRequest REQ;
    CREAMTYPES__JobDescription jd;
    
    jd.JDL             = "[  ce_id=\"cream-01.pd.infn.it:8443/cream-lsf-cream\";   X509UserProxy=\"/tmp/x509up_u501\";   edg_jobid=\"https://grid005.pd.infn.it:9000/0000000000000000000002\";  VirtualOrganisation=\"infngrid\"; LB_sequence_code = \"UI=000003:NS=0000000003:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000\";  executable=\"/bin/sleep\"; arguments=\"30\";]";
    jd.delegationId    = new string("pippo");
    jd.delegationProxy = new string("");
    jd.leaseId         = new string("");
    jd.autoStart       = false;
    jd.jobDescriptionId= new string("foo");
    
    REQ.jobDescriptionList.push_back( &jd );
    
    if(SOAP_OK != soap_call___CREAM__JobRegister(SOAP,
						 serviceURL.c_str(),
						 NULL,// soapAction
						 &REQ,
						 &RESP))
      {
	//	cerr << "Service didn't return SOAP_OK" <<endl;
// 	try{
// 	  glite::ce::cream_client_api::soap_proxy::ExceptionFactory::raiseException(SOAP);}
// 	catch(exception& ex) {
// 	  cerr << ex.what() << endl;
// 	}
	return 1;
      }
    
    
    if((*RESP.result.begin())->__union_JobRegisterResult == SOAP_UNION__CREAMTYPES__union_JobRegisterResult_jobId)
      cout << i << ": " << (*RESP.result.begin())->union_JobRegisterResult.jobId->id << "/" << (*RESP.result.begin())->union_JobRegisterResult.jobId->creamURL<<endl;
   
    i++;

    
    delete jd.delegationId;
    delete jd.delegationProxy;
    delete jd.leaseId;
    delete jd.jobDescriptionId;
    
    soap_destroy(SOAP); // deletes deserialized class instances (for C++ only)
    soap_end(SOAP); // removes deserialized data and clean up
    soap_done(SOAP); // detach soap runtime
    
    if(!::getenv("TEST_NO_AUTHN")) {
      if( m_ctx ) {
	glite_gsplugin_free_context( m_ctx );
	m_ctx = NULL;
      }
    }
    
    free(SOAP);
  }

}
