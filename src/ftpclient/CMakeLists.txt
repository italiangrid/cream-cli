#
#  Copyright (c) Members of the EGEE Collaboration. 2010.
#  See http://www.eu-egee.org/partners/ for details on the copyright
#  holders.
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#     http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#  implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  

include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${VOMS_INCLUDE_DIRS}
                    ${GSOAP_INCLUDE_DIRS}
                    ${LOG4CPP_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${GLOBUS_CORE_INCLUDE_DIRS})

add_library(glite_ce_cream_cli_ftpclient SHARED gridftpClient.cpp)

set_target_properties(glite_ce_cream_cli_ftpclient PROPERTIES VERSION 0.0.0 SOVERSION 0)

target_link_libraries(glite_ce_cream_cli_ftpclient ${COMMON_LIBS})

install(TARGETS glite_ce_cream_cli_ftpclient
        LIBRARY DESTINATION usr/${LIBSUBDIR})
