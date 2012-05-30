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

/*
purify g++ -o test_register_api -g -O0 -Iftpclient -I../../repository/org.glite/org.glite.security.voms/1.8.1/slc4_ia32_gcc346/include -I../../repository/org.glite/org.glite.wms-utils.exception/3.1.3/slc4_ia32_gcc346/include -Iutil -I ../../org.glite.ce.cream-client-api-c/build/src/autogen -I../../org.glite.ce.cream-client-api-c/interface -I../../repository/externals/gsoap/2.7.6b/slc4_ia32_gcc346/include -I../../repository/externals/log4cpp/0.3.4b/slc4_ia32_gcc346/include -I../../repository/org.glite/org.glite.security.gsoap-plugin/1.5.2/slc4_ia32_gcc346/include -I../../repository/vdt/globus/4.0.3-VDT-1.6.0/slc4_ia32_gcc346/include/gcc32dbgpthr -I../../repository/org.glite/org.gridsite.core/1.1.18/slc4_ia32_gcc346/include -I ../../repository/externals/classads/0.9.8/slc4_ia32_gcc346/include -I../../repository/org.glite/org.glite.wms-utils.jobid/3.1.3/slc4_ia32_gcc346/include -I ../../repository/org.glite/org.glite.jdl.api-cpp/3.1.13/slc4_ia32_gcc346/include -DWANT_NAMESPACES -UHAVE_CONFIG_H test_register_api.cpp ../../repository/externals/gsoap/2.7.6b/slc4_ia32_gcc346/src/stdsoap2.cpp ../../org.glite.ce.cream-client-api-c/build/src/autogen/CREAM_CLIENTC.cpp ../../org.glite.ce.cream-client-api-c/build/src/autogen/CREAM_CLIENTClient.cpp util/jdlHelper.cpp util/cliUtils.cpp ftpclient/gridftpClient.cpp *.o -L ../../repository/externals/log4cpp/0.3.4b/slc4_ia32_gcc346/lib -llog4cpp -L../../repository/externals/boost/1.32.0/slc4_ia32_gcc346/lib -lboost_date_time -lboost_filesystem -lboost_program_options -lboost_regex -lboost_thread -L../../repository/org.glite/org.glite.security.gsoap-plugin/1.5.2/slc4_ia32_gcc346/lib -lglite_security_gsoap_plugin_276b_gcc32dbgpthr -L../../repository/org.glite/org.glite.wms-utils.exception/3.1.3/slc4_ia32_gcc346/lib -lglite_wmsutils_exception -L../../repository/org.glite/org.glite.jdl.api-cpp/3.1.13/slc4_ia32_gcc346/lib -lglite_jdl_api_cpp -L../../repository/org.glite/org.glite.wms-utils.jobid/3.1.3/slc4_ia32_gcc346/lib -lglite_wmsutils_jobid -lglite_wmsutils_cjobid -L../../repository/org.glite/org.glite.wms-utils.classad/3.1.6/slc4_ia32_gcc346/lib -lglite_wmsutils_classads -L../../repository/externals/classads/0.9.8/slc4_ia32_gcc346/lib/ -lclassad -L../../repository/org.glite/org.gridsite.core/1.1.18/slc4_ia32_gcc346/lib -lgridsite -L ../../repository/vdt/globus/4.0.3-VDT-1.6.0/slc4_ia32_gcc346/lib -lxml2_gcc32dbg -L../../repository/vdt/globus/4.0.3-VDT-1.6.0/slc4_ia32_gcc346/lib -lglobus_ftp_client_gcc32dbgpthr -L../../stage/lib -lvomsapi_gcc32dbgpthr


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
#include "glite/ce/cream-client-api-c/JobDescriptionWrapper.h"
#include "glite/ce/cream-client-api-c/ConfigurationManager.h"
#include "glite/ce/cream-client-api-c/JobPropertyWrapper.h"
#include "glite/ce/cream-client-api-c/CreamProxyFactory.h"
#include "glite/ce/cream-client-api-c/JobFilterWrapper.h"
#include "glite/ce/cream-client-api-c/CreamProxy_Impl.h"
#include "glite/ce/cream-client-api-c/creamApiLogger.h"
#include "glite/ce/cream-client-api-c/ResultWrapper.h"
#include "glite/ce/cream-client-api-c/JobIdWrapper.h"
#include "glite/ce/cream-client-api-c/VOMSWrapper.h"
#include "glite/ce/cream-client-api-c/certUtil.h"
#include "glite/ce/cream-client-api-c/file_ex.h"
#include "glite/ce/cream-client-api-c/CEUrl.h"


using namespace std;

char *namespaces[] = {};
void soap_serialize_xsd__QName(soap*, 
                               std::basic_string<char, std::char_traits<char>, 
                               std::allocator<char> > const*) 
{}

int main(int argc, char *argv[]) {

  //  glite_gsplugin_Context m_ctx = 0;

  //  int i = 0;

  string m_certfile = "/tmp/x509up_u501";

  char *port = "8443";
  if(argv[1])
    port = argv[1];

  int ii = 0;
  cin >> ii;
  
  string JDL             = "[  ce_id=\"cream-01.pd.infn.it:8443/cream-lsf-cream\";   X509UserProxy=\"/tmp/x509up_u501\";   edg_jobid=\"https://grid005.pd.infn.it:9000/0000000000000000000002\";  VirtualOrganisation=\"infngrid\"; LB_sequence_code = \"UI=000003:NS=0000000003:WM=000000:BH=0000000000:JSS=000000:LM=000000:LRMS=000000:APP=000000\";  executable=\"/bin/sleep\"; arguments=\"30\";]";

  glite::ce::cream_client_api::soap_proxy::JobDescriptionWrapper jd(JDL, "pippo", "", "", false, "foo");
  
  glite::ce::cream_client_api::soap_proxy::AbsCreamProxy *client;
  glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayRequest reqs;
  reqs.push_back( &jd );
  glite::ce::cream_client_api::soap_proxy::AbsCreamProxy::RegisterArrayResult resp;

  int i = 0;

  resp.clear();
  string serviceURL;
  if(!::getenv("CREAM_CLIENT_NO_AUTHN")) {
    serviceURL = "https";
  } else {
    serviceURL = "http";
  }
  
  serviceURL += "://localhost:" + string(port) + "/ce-cream/services/CREAM2";

  resp.clear();
  
  while(1) {
    cout << "Iteration #" << i++<<endl;
    client = glite::ce::cream_client_api::soap_proxy::CreamProxyFactory::make_CreamProxyRegister( &reqs, &resp, 30 );
    client->setCredential( m_certfile );
    client->execute( serviceURL );
    cout << resp["foo"].get<1>().getCreamURL() << endl;
    usleep(5000);
    delete client;
  }
  
}
