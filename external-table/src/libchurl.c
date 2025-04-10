/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "libchurl.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/guc.h"

#if PG_VERSION_NUM >= 130000
#include "common/jsonapi.h"
#include "utils/jsonfuncs.h"
#else
#include "utils/jsonapi.h"
#endif

/* include libcurl without typecheck.
 * This allows wrapping curl_easy_setopt to be wrapped
 * for readability. O/w an error is generated when anything
 * other than the expected type is given as parameter
 */
#define CURL_DISABLE_TYPECHECK
#include <curl/curl.h>
#undef CURL_DISABLE_TYPECHECK

/*
 * internal buffer for libchurl internal context
 */
typedef struct
{
	char	   *ptr;
	int			max;
	int			bot,
				top;

} churl_buffer;

/*
 * internal context of libchurl
 */
typedef struct
{
	/* curl easy API handle */
	CURL	   *curl_handle;

	/*
	 * curl multi API handle used to allow non-blocking callbacks
	 */
	CURLM	   *multi_handle;

	/*
	 * curl API puts internal errors in this buffer used for error reporting
	 */
	char		curl_error_buffer[CURL_ERROR_SIZE];

	/*
	 * perform() (libcurl API) lets us know if the session is over using this
	 * int
	 */
	int			curl_still_running;

	/* internal buffer for download */
	churl_buffer *download_buffer;

	/* internal buffer for upload */
	churl_buffer *upload_buffer;

	/*
	 * holds http error code returned from remote server
	 */
	char	   *last_http_reponse;

	/* true on upload, false on download */
	bool		upload;
} churl_context;

/*
 * holds http header properties
 */
typedef struct
{
	struct curl_slist *headers;
} churl_settings;

#if PG_VERSION_NUM < 130000
/* the null action object used for pure validation */
static JsonSemAction nullSemAction =
{
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL
};
#endif

churl_context *churl_new_context(void);
static void		create_curl_handle(churl_context *context);
static void		set_curl_option(churl_context *context, CURLoption option, const void *data);
static size_t	read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
static void		setup_multi_handle(churl_context *context);
static void		multi_perform(churl_context *context);
static bool		internal_buffer_large_enough(churl_buffer *buffer, size_t required);
static void		flush_internal_buffer(churl_context *context);
static char	   *get_dest_address(CURL * curl_handle);
static void		enlarge_internal_buffer(churl_buffer *buffer, size_t required);
static void		finish_upload(churl_context *context);
static void		cleanup_curl_handle(churl_context *context);
static void		multi_remove_handle(churl_context *context);
static void		cleanup_internal_buffer(churl_buffer *buffer);
static void		churl_cleanup_context(churl_context *context);
static size_t	write_callback(char *buffer, size_t size, size_t nitems, void *userp);
static void		fill_internal_buffer(churl_context *context, int want);
static void		churl_headers_set(churl_context *context, CHURL_HEADERS settings);
static void		check_response_status(churl_context *context);
static void		check_response_code(churl_context *context);
static void		check_response(churl_context *context);
static void		clear_error_buffer(churl_context *context);
static size_t	header_callback(char *buffer, size_t size, size_t nitems, void *userp);
static void		free_http_response(churl_context *context);
static void		compact_internal_buffer(churl_buffer *buffer);
static void		realloc_internal_buffer(churl_buffer *buffer, size_t required);
static bool		handle_special_error(long response, StringInfo err);
static char	   *get_http_error_msg(long http_ret_code, char *msg, char *curl_error_buffer, char **hint_message, char **trace_message);
static char	   *build_header_str(const char *format, const char *key, const char *value);
static bool	IsValidJson(text *json);


CHURL_HEADERS
churl_headers_init(void)
{
	churl_settings *settings = (churl_settings *) palloc0(sizeof(churl_settings));

	return (CHURL_HEADERS) settings;
}

/*
 * Build a header string, in the form of given format (e.g. "%s: %s"),
 * and populate <key> and <value> in it.
 * If value is empty, return <key>.
 */
static char *
build_header_str(const char *format, const char *key, const char *value)
{
	char	   *header_option = NULL;
	char	   *output = NULL;

	if (value == NULL)			/* the option is just a "key" */
		header_option = pstrdup(key);
	else						/* the option is a "key: value" */
	{
		StringInfoData formatter;
		initStringInfo(&formatter);

		/* Only encode custom headers */
		if (pg_strncasecmp("X-GP-", key, 5) == 0)
		{
			output = curl_easy_escape(NULL, value, strlen(value));

			if (!output)
				elog(ERROR, "internal error: curl_easy_escape failed for value %s", value);

			appendStringInfo(&formatter, format, key, output);
			curl_free(output);
		}
		else
		{
			appendStringInfo(&formatter, format, key, value);
		}

		header_option = formatter.data;
	}
	return header_option;
}

void
churl_headers_append(CHURL_HEADERS headers, const char *key, const char *value)
{
	churl_settings *settings = (churl_settings *) headers;
	char	   *header_option = NULL;

	header_option = build_header_str("%s: %s", key, value);

	settings->headers = curl_slist_append(settings->headers,
										  header_option);
	pfree(header_option);
}

void
churl_headers_override(CHURL_HEADERS headers, const char *key, const char *value)
{
	churl_settings *settings = (churl_settings *) headers;
	struct curl_slist *header_cell = settings->headers;
	char	   *key_option = NULL;
	char	   *header_data = NULL;

	/* key must not be empty */
	Assert(key != NULL);

	/* key to compare with in the headers */
	key_option = build_header_str("%s:%s", key, value ? "" : NULL);

	/* find key in headers list */
	while (header_cell != NULL)
	{
		header_data = header_cell->data;

		if (strncmp(key_option, header_data, strlen(key_option)) == 0)
		{
			elog(DEBUG2, "churl_headers_override: Found existing header %s with key %s (for new value %s)",
				 header_data, key_option, value);
			break;
		}
		header_cell = header_cell->next;
	}

	if (header_cell != NULL)	/* found key */
	{
		char	   *new_data = build_header_str("%s: %s", key, value);
		char	   *old_data = header_cell->data;

		header_cell->data = strdup(new_data);
		elog(DEBUG4, "churl_headers_override: new data: %s, old data: %s", new_data, old_data);
		free(old_data);
		pfree(new_data);
	}
	else
	{
		churl_headers_append(headers, key, value);
	}

	pfree(key_option);
}

void
churl_headers_remove(CHURL_HEADERS headers, const char *key, bool has_value)
{

	churl_settings *settings = (churl_settings *) headers;
	struct curl_slist *to_del_cell = settings->headers;
	struct curl_slist *prev_cell = NULL;
	char	   *key_option = NULL;
	char	   *header_data = NULL;

	/* key must not be empty */
	Assert(key != NULL);

	/* key to compare with in the headers */
	key_option = build_header_str("%s:%s", key, has_value ? "" : NULL);

	/* find key in headers list */
	while (to_del_cell != NULL)
	{

		header_data = to_del_cell->data;

		if (strncmp(key_option, header_data, strlen(key_option)) == 0)
		{
			elog(DEBUG2, "churl_headers_remove: Found existing header %s with key %s",
				 header_data, key_option);
			break;
		}
		prev_cell = to_del_cell;
		to_del_cell = to_del_cell->next;
	}

	if (to_del_cell != NULL)	/* found key */
	{
		/* skip this cell */
		if (prev_cell != NULL)
		{
			/* not the header */
			prev_cell->next = to_del_cell->next;
		}
		else
		{
			/* remove header - make the next cell header now */
			settings->headers = to_del_cell->next;
		}

		/* remove header data and cell */
		if (to_del_cell->data)
			free(to_del_cell->data);
		free(to_del_cell);
	}
	else
	{
		elog(DEBUG2, "churl_headers_remove: No header with key %s to remove",
			 key_option);
	}

	pfree(key_option);
}

void
churl_headers_cleanup(CHURL_HEADERS headers)
{
	churl_settings *settings = (churl_settings *) headers;

	if (!settings)
		return;

	if (settings->headers)
		curl_slist_free_all(settings->headers);

	pfree(settings);
}

/*
 * debug callback for CURLOPT_DEBUGFUNCTION
 * logs debug information via postgres' logging mechanism
 */
static int
log_curl_debug(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
	static const char prefix_map[CURLINFO_END][13] = {
		"info: ",
		"header in: ",
		"header out: ",
		"data in: ",
		"data out: ",
		"data in: ",
		"data out: "
	};

	switch(type) {
	case CURLINFO_TEXT:
	case CURLINFO_HEADER_IN:
	case CURLINFO_HEADER_OUT:
		ereport(DEBUG3,
			(errmsg("curl debug - %s%s", prefix_map[type], data)));
		break;
	case CURLINFO_DATA_IN:
	case CURLINFO_DATA_OUT:
	case CURLINFO_SSL_DATA_IN:
	case CURLINFO_SSL_DATA_OUT:
		ereport(DEBUG3,
			(errmsg("curl debug - %s%zu bytes", prefix_map[type], size)));
		break;
	default:
		break;
	}

	return 0;
}

static CHURL_HANDLE
churl_init(const char *url, CHURL_HEADERS headers)
{
	churl_context *context = churl_new_context();

	create_curl_handle(context);
	clear_error_buffer(context);

/* Required for resolving localhost on some docker environments that
 * had intermittent networking issues when using pxf on HAWQ.
 */
	if (strstr(url, LocalhostIpV4) != NULL)
	{
		struct curl_slist *resolve_hosts = NULL;
		char	   *pxf_host_entry = (char *) palloc0(LOCAL_HOST_RESOLVE_STRING_MAX_LENGTH);

		snprintf(pxf_host_entry, LOCAL_HOST_RESOLVE_STRING_MAX_LENGTH, LOCAL_HOST_RESOLVE_STRING_FORMAT, get_pxf_port());
		elog(DEBUG3, "adding CURLOPT_RESOLVE with entry '%s'", pxf_host_entry);
		resolve_hosts = curl_slist_append(NULL, pxf_host_entry);
		set_curl_option(context, CURLOPT_RESOLVE, resolve_hosts);
		pfree(pxf_host_entry);
	}

	long curl_verbose = (DEBUG3 >= log_min_messages) || (DEBUG3 >= client_min_messages);

	set_curl_option(context, CURLOPT_URL, url);
	set_curl_option(context, CURLOPT_DEBUGFUNCTION, log_curl_debug);
	set_curl_option(context, CURLOPT_VERBOSE, (const void *) curl_verbose);
	set_curl_option(context, CURLOPT_ERRORBUFFER, context->curl_error_buffer);
	set_curl_option(context, CURLOPT_IPRESOLVE, (const void *) CURL_IPRESOLVE_V4);
	set_curl_option(context, CURLOPT_WRITEFUNCTION, write_callback);
	set_curl_option(context, CURLOPT_WRITEDATA, context);
	set_curl_option(context, CURLOPT_HEADERFUNCTION, header_callback);
	set_curl_option(context, CURLOPT_HEADERDATA, context);
	churl_headers_set(context, headers);

	return (CHURL_HANDLE) context;
}

CHURL_HANDLE
churl_init_upload(const char *url, CHURL_HEADERS headers)
{
	churl_context *context = churl_init(url, headers);

	context->upload = true;

	set_curl_option(context, CURLOPT_POST, (const void *) true);
	set_curl_option(context, CURLOPT_READFUNCTION, read_callback);
	set_curl_option(context, CURLOPT_READDATA, context);
	churl_headers_append(headers, "Content-Type", "application/octet-stream");
	churl_headers_append(headers, "Transfer-Encoding", "chunked");
	churl_headers_append(headers, "Expect", "100-continue");

	setup_multi_handle(context);
	return (CHURL_HANDLE) context;
}

CHURL_HANDLE
churl_init_download(const char *url, CHURL_HEADERS headers)
{
	churl_context *context = churl_init(url, headers);

	context->upload = false;

	setup_multi_handle(context);
	return (CHURL_HANDLE) context;
}

void
churl_download_restart(CHURL_HANDLE handle, const char *url, CHURL_HEADERS headers)
{
	churl_context *context = (churl_context *) handle;

	Assert(!context->upload);

	/* halt current transfer */
	multi_remove_handle(context);

	/* set a new url */
	set_curl_option(context, CURLOPT_URL, url);

	/* set headers again */
	if (headers)
		churl_headers_set(context, headers);

	/* restart */
	setup_multi_handle(context);
}

/*
 * upload
 */
size_t
churl_write(CHURL_HANDLE handle, const char *buf, size_t bufsize)
{
	churl_context *context = (churl_context *) handle;
	churl_buffer *context_buffer = context->upload_buffer;

	Assert(context->upload);

	if (!internal_buffer_large_enough(context_buffer, bufsize))
	{
		flush_internal_buffer(context);
		if (!internal_buffer_large_enough(context_buffer, bufsize))
			enlarge_internal_buffer(context_buffer, bufsize);
	}

	memcpy(context_buffer->ptr + context_buffer->top, buf, bufsize);
	context_buffer->top += bufsize;

	return bufsize;
}

/*
 * check that connection is ok, read a few bytes and check response.
 */
void
churl_read_check_connectivity(CHURL_HANDLE handle)
{
	churl_context *context = (churl_context *) handle;

	Assert(!context->upload);

	fill_internal_buffer(context, 1);
	check_response(context);
}

/*
 * download
 */
size_t
churl_read(CHURL_HANDLE handle, char *buf, size_t max_size)
{
	int			n = 0;
	churl_context *context = (churl_context *) handle;
	churl_buffer *context_buffer = context->download_buffer;

	Assert(!context->upload);

	fill_internal_buffer(context, max_size);

	n = context_buffer->top - context_buffer->bot;

	/*------
	 * TODO: this means we are done. Should we do something with it?
	 * if (n == 0 && !context->curl_still_running)
	 * context->eof = true;
	 *------
	 */

	if (n > max_size)
		n = max_size;

	memcpy(buf, context_buffer->ptr + context_buffer->bot, n);
	context_buffer->bot += n;

	return n;
}

void
churl_cleanup(CHURL_HANDLE handle, bool after_error)
{
	churl_context *context = (churl_context *) handle;

	if (!context)
		return;

	/* don't try to read/write data after an error */
	if (!after_error)
	{
		if (context->upload)
			finish_upload(context);
		else
			churl_read_check_connectivity(handle);
	}

	cleanup_curl_handle(context);
	cleanup_internal_buffer(context->download_buffer);
	cleanup_internal_buffer(context->upload_buffer);
	churl_cleanup_context(context);
}

churl_context *
churl_new_context()
{
	churl_context *context = palloc0(sizeof(churl_context));

	context->download_buffer = palloc0(sizeof(churl_buffer));
	context->upload_buffer = palloc0(sizeof(churl_buffer));
	return context;
}

static void
clear_error_buffer(churl_context *context)
{
	if (!context)
		return;
	context->curl_error_buffer[0] = 0;
}

static void
create_curl_handle(churl_context *context)
{
	context->curl_handle = curl_easy_init();
	if (!context->curl_handle)
		elog(ERROR, "internal error: curl_easy_init failed");
}

static void
set_curl_option(churl_context *context, CURLoption option, const void *data)
{
	int			curl_error;

	if (CURLE_OK != (curl_error = curl_easy_setopt(context->curl_handle, option, data)))
		elog(ERROR, "internal error: curl_easy_setopt %d error (%d - %s)",
			 option, curl_error, curl_easy_strerror(curl_error));
}

/*
 * Called by libcurl perform during an upload.
 * Copies data from internal buffer to libcurl's buffer.
 * Once zero is returned, libcurl knows upload is over
 */
static size_t
read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	churl_context *context = (churl_context *) userdata;
	churl_buffer *context_buffer = context->upload_buffer;

	int			written = Min(size * nmemb, context_buffer->top - context_buffer->bot);

	memcpy(ptr, context_buffer->ptr + context_buffer->bot, written);
	context_buffer->bot += written;

	return written;
}

/*
 * Setups the libcurl multi API
 */
static void
setup_multi_handle(churl_context *context)
{
	int			curl_error;

	/* Create multi handle on first use */
	if (!context->multi_handle)
		if (!(context->multi_handle = curl_multi_init()))
			elog(ERROR, "internal error: curl_multi_init failed");

	/* add the easy handle to the multi handle */
	/* don't blame me, blame libcurl */
	if (CURLM_OK != (curl_error = curl_multi_add_handle(context->multi_handle, context->curl_handle)))
		if (CURLM_CALL_MULTI_PERFORM != curl_error)
			elog(ERROR, "internal error: curl_multi_add_handle failed (%d - %s)",
				 curl_error, curl_easy_strerror(curl_error));

	multi_perform(context);
}

/*
 * Does the real work. Causes libcurl to do
 * as little work as possible and return.
 * During this functions execution,
 * callbacks are called.
 */
static void
multi_perform(churl_context *context)
{
	int			curl_error;

	while (CURLM_CALL_MULTI_PERFORM ==
		   (curl_error = curl_multi_perform(context->multi_handle, &context->curl_still_running)));

	if (curl_error != CURLM_OK)
		elog(ERROR, "internal error: curl_multi_perform failed (%d - %s)",
			 curl_error, curl_easy_strerror(curl_error));
}

static bool
internal_buffer_large_enough(churl_buffer *buffer, size_t required)
{
	return ((buffer->top + required) <= buffer->max);
}

static void
flush_internal_buffer(churl_context *context)
{
	churl_buffer *context_buffer = context->upload_buffer;

	if (context_buffer->top == 0)
		return;

	while ((context->curl_still_running != 0) &&
		   ((context_buffer->top - context_buffer->bot) > 0))
	{
		/*
		 * Allow canceling a query while waiting for input from remote service
		 */
		CHECK_FOR_INTERRUPTS();

		multi_perform(context);
	}

	check_response(context);

	if ((context->curl_still_running == 0) &&
		((context_buffer->top - context_buffer->bot) > 0))
		elog(ERROR, "failed sending to remote component %s", get_dest_address(context->curl_handle));

	context_buffer->top = 0;
	context_buffer->bot = 0;
}

/*
 * Returns the remote ip and port of the curl response.
 * If it's not available, returns an empty string.
 * The returned value should be free'd.
 */
static char *
get_dest_address(CURL *curl_handle)
{
	char	   *dest_url = NULL;

	/* add dest url, if any, and curl was nice to tell us */
	if (CURLE_OK == curl_easy_getinfo(curl_handle, CURLINFO_PRIMARY_IP, &dest_url) && dest_url)
	{
		return psprintf("'%s:%d'", dest_url, get_pxf_port());
	}
	return dest_url;
}

static void
enlarge_internal_buffer(churl_buffer *buffer, size_t required)
{
	if (buffer->ptr != NULL)
		pfree(buffer->ptr);

	buffer->max = required + 1024;
	buffer->ptr = palloc(buffer->max);
}

/*
 * Let libcurl finish the upload by
 * calling perform repeatedly
 */
static void
finish_upload(churl_context *context)
{
	if (!context->multi_handle)
		return;

	flush_internal_buffer(context);

	/*
	 * allow read_callback to say 'all done' by returning a zero thus ending
	 * the connection
	 */
	while (context->curl_still_running != 0)
		multi_perform(context);

	check_response(context);
}

static void
cleanup_curl_handle(churl_context *context)
{
	if (!context->curl_handle)
		return;
	if (context->multi_handle)
		multi_remove_handle(context);
	curl_easy_cleanup(context->curl_handle);
	context->curl_handle = NULL;
	curl_multi_cleanup(context->multi_handle);
	context->multi_handle = NULL;
}

static void
multi_remove_handle(churl_context *context)
{
	int			curl_error;

	Assert(context->curl_handle && context->multi_handle);

	if (CURLM_OK !=
		(curl_error = curl_multi_remove_handle(context->multi_handle, context->curl_handle)))
		elog(ERROR, "internal error: curl_multi_remove_handle failed (%d - %s)",
			 curl_error, curl_easy_strerror(curl_error));
}

static void
cleanup_internal_buffer(churl_buffer *buffer)
{
	if ((buffer) && (buffer->ptr))
	{
		pfree(buffer->ptr);
		buffer->ptr = NULL;
		buffer->bot = 0;
		buffer->top = 0;
		buffer->max = 0;
	}
}

static void
churl_cleanup_context(churl_context *context)
{
	if (context)
	{
		if (context->download_buffer)
		{
			if (context->download_buffer->ptr)
				pfree(context->download_buffer->ptr);
			pfree(context->download_buffer);
		}
		if (context->upload_buffer)
		{
			if (context->upload_buffer->ptr)
				pfree(context->upload_buffer->ptr);
			pfree(context->upload_buffer);
		}

		pfree(context);
	}
}

/*
 * Called by libcurl perform during a download.
 * Stores data from libcurl's buffer into the internal buffer.
 * If internal buffer is not large enough, increases it.
 */
static size_t
write_callback(char *buffer, size_t size, size_t nitems, void *userp)
{
	churl_context *context = (churl_context *) userp;
	churl_buffer *context_buffer = context->download_buffer;
	const int	nbytes = size * nitems;

	if (!internal_buffer_large_enough(context_buffer, nbytes))
	{
		compact_internal_buffer(context_buffer);
		if (!internal_buffer_large_enough(context_buffer, nbytes))
			realloc_internal_buffer(context_buffer, nbytes);
	}

	/* enough space. copy buffer into curl->buf */
	memcpy(context_buffer->ptr + context_buffer->top, buffer, nbytes);
	context_buffer->top += nbytes;

	return nbytes;
}

/*
 * Fills internal buffer up to want bytes.
 * returns when size reached or transfer ended
 */
static void
fill_internal_buffer(churl_context *context, int want)
{
	fd_set		fdread;
	fd_set		fdwrite;
	fd_set		fdexcep;
	int			maxfd;
	int			curl_error;

	/* attempt to fill buffer */
	while (context->curl_still_running &&
		   ((context->download_buffer->top - context->download_buffer->bot) < want))
	{
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		/* allow canceling a query while waiting for input from remote service */
		CHECK_FOR_INTERRUPTS();

		/* set a suitable timeout to fail on */
		long curl_timeo = -1;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		curl_multi_timeout(context->multi_handle, &curl_timeo);
		if (curl_timeo >= 0)
		{
			timeout.tv_sec = curl_timeo / 1000;
			if (timeout.tv_sec > 1)
				timeout.tv_sec = 1;
			else
				timeout.tv_usec = (curl_timeo % 1000) * 1000;
		}

		/* get file descriptors from the transfers */
		curl_error = curl_multi_fdset(context->multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
		if (CURLE_OK != curl_error)
			elog(ERROR, "internal error: curl_multi_fdset failed (%d - %s)",
				 curl_error, curl_easy_strerror(curl_error));

		/* curl is not ready if maxfd -1 is returned */
		if (maxfd == -1)
			pg_usleep(100);
		else if (-1 == select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout))
		{
			int save_errno = errno;
			if (save_errno == EINTR || save_errno == EAGAIN)
				continue;
			elog(ERROR, "internal error: select failed on curl_multi_fdset (maxfd %d) (%d - %s)",
				 maxfd, save_errno, strerror(save_errno));
		}
		multi_perform(context);
	}
}

static void
churl_headers_set(churl_context *context, CHURL_HEADERS headers)
{
	churl_settings *settings = (churl_settings *) headers;

	set_curl_option(context, CURLOPT_HTTPHEADER, settings->headers);
}

/*
 * Checks that the response finished successfully
 * with a valid response status and code.
 */
static void
check_response(churl_context *context)
{
	check_response_code(context);
	check_response_status(context);
}

/*
 * Checks that libcurl transfers completed successfully.
 * This is different than the response code (HTTP code) -
 * a message can have a response code 200 (OK), but end prematurely
 * and so have an error status.
 */
static void
check_response_status(churl_context *context)
{
	CURLMsg    *msg;			/* for picking up messages with the transfer
								 * status */
	int			msgs_left;		/* how many messages are left */
	long		status;

	while ((msg = curl_multi_info_read(context->multi_handle, &msgs_left)))
	{
		int			i = 0;

		/* CURLMSG_DONE is the only possible status. */
		if (msg->msg != CURLMSG_DONE)
			continue;
		if (CURLE_OK != (status = msg->data.result))
		{
			char	   *addr = get_dest_address(msg->easy_handle);
			StringInfoData err;

			initStringInfo(&err);

			/* if the request did not complete correctly, show the error
			 * information. If no detailed error information was written to errbuf
			 * show the more generic information from curl_easy_strerror instead.
			 */

			appendStringInfo(&err, "transfer error (%ld): %s",
							 status, curl_easy_strerror(status));

			if (addr)
			{
				appendStringInfo(&err, " from %s", addr);
				pfree(addr);
			}

			size_t len = strlen(context->curl_error_buffer);
			if (len > 0)
			{
				ereport(ERROR,
					(errmsg("%s", err.data),
					errdetail("curl error buffer: %s", context->curl_error_buffer)));
			}
			else
			{
				ereport(ERROR,
					(errmsg("%s", err.data)));
			}
		}
		ereport(DEBUG2,
			(errmsg("check_response_status: msg %d done with status OK", i++)));
	}
}

/*
 * Parses return code from libcurl operation and
 * reports if different than 200 and 100
 */
static void
check_response_code(churl_context *context)
{
	long		response_code;
	char	   *response_text = NULL;
	int			curl_error;

	if (CURLE_OK != (curl_error = curl_easy_getinfo(context->curl_handle, CURLINFO_RESPONSE_CODE, &response_code)))
		elog(ERROR, "internal error: curl_easy_getinfo failed(%d - %s)",
			 curl_error, curl_easy_strerror(curl_error));

	elog(DEBUG2, "http response code: %ld", response_code);
	if ((response_code == 0) && (context->curl_still_running > 0))
	{
		elog(DEBUG2, "check_response_code: curl is still running, but no data was received.");
	}
	else if (response_code != 200 && response_code != 100)
	{
		StringInfoData err;
		char	   *hint_msg = NULL,
				   *http_error_msg,
				   *trace_msg = NULL;

		initStringInfo(&err);

		/* prepare response text if any */
		if (context->download_buffer->ptr)
		{
			context->download_buffer->ptr[context->download_buffer->top] = '\0';
			response_text = context->download_buffer->ptr + context->download_buffer->bot;
		}

		appendStringInfo(&err, "PXF server error");
		if ((LOG >= log_min_messages) || (LOG >= client_min_messages))
		{
			/* add remote http error code */
			appendStringInfo(&err, "(%ld)", response_code);
		}

		if (!handle_special_error(response_code, &err))
		{
			/*
			 * add detailed error message from the http response.
			 * response_text could be NULL in some cases. get_http_error_msg
			 * checks for that.
			 */
			http_error_msg = get_http_error_msg(response_code, response_text, context->curl_error_buffer, &hint_msg, &trace_msg);

			appendStringInfo(&err, " : %s", http_error_msg);
		}

		if (trace_msg != NULL && hint_msg != NULL)
		{
			ereport(ERROR,
				(errcode(ERRCODE_CONNECTION_EXCEPTION),
				errmsg("%s", err.data),
				errdetail("%s", trace_msg),
				errhint("%s", hint_msg)));
		}
		else if (trace_msg != NULL)
		{
			ereport(ERROR,
				(errcode(ERRCODE_CONNECTION_EXCEPTION),
				errmsg("%s", err.data),
				errdetail("%s", trace_msg)));
		}
		else if (hint_msg != NULL)
		{
			ereport(ERROR,
				(errcode(ERRCODE_CONNECTION_EXCEPTION),
				errmsg("%s", err.data),
				errhint("%s", hint_msg)));
		}
		else
		{
			ereport(ERROR,
				(errcode(ERRCODE_CONNECTION_EXCEPTION),
				errmsg("%s", err.data)));
		}
	}

	free_http_response(context);
}

/*
 * Returns true if the provided json text is valid JSON, false otherwise
 */
static bool
IsValidJson(text *json)
{
	MemoryContext oldcontext    = CurrentMemoryContext;
	bool          is_valid_json = true;
	JsonLexContext *lex;

	PG_TRY();
	{
		/* validate it */
		lex = makeJsonLexContext(json, false);
		pg_parse_json(lex, &nullSemAction);
	}
	PG_CATCH();
	{
		is_valid_json = false;
		MemoryContextSwitchTo(oldcontext);
	}
	PG_END_TRY();

	return is_valid_json;
}

/*
 * Extracts the error message from the full HTTP response
 * We test for several conditions in the http_ret_code and the HTTP response message.
 * The first condition that matches, defines the final message string and ends the function.
 * The layout of the HTTP response message is:

 {
  "timestamp": "the server timestamp",
  "status": status code int,
  "error": "error description",
  "message": "error message",
  "trace": "the stack trace for the error",
  "path": "uri for the request",
  "hint": "hint for the user"
 }

 * An alternative HTTP response message looks like this

 <html>
 <head>
 <meta meta_data_attributes />
 <title> title_content_which_has_a_brief_description_of_the_error </title>
 </head>
 <body>
 <h2> heading_containing_the_error_code </h2>
 <p>
 main_body_paragraph_with_a_detailed_description_of_the_error_on_the_rest_server_side
 <pre> the_error_in_original_format_not_HTML_ususally_the_title_of_the_java_exception</pre>
 </p>
 <h3>Caused by:</h3>
 <pre>
 the_full_java_exception_with_the_stack_output
 </pre>
 <hr /><i><small>Powered by Jetty://</small></i>
 <br/>
 <br/>
 </body>
 </html>

 * Our first priority is to get the paragraph <p> inside <body>, and in case we don't find it, then we try to get
 * the <title>.
 */
char *
get_http_error_msg(long http_ret_code, char *msg, char *curl_error_buffer, char **hint_message, char **trace_message)
{
	char	   *start,
			   *end,
			   *ret,
			   *fmessagestr = "message",
			   *ftracestr = "trace",
			   *fhintstr = "hint";

	text	   *error_text;
	Datum		result;
	StringInfoData errMsg;
	FmgrInfo *json_object_field_text_fn;


	initStringInfo(&errMsg);

	/*
	 * 1. The server not listening on the port specified in the <create
	 * external...> statement" In this case there is no Response from the
	 * server, so we issue our own message
	 */

	if (http_ret_code == 0)
	{
		*hint_message = "Use the 'pxf [cluster] start' command to start the PXF service.";
		if (curl_error_buffer == NULL)
			return "There is no pxf servlet listening on the host and port specified in the external table url";
		else
		{
			return curl_error_buffer;
		}
	}

	/*
	 * 2. There is a response from the server since the http_ret_code is not
	 * 0, but there is no response message. This is an abnormal situation that
	 * could be the result of a bug, libraries incompatibility or versioning
	 * issue in the Rest server or our curl client. In this case we again
	 * issue our own message.
	 */
	if (!msg || (msg && strlen(msg) == 0))
	{
		appendStringInfo(&errMsg, "HTTP status code is %ld but HTTP response string is empty", http_ret_code);
		ret = pstrdup(errMsg.data);
		pfree(errMsg.data);
		return ret;
	}

	/*
	 * 3. The "normal" case - There is an HTTP response and the response has a
	 * <body> section inside where there is a paragraph contained by the <p>
	 * tag.
	 */
	start = strstr(msg, "<body>");
	if (start != NULL)
	{
		start = strstr(start, "<p>");
		if (start != NULL)
		{
			char	   *tmp;
			bool		skip = false;

			start += 3;
			end = strstr(start, "</p>");	/* assuming where is a <p>, there
											 * is a </p> */
			if (end != NULL)
			{
				/* Take one more line after the </p> */
				tmp = strchr(end, '\n');
				if (tmp != NULL)
					end = tmp;

				tmp = start;

				/*
				 * Right now we have the full paragraph inside the <body>. We
				 * need to extract from it the <pre> tags, the '\n' and the
				 * '\r'.
				 */
				while (tmp != end)
				{
					if (*tmp == '>')	/* skipping the <pre> tags */
						skip = false;
					else if (*tmp == '<')	/* skipping the <pre> tags */
					{
						skip = true;
						appendStringInfoChar(&errMsg, ' ');
					}
					else if (*tmp != '\n' && *tmp != '\r' && skip == false)
						appendStringInfoChar(&errMsg, *tmp);
					tmp++;
				}

				ret = pstrdup(errMsg.data);
				pfree(errMsg.data);
				return ret;
			}
		}
	}

	/*
	 * 4. We did not find the <body>. So we try to print the <title>.
	 */
	start = strstr(msg, "<title>");
	if (start != NULL)
	{
		start += 7;

		/*
		 * no need to check if end is null, if <title> exists then also
		 * </title> exists
		 */
		end = strstr(start, "</title>");
		if (end != NULL)
		{
			ret = pnstrdup(start, end - start);
			return ret;
		}
	}

	error_text = cstring_to_text(msg);

	/*
	 * 5. First make sure we have a valid JSON so we can extract the fields we
	 * need for the error message. If we don't have a valid JSON just return
	 * the raw error text.
	 */
	if (!IsValidJson(error_text))
		return msg;

	/*
	 * 6. The "normal" case - There is an HTTP response and we parse the
	 * json response fields "message" and "trace"
	 */
	json_object_field_text_fn = palloc(sizeof(FmgrInfo));

	/* find the json_object_field_text function */
	fmgr_info(F_JSON_OBJECT_FIELD_TEXT, json_object_field_text_fn);

	if ((LOG >= log_min_messages) || (LOG >= client_min_messages))
	{
		/* get the "trace" field from the json error */
		result = FunctionCall2(json_object_field_text_fn,
			PointerGetDatum(error_text),
			PointerGetDatum(cstring_to_text(ftracestr)));

		if (DatumGetPointer(result) != NULL)
			*trace_message = text_to_cstring(DatumGetTextP(result));
	}

	/* get the "hint" field from the json error */
	result = FunctionCall2(json_object_field_text_fn,
		PointerGetDatum(error_text),
		PointerGetDatum(cstring_to_text(fhintstr)));

	if (DatumGetPointer(result) != NULL)
		*hint_message = text_to_cstring(DatumGetTextP(result));

	/* get the "message" field from the json error */
	result = FunctionCall2(json_object_field_text_fn,
		PointerGetDatum(error_text),
		PointerGetDatum(cstring_to_text(fmessagestr)));

	pfree(json_object_field_text_fn);

	if (DatumGetPointer(result) != NULL)
	{
		char* parsed_message = text_to_cstring(DatumGetTextP(result));

		end = strstr(parsed_message, "\n");
		if (end != NULL)
		{
			ret = pnstrdup(parsed_message, end - parsed_message);
			return ret;
		}
		return parsed_message;
	}

	/*
	 * 5. This is an unexpected situation. We received an error message from
	 * the server but it does not have neither a <body> nor a <title>. In this
	 * case we return the error message we received as-is.
	 */
	return msg;
}

static void
free_http_response(churl_context *context)
{
	if (!context->last_http_reponse)
		return;

	pfree(context->last_http_reponse);
	context->last_http_reponse = NULL;
}

/*
 * Called during a perform by libcurl on either download or an upload.
 * Stores the first line of the header for error reporting
 */
static size_t
header_callback(char *buffer, size_t size, size_t nitems, void *userp)
{
	const int	nbytes = size * nitems;
	churl_context *context = (churl_context *) userp;

	if (context->last_http_reponse)
		return nbytes;

	char	   *p = palloc(nbytes + 1);

	memcpy(p, buffer, nbytes);
	p[nbytes] = 0;
	context->last_http_reponse = p;

	return nbytes;
}

static void
compact_internal_buffer(churl_buffer *buffer)
{
	int			n;

	/* no compaction required */
	if (buffer->bot == 0)
		return;

	n = buffer->top - buffer->bot;
	memmove(buffer->ptr, buffer->ptr + buffer->bot, n);
	buffer->bot = 0;
	buffer->top = n;
}

static void
realloc_internal_buffer(churl_buffer *buffer, size_t required)
{
	int			n;

	n = buffer->top - buffer->bot + required + 1024;
	if (buffer->ptr == NULL)
		buffer->ptr = palloc(n);
	else
		/* repalloc does not support NULL ptr */
		buffer->ptr = repalloc(buffer->ptr, n);

	buffer->max = n;
}

static bool
handle_special_error(long response, StringInfo err)
{
	switch (response)
	{
		case 404:
			appendStringInfo(err, ": PXF service could not be reached. PXF is not running in the tomcat container");
			break;
		default:
			return false;
	}
	return true;
}
