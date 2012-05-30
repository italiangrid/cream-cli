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

find_file(DB_MANPAGES_STYLESHEET "docbook.xsl"
          PATHS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages"
                "/usr/share/sgml/docbook/xsl-stylesheets/manpages")
if( NOT DB_MANPAGES_STYLESHEET )
    message(FATAL_ERROR "Cannot find stylesheet for man pages")
endif( NOT DB_MANPAGES_STYLESHEET )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-allowed-submission.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-allowed-submission.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-allowed-submission.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-allowed-submission.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-allowed-submission.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-delegate-proxy.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-delegate-proxy.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-delegate-proxy.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-delegate-proxy.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-delegate-proxy.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-disable-submission.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-disable-submission.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-disable-submission.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-disable-submission.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-disable-submission.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-enable-submission.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-enable-submission.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-enable-submission.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-enable-submission.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-enable-submission.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-get-cemon-url.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-get-cemon-url.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-get-cemon-url.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-get-cemon-url.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-get-cemon-url.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-cancel.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-cancel.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-cancel.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-cancel.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-cancel.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-list.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-list.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-list.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-list.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-list.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-output.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-output.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-output.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-output.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-output.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-purge.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-purge.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-purge.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-purge.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-purge.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-resume.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-resume.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-resume.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-resume.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-resume.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-status.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-status.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-status.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-status.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-status.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-submit.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-submit.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-submit.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-submit.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-submit.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-suspend.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-suspend.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-suspend.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-job-suspend.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-suspend.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-proxy-renew.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-proxy-renew.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-proxy-renew.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-proxy-renew.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-proxy-renew.1)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-service-info.1.gz
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-service-info.xml
                   COMMAND xsltproc -o ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-service-info.1
                                    ${DB_MANPAGES_STYLESHEET} 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/glite-ce-service-info.xml
                   COMMAND gzip -f ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-service-info.1)

add_custom_target(manpages ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-allowed-submission.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-delegate-proxy.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-disable-submission.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-enable-submission.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-get-cemon-url.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-cancel.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-list.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-output.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-purge.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-resume.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-status.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-submit.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-job-suspend.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-proxy-renew.1.gz
                                       ${CMAKE_CURRENT_BINARY_DIR}/glite-ce-service-info.1.gz)


install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
        DESTINATION usr/share/man/man1
        FILES_MATCHING PATTERN "*.1.gz"
        PATTERN "CMakeFiles" EXCLUDE)


