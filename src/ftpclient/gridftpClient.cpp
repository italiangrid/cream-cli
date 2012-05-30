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

#include "gridftpClient.h"
#include <cstdio>
#include <iostream>

#include "glite/ce/cream-client-api-c/creamApiLogger.h"

#define MAX_BUFFER_SIZE 4*1024
#define SUCCESS 0

using namespace std;

bool thread_error = false;

static globus_mutex_t  lock;
static globus_cond_t   cond;
static globus_bool_t   done;
long long              global_offset = 0;

/********************************************************************
 * done_cb:  A pointer to this function is passed to the call to
 * globus_ftp_client_put (and all the other high level transfer
 * operations).   It is called when the transfer is completely
 * finished, i.e. both the data channel and control channel exchange.
 * Here it simply sets a global variable (done) to true so the main
 * program will exit the while loop.
 ********************************************************************/
static
void
done_cb(
	void *				user_arg,
	globus_ftp_client_handle_t *	handle,
	globus_object_t *		err)
{
    glite::ce::cream_client_api::util::creamApiLogger* logger_instance =
    glite::ce::cream_client_api::util::creamApiLogger::instance();

    if(err)
    {
      CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR,
      string("done_cb() - ") + globus_object_printable_to_string(err), true, true, true));
      thread_error = true;
    }
    globus_mutex_lock(&lock);
    done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
    globus_mutex_unlock(&lock);
    return;
}

/*************************************************************************
 * data_cb: A pointer to this function is passed to the call to
 * globus_ftp_client_register_write.  It is called when the user supplied
 * buffer has been successfully transferred to the kernel.  Note that does
 * not mean it has been successfully transmitted.  In this simple version,
 * it justs reads the next block of data and calls register_write again.
 *************************************************************************/
static
void
data_cb_read(
	     void *			user_arg,
	     globus_ftp_client_handle_t *handle,
	     globus_object_t *		err,
	     globus_byte_t *		buffer,
	     globus_size_t		length,
	     globus_off_t		offset,
	     globus_bool_t		eof)
{
  glite::ce::cream_client_api::util::creamApiLogger* logger_instance =
    glite::ce::cream_client_api::util::creamApiLogger::instance();
  
  if(err)
    {
      CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR,
					  string("data_cb_read() - ") 
					  + globus_object_printable_to_string(err), 
					  true, true, true));
      thread_error = true;
      return;
    }
  else
    {
      if(!eof)
	{
	  FILE *fd = (FILE *) user_arg;
	  int rc;
	  rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
	  
	  if (ferror(fd) != SUCCESS)
	    {
	      CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR, 
						  string("Read error in function data_cb; error = [")
						  +strerror(errno)+"]", true, true, true));
	      //printf("Read error in function data_cb; errno = %d\n", errno);
	      return;
	    }
	  globus_ftp_client_register_write(
					   handle,
					   buffer,
					   rc,
					   global_offset,
					   feof(fd) != SUCCESS,
					   data_cb_read,
					   (void *) fd);
	  global_offset += rc;
	} /* if(!eof) */
      else
	{
	  //printf("*** Freeing buffer\n");
	  globus_libc_free(buffer);
	}
      
    } /* else */
  return;
} /* data_cb_read */

//__________________________________________________________________________________________
ftpclient::ftpclient( const char* destination_url, const int num_streams )
  : m_server_url( destination_url ), m_valid( false ), m_num_streams( num_streams )
{
//   globus_ftp_client_operationattr_t       attr;
//   globus_ftp_client_handleattr_t          handle_attr;
//   //globus_result_t                         result;
//   globus_ftp_control_parallelism_t        parallelism;
//   globus_ftp_control_layout_t             layout;

  thread_error = false;
  global_offset = 0;

  glite::ce::cream_client_api::util::creamApiLogger* logger_instance =
    glite::ce::cream_client_api::util::creamApiLogger::instance();

  /*********************************************************************
   * Initialize the module, handleattr, operationattr, and client handle
   * This has to be done EVERY time you use the client library
   * (if you don't use attrs, you don't need to initialize them and can
   * pass NULL in the parameter list)
   * The mutex and cond are theoretically optional, but highly recommended
   * because they will make the code work correctly in a threaded build.
   *
   * NOTE: It is possible for each of the initialization calls below to
   * fail and we should be checking for errors.  To keep the code simple
   * and clean we are not.  See the error checking after the call to
   * globus_ftp_client_put for an example of how to handle errors in
   * the client library.
   *********************************************************************/

  globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
  globus_mutex_init(&lock, GLOBUS_NULL);
  globus_cond_init(&cond, GLOBUS_NULL);
  globus_ftp_client_handleattr_init(&m_handle_attr);
  globus_ftp_client_operationattr_init(&m_attr);

  /************************************************************************
   * Set any desired attributes, in this case we are using parallel streams
   ************************************************************************/

  m_parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
  m_parallelism.fixed.size = num_streams;
  m_layout.mode = GLOBUS_FTP_CONTROL_STRIPING_BLOCKED_ROUND_ROBIN;
  m_layout.round_robin.block_size = 64*1024;
  globus_ftp_client_operationattr_set_mode(
					   &m_attr,
					   GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
  globus_ftp_client_operationattr_set_parallelism(&m_attr,
						  &m_parallelism);

  globus_ftp_client_operationattr_set_layout(&m_attr,
					     &m_layout);

  globus_ftp_client_handle_init(&m_handle,  &m_handle_attr);


  /********************************************************************
   * globus_ftp_client_put starts the protocol exchange on the control
   * channel.  Note that this does NOT start moving data over the data
   * channel
   *******************************************************************/
//  done = GLOBUS_FALSE;

  //bool PUT_RESULT = true;

//   CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::DEBUG,
// 				      string("ftpclient::initiate() - dst=[")
// 				      + destination_url + "]", 
// 				      true,true,true));

  globus_ftp_client_handle_cache_url_state( &m_handle, m_server_url );

/*
  result = globus_ftp_client_put(&handle,
				 destination_url,
				 &attr,
				 GLOBUS_NULL,
				 done_cb,
				 0);


  if(result != GLOBUS_SUCCESS)
    {
      globus_object_t * err;
      err = globus_error_get(result);
      CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR,
					  string("globus_ftp_client_put() - ") 
					  + globus_object_printable_to_string(err), 
					  true, true, true));
      done = GLOBUS_TRUE;
      PUT_RESULT = false;
      return false;
    }
*/
  //return true;
}

//__________________________________________________________________________________________
ftpclient::~ftpclient( )
{
  globus_ftp_client_handle_flush_url_state( &m_handle, m_server_url );
}

//__________________________________________________________________________________________
bool ftpclient::put(const char* src, const char* dst ) 
{
  
  int   i;

  thread_error = false;
  global_offset = 0;
  globus_byte_t * buffer;

  glite::ce::cream_client_api::util::creamApiLogger* logger_instance =
    glite::ce::cream_client_api::util::creamApiLogger::instance();

  globus_result_t result = globus_ftp_client_put(&m_handle,
				 dst,
				 &m_attr,
				 GLOBUS_NULL,
				 done_cb,
				 0);

  done = GLOBUS_FALSE;
  
  if(result != GLOBUS_SUCCESS)
    {
      globus_object_t * err;
      err = globus_error_get(result);
      CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR,
					  string("globus_ftp_client_put() - ") 
					  + globus_object_printable_to_string(err), 
					  true, true, true));
      done = GLOBUS_TRUE;
      //PUT_RESULT = false;
      return false;
    }

  FILE* fd;
  //bool PUT_RESULT = true;
  
      fd = fopen( src, "r" );
      
      if(!fd) {
	CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR, 
					    "ftpclient::put - fopen() call failed", 
					    true, true, true));
	return false;
      } 
      
      int rc;
      
      /**************************************************************
       * This is where the data movement over the data channel is initiated.
       * You read a buffer, and call register_write.  This is an asynch
       * call which returns immediately.  When it is finished writing
       * the buffer, it calls the data callback (defined above) which
       * reads another buffer and calls register_write again.
       * The data callback will also indicate when you have hit eof
       * Note that eof on the data channel does not mean the control
       * channel protocol exchange is complete.  This is indicated by
       * the done callback being called.
       *
       * NOTE: The for loop is present BECAUSE of the parallelism, but
       * it is not CAUSING the parallelism.  The parallelism is hidden
       * inside the client library.  This for loop simply insures that
       * we have sufficient buffers queued up so that we don't have
       * TCP steams sitting idle.
       *************************************************************/
      for (i = 0; i< m_num_streams && feof(fd) == SUCCESS; i++)
	{
	  buffer = (globus_byte_t*)malloc(MAX_BUFFER_SIZE);
	  rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
	  globus_ftp_client_register_write(
					   &m_handle,
					   buffer,
					   rc,
					   global_offset,
					   feof(fd) != SUCCESS,
					   data_cb_read,
					   (void *) fd);
	  global_offset += rc;
	}
      
      
      
      /*********************************************************************
       * The following is a standard thread construct.  The while loop is
       * required because pthreads may wake up arbitrarily.  In non-threaded
       * code, cond_wait becomes globus_poll and it sits in a loop using
       * CPU to wait for the callback.  In a threaded build, cond_wait would
       * put the thread to sleep
       *********************************************************************/
      globus_mutex_lock(&lock);
      while(!done)
	{
	  globus_cond_wait(&cond, &lock);
	}
      globus_mutex_unlock(&lock);
      
      /**********************************************************************
       * Since done has been set to true, the done callback has been called.
       * The transfer is now completely finished (both control channel and
       * data channel).  Now, Clean up and go home
       **********************************************************************/
      
      fclose(fd);
      if(thread_error) return false;
    
  
  return true;//PUT_RESULT;
}


//__________________________________________________________________________________________
// bool ftpclient::get(const string& src, const string& dst, const int& num_streams) {

//     globus_ftp_client_handle_t              handle;
//     globus_ftp_client_operationattr_t       attr;
//     globus_ftp_client_handleattr_t          handle_attr;
//     globus_byte_t *                         buffer;
//     globus_result_t                         result;
//     globus_ftp_control_parallelism_t        parallelism;
//     globus_ftp_control_layout_t             layout;
//     int                                     i;

//     thread_error = false;
//     global_offset = 0;

//     glite::ce::cream_client_api::util::creamApiLogger* logger_instance =
//     glite::ce::cream_client_api::util::creamApiLogger::instance();

//   /*********************************************************************
//      * Initialize the module, handleattr, operationattr, and client handle
//      * This has to be done EVERY time you use the client library
//      * (if you don't use attrs, you don't need to initialize them and can
//      * pass NULL in the parameter list)
//      * The mutex and cond are theoretically optional, but highly recommended
//      * because they will make the code work correctly in a threaded build.
//      *
//      * NOTE: It is possible for each of the initialization calls below to
//      * fail and we should be checking for errors.  To keep the code simple
//      * and clean we are not.  See the error checking after the call to
//      * globus_ftp_client_put for an example of how to handle errors in
//      * the client library.
//      *********************************************************************/

//     globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
//     globus_mutex_init(&lock, GLOBUS_NULL);
//     globus_cond_init(&cond, GLOBUS_NULL);
//     globus_ftp_client_handleattr_init(&handle_attr);
//     globus_ftp_client_operationattr_init(&attr);

//     /************************************************************************
//      * Set any desired attributes, in this case we are using parallel streams
//      ************************************************************************/

//     parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
//     parallelism.fixed.size = num_streams;
//     layout.mode = GLOBUS_FTP_CONTROL_STRIPING_BLOCKED_ROUND_ROBIN;
//     layout.round_robin.block_size = 64*1024;
//     globus_ftp_client_operationattr_set_mode(
//         &attr,
//         GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
//     globus_ftp_client_operationattr_set_parallelism(&attr,
// 					            &parallelism);

//     globus_ftp_client_operationattr_set_layout(&attr,
// 				               &layout);

//     globus_ftp_client_handle_init(&handle,  &handle_attr);


//     /********************************************************************
//      * globus_ftp_client_put starts the protocol exchange on the control
//      * channel.  Note that this does NOT start moving data over the data
//      * channel
//      *******************************************************************/
//     done = GLOBUS_FALSE;

//     FILE* fd = fopen(dst.c_str(), "w");

//     bool GET_RESULT = true;

//     if(!fd) {
//       CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR, "fopen() call failed", true,
//       true, true));
//       return false;
//     }

//     CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::DEBUG,
// 					string("ftpclient::get() - dst=[")
// 					+ dst + "]", true,true,true));

//     result = globus_ftp_client_get(&handle,
// 				   src.c_str(),
// 				   &attr,
// 				   GLOBUS_NULL,
// 				   done_cb,
// 				   0);

//     if(result != GLOBUS_SUCCESS)
//     {
// 	globus_object_t * err;
// 	err = globus_error_get(result);
// 	//fprintf(stderr, "%s", globus_object_printable_to_string(err));
// 	CREAM_SAFE_LOG(logger_instance->log(log4cpp::Priority::ERROR,
// 	string("globus_ftp_client_get() - ") + globus_object_printable_to_string(err), true, true, true));
// 	done = GLOBUS_TRUE;
// 	GET_RESULT = false;
// 	return false;
//     }
//     else
//       {
// 	int rc;
	
// 	/**************************************************************
// 	 * This is where the data movement over the data channel is initiated.
// 	 * You read a buffer, and call register_write.  This is an asynch
// 	 * call which returns immediately.  When it is finished writing
// 	 * the buffer, it calls the data callback (defined above) which
// 	 * reads another buffer and calls register_write again.
// 	 * The data callback will also indicate when you have hit eof
// 	 * Note that eof on the data channel does not mean the control
// 	 * channel protocol exchange is complete.  This is indicated by
// 	 * the done callback being called.
// 	 *
// 	 * NOTE: The for loop is present BECAUSE of the parallelism, but
// 	 * it is not CAUSING the parallelism.  The parallelism is hidden
// 	 * inside the client library.  This for loop simply insures that
// 	 * we have sufficient buffers queued up so that we don't have
// 	 * TCP steams sitting idle.
// 	 *************************************************************/
// 	for (i = 0; i< num_streams; i++)
// 	  {
// 	    buffer = (globus_byte_t*)malloc(MAX_BUFFER_SIZE);
// 	    //rc = fread(buffer, 1, MAX_BUFFER_SIZE, fd);
// 	    globus_ftp_client_register_read(
// 					    &handle,
// 					    buffer,
// 					    MAX_BUFFER_SIZE,
// 					    data_cb_write,
// 					    (void *) fd);
// 	    global_offset += rc;
// 	  }
	
//       }

//     /*********************************************************************
//      * The following is a standard thread construct.  The while loop is
//      * required because pthreads may wake up arbitrarily.  In non-threaded
//      * code, cond_wait becomes globus_poll and it sits in a loop using
//      * CPU to wait for the callback.  In a threaded build, cond_wait would
//      * put the thread to sleep
//      *********************************************************************/
//     globus_mutex_lock(&lock);
//     while(!done)
//       {
// 	globus_cond_wait(&cond, &lock);
//       }
//     globus_mutex_unlock(&lock);
    
//     /**********************************************************************
//      * Since done has been set to true, the done callback has been called.
//      * The transfer is now completely finished (both control channel and
//      * data channel).  Now, Clean up and go home
//      **********************************************************************/
    
//     fclose(fd);
//     if(thread_error) return false;
//     return GET_RESULT;
// }
