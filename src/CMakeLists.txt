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

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSOAP gsoapssl++)
pkg_check_modules(GSOAP_PLUGIN glite-security-gsoap-plugin++)
pkg_check_modules(GLITE_JOBID emi-jobid)
pkg_check_modules(GLITE_CREAM_SOAP cream-client-api-soap)
pkg_check_modules(GLITE_CREAM_UTIL cream-client-api-util)
pkg_check_modules(GLITE_CREAM_ES es-client-api-c)
pkg_check_modules(ARES libcares)
pkg_check_modules(GLITE_JDL jdl-api-cpp)
pkg_check_modules(GLITE_WMSUTILS_EXCEPTION jobman-exception)
pkg_check_modules(GLITE_WMSUTILS_CLASSADS classad-utils)
pkg_check_modules(GRIDSITE_OPENSSL emi-gridsite-openssl)
pkg_check_modules(GLOBUS_COMMON globus-common)
pkg_check_modules(GLOBUS_CORE globus-core)
pkg_check_modules(GLOBUS_CALLOUT globus-callout)
pkg_check_modules(GLOBUS_OPENSSL_MODULE globus-openssl-module)
pkg_check_modules(GLOBUS_GSI_CALLBACK globus-gsi-callback)
pkg_check_modules(GLOBUS_GSI_CERT_UTILS globus-gsi-cert-utils)
pkg_check_modules(GLOBUS_GSI_CREDENTIAL globus-gsi-credential)
pkg_check_modules(GLOBUS_GSI_OPENSSL_ERROR globus-gsi-openssl-error)
pkg_check_modules(GLOBUS_GSI_PROXY_CORE globus-gsi-proxy-core)
pkg_check_modules(GLOBUS_GSI_PROXY_SSL globus-gsi-proxy-ssl)
pkg_check_modules(GLOBUS_GSI_SYSCONFIG globus-gsi-sysconfig)
pkg_check_modules(GLOBUS_GSSAPI_ERROR globus-gssapi-error)
pkg_check_modules(GLOBUS_GSSAPI_GSI globus-gssapi-gsi)
pkg_check_modules(GLOBUS_GSS_ASSIST globus-gss-assist)
pkg_check_modules(GLOBUS_FTP_CLIENT globus-ftp-client)
pkg_check_modules(GLOBUS_FTP_CONTROL globus-ftp-control)
pkg_check_modules(LOG4CPP log4cpp)
pkg_check_modules(VOMS voms-2.0)
pkg_check_modules(GRIDSITE_OPENSSL emi-gridsite-openssl)
pkg_check_modules(CLASSAD emi-classad-ns)

if( NOT DEFINED BOOST_LIBRARYDIR AND IS_DIRECTORY "/usr/lib64" )
    set(BOOST_LIBRARYDIR "/usr/lib64")
endif( NOT DEFINED BOOST_LIBRARYDIR AND IS_DIRECTORY "/usr/lib64" )

set(Boost_USE_MULTITHREADED ON)
find_package( Boost COMPONENTS date_time filesystem regex thread program_options)

add_definitions( -pipe -DWANT_NAMESPACES -DWANT_CLASSAD_NAMESPACE )

set(COMMON_LIBS ${GSOAP_LIBRARIES}
                ${GSOAP_PLUGIN_LIBRARIES}
                ${GLITE_JOBID_LIBRARIES}
                ${GLITE_CREAM_SOAP_LIBRARIES}
                ${GLITE_CREAM_UTIL_LIBRARIES}
                ${GLITE_CREAM_ES_LIBRARIES}
                ${ARES_LIBRARIES}
                ${GLITE_JDL_LIBRARIES}
                ${GLITE_WMSUTILS_EXCEPTION_LIBRARIES}
                ${GLITE_WMSUTILS_CLASSADS_LIBRARIES}
                ${GLOBUS_FTP_CLIENT_LIBRARIES}
                ${GLOBUS_FTP_CONTROL_LIBRARIES}
                ${CLASSAD_LIBRARIES}
                ${VOMS_LIBRARIES}
                ${LOG4CPP_LIBRARIES}
                ${GRIDSITE_OPENSSL_LIBRARIES}
                ${Boost_LIBRARIES})

include_directories(${CLASSAD_INCLUDE_DIRS}
                    ${VOMS_INCLUDE_DIRS}
                    ${GSOAP_INCLUDE_DIRS}
                    ${LOG4CPP_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${GLOBUS_CORE_INCLUDE_DIRS})

link_directories(${CMAKE_BINARY_DIR}/util
                 ${CMAKE_BINARY_DIR}/services
                 ${CMAKE_BINARY_DIR}/ftpclient
                 ${VOMS_LIBRARY_DIRS}
                 ${GSOAP_PLUGIN_LIBRARY_DIRS})

add_executable(glite-ce-allowed-submission glite-ce-allowed-submission.cpp)
target_link_libraries(glite-ce-allowed-submission ${COMMON_LIBS}
                                                  glite_ce_cream_cli_util
                                                  glite_ce_cream_cli_services
                                                  glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-delegate-proxy glite-ce-delegate-proxy.cpp)
target_link_libraries(glite-ce-delegate-proxy ${COMMON_LIBS}
                                              glite_ce_cream_cli_util
                                              glite_ce_cream_cli_services
                                              glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-disable-submission glite-ce-disable-submission.cpp)
target_link_libraries(glite-ce-disable-submission ${COMMON_LIBS}
                                                  glite_ce_cream_cli_util
                                                  glite_ce_cream_cli_services
                                                  glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-enable-submission glite-ce-enable-submission.cpp)
target_link_libraries(glite-ce-enable-submission ${COMMON_LIBS}
                                                 glite_ce_cream_cli_util
                                                 glite_ce_cream_cli_services
                                                 glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-event-query glite-ce-event-query.cpp)
target_link_libraries(glite-ce-event-query ${COMMON_LIBS}
                                           glite_ce_cream_cli_util
                                           glite_ce_cream_cli_services
                                           glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-get-cemon-url glite-ce-get-cemon-url.cpp)
target_link_libraries(glite-ce-get-cemon-url ${COMMON_LIBS}
                                             glite_ce_cream_cli_util
                                             glite_ce_cream_cli_services
                                             glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-cancel glite-ce-job-cancel.cpp)
target_link_libraries(glite-ce-job-cancel ${COMMON_LIBS}
                                          glite_ce_cream_cli_util
                                          glite_ce_cream_cli_services
                                          glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-lease glite-ce-job-lease.cpp)
target_link_libraries(glite-ce-job-lease ${COMMON_LIBS}
                                         glite_ce_cream_cli_util
                                         glite_ce_cream_cli_services
                                         glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-list glite-ce-job-list.cpp)
target_link_libraries(glite-ce-job-list ${COMMON_LIBS}
                                        glite_ce_cream_cli_util
                                        glite_ce_cream_cli_services
                                        glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-output glite-ce-job-output.cpp)
target_link_libraries(glite-ce-job-output ${COMMON_LIBS}
                                          glite_ce_cream_cli_util
                                          glite_ce_cream_cli_services
                                          glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-purge glite-ce-job-purge.cpp)
target_link_libraries(glite-ce-job-purge ${COMMON_LIBS}
                                         glite_ce_cream_cli_util
                                         glite_ce_cream_cli_services
                                         glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-resume glite-ce-job-resume.cpp)
target_link_libraries(glite-ce-job-resume ${COMMON_LIBS}
                                          glite_ce_cream_cli_util
                                          glite_ce_cream_cli_services
                                          glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-status glite-ce-job-status.cpp)
target_link_libraries(glite-ce-job-status ${COMMON_LIBS}
                                          glite_ce_cream_cli_util
                                          glite_ce_cream_cli_services
                                          glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-submit glite-ce-job-submit.cpp)
target_link_libraries(glite-ce-job-submit ${COMMON_LIBS}
                                          glite_ce_cream_cli_util
                                          glite_ce_cream_cli_services
                                          glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-job-suspend glite-ce-job-suspend.cpp)
target_link_libraries(glite-ce-job-suspend ${COMMON_LIBS}
                                           glite_ce_cream_cli_util
                                           glite_ce_cream_cli_services
                                           glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-proxy-renew glite-ce-proxy-renew.cpp)
target_link_libraries(glite-ce-proxy-renew ${COMMON_LIBS}
                                           glite_ce_cream_cli_util
                                           glite_ce_cream_cli_services
                                           glite_ce_cream_cli_ftpclient)

add_executable(glite-ce-service-info glite-ce-service-info.cpp)
target_link_libraries(glite-ce-service-info ${COMMON_LIBS}
                                            glite_ce_cream_cli_util
                                            glite_ce_cream_cli_services
                                            glite_ce_cream_cli_ftpclient)


install(TARGETS glite-ce-allowed-submission
                glite-ce-delegate-proxy
                glite-ce-disable-submission
                glite-ce-enable-submission
                glite-ce-event-query
                glite-ce-get-cemon-url
                glite-ce-job-cancel
                glite-ce-job-lease
                glite-ce-job-list
                glite-ce-job-output
                glite-ce-job-purge
                glite-ce-job-resume
                glite-ce-job-status
                glite-ce-job-submit
                glite-ce-job-suspend
                glite-ce-proxy-renew
                glite-ce-service-info
        RUNTIME DESTINATION usr/bin)


add_subdirectory(util)
add_subdirectory(services)
add_subdirectory(ftpclient)


