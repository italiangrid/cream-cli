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

include_directories(${ClassAd_INCLUDE_DIR}
                    ${VOMS_INCLUDE_DIRS}
                    ${GLITE_CREAM_SOAP_INCLUDE_DIRS}
                    ${GLITE_CREAM_UTIL_INCLUDE_DIRS}
                    ${GLITE_CREAM_ES_INCLUDE_DIRS}
                    ${GSOAP_INCLUDE_DIRS}
                    ${LOG4CPP_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${GLOBUS_CORE_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/src)

add_library(glite_ce_cream_cli_services SHARED 
                     cli_service_allowed_submission.cpp
                     cli_service.cpp
                     cli_service_delegate_proxy.cpp
                     cli_service_disable_submission.cpp
                     cli_service_enable_submission.cpp
                     cli_service_event_query.cpp
                     cli_service_get_cemon_url.cpp
                     cli_service_jobcancel.cpp
                     cli_service_joblease.cpp
                     cli_service_joblist.cpp
                     cli_service_jobpurge.cpp
                     cli_service_jobresume.cpp
                     cli_service_jobstatus.cpp
                     cli_service_jobsubmit.cpp
                     cli_service_jobsuspend.cpp
                     cli_service_proxy_renew.cpp
                     cli_service_service_info.cpp)

set_target_properties(glite_ce_cream_cli_services PROPERTIES VERSION 0.0.0 SOVERSION 0)

target_link_libraries(glite_ce_cream_cli_services ${COMMON_LIBS})

install(TARGETS glite_ce_cream_cli_services
        LIBRARY DESTINATION usr/${LIBSUBDIR})

