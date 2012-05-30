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

#include <vector>
#include <string>
#include <globus_ftp_client.h>

#define MAX_BUFFER_SIZE 4*1024
//#define SUCCESS 0

// class gridftpClient  {
//   public:
//     virtual bool put(const std::string&, const std::string&, const int&) = 0;
// };

/*extern "C" {
gridftpClient* maker() {
	return new gridftpClient;
};
}*/

class ftpclient // : public gridftpClient 
{
  globus_ftp_client_operationattr_t  m_attr;
  globus_ftp_client_handle_t         m_handle;
  globus_ftp_client_handleattr_t     m_handle_attr;
  globus_ftp_control_parallelism_t   m_parallelism;
  globus_ftp_control_layout_t        m_layout;
  const char*                        m_server_url;
  bool                               m_valid;
  int                                m_num_streams;

public:

   ftpclient( const char*, const int );
   ~ftpclient( ); 
   bool put(const char*, const char*);
};
