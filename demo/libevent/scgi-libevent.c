// Copyright (c) 2012, Luka Perkov (freeacs-ng@lukaperkov.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <arpa/inet.h>
#include <errno.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scgi.h>

// Bookkeeping for each connection.
struct connection_t
{
    // SCGI request parser.
    struct scgi_parser parser;

    // libevent data stream (socket).
    struct bufferevent * stream;

    // Request buffers.
    struct evbuffer * head;
    struct evbuffer * body;

    // TODO: add custom data...
};

// SCGI callback functions.
static void accept_field (struct scgi_parser * parser,
                          const char * data, size_t size);
static void finish_field (struct scgi_parser * parser);
static void accept_value (struct scgi_parser * parser,
                          const char * data, size_t size);
static void finish_value (struct scgi_parser * parser);
static void finish_head (struct scgi_parser * parser);
static size_t accept_body (struct scgi_parser * parser,
			   const char * data, size_t size);

struct connection_t * prepare_connection ()
{
    // Allocate memory for bookkeeping.
    struct connection_t * connection = malloc(sizeof(struct connection_t));
    if (connection == NULL) {
        fprintf(stderr, "Failed to allocate connection object.\n");
        return (NULL);
    }
    memset(connection, 0, sizeof(struct connection_t));

    fprintf(stderr, "Setting up SCGI parser.\n");

    // Prepare the SCGI connection parser.
    struct scgi_limits limits =	{
        .max_head_size =  4*1024,
        .max_body_size = 60*1024,
    };
    scgi_setup(&limits, &connection->parser);

    // Register SCGI parser callbacks.
    connection->parser.accept_field = accept_field;
    connection->parser.finish_field = finish_field;
    connection->parser.accept_value = accept_value;
    connection->parser.finish_value = finish_value;
    connection->parser.finish_head = finish_head;
    connection->parser.accept_body = accept_body;

    // Make sure SCGI callbacks can access the connection object.
    connection->parser.object = connection;

    // Will be initialized later.
    connection->stream = 0;

    // Prepare buffer for HTTP headers.
    connection->head = evbuffer_new();
    if (connection->head == NULL) {
        fprintf(stderr, "Failed to allocate memory for HTTP head\n.");
    }
    if (evbuffer_expand(connection->head, 4*1024) != 0) {
        fprintf(stderr, "Failed to allocate memory for HTTP headers\n.");
    }

    // Prepare buffer for HTTP body.
    connection->body = evbuffer_new();
    if (connection->body == NULL) {
        fprintf(stderr, "Failed to allocate memory for HTTP body.\n");
    }

    // TODO: initialize extra data.

    fprintf(stderr, "Connection object ready.\n.");
    return (connection);
}

void release_connection (struct connection_t * connection)
{ 
    fprintf(stderr, "Dropping connection.\n.");

    // Close socket.
    bufferevent_free(connection->stream);

    // Release bookkeeping data.
    free(connection);
}


// Buffer header name inside connection object.
static void accept_field (struct scgi_parser * parser,
                          const char * data, size_t size)
{
    struct connection_t * connection = parser->object;
    evbuffer_add(connection->head, data, size);
}

// Null-terminate the HTTP header name.
static void finish_field (struct scgi_parser * parser)
{
    struct connection_t * connection = parser->object;
    evbuffer_add(connection->head, "\0", 1);
}

// Buffer header data inside connection object.
static void accept_value (struct scgi_parser * parser,
                          const char * data, size_t size)
{
    struct connection_t * connection = parser->object;
    evbuffer_add(connection->head, data, size);
}

// Null-terminate the HTTP header data.
static void finish_value (struct scgi_parser * parser)
{
    struct connection_t * connection = parser->object;
    evbuffer_add(connection->head, "\0", 1);
}

// Double-null terminate the HTTP headers, start responding.
static void finish_head (struct scgi_parser * parser)
{
    struct evbuffer_ptr here;
    struct evbuffer_ptr next;
    struct evbuffer_iovec name;
    struct evbuffer_iovec data;
    struct connection_t * connection = parser->object;

    fprintf(stderr, "Headers (done).\n");

    // Iterate over HTTP headers.
    memset(&here, 0, sizeof(here));
    evbuffer_ptr_set(connection->head, &here, 0, EVBUFFER_PTR_SET);
    next = evbuffer_search(connection->head, "\0", 1, &here);
    while (next.pos >= 0)
    {
        // Locate the header name.
        evbuffer_peek(connection->head, next.pos-here.pos, &here, &name, 1);
        name.iov_len = next.pos-here.pos; // truncate.

        // Locate the header data.
        evbuffer_ptr_set(connection->head, &next, 1, EVBUFFER_PTR_ADD);
        here = next;
        next = evbuffer_search(connection->head, "\0", 1, &next);
        evbuffer_peek(connection->head, next.pos-here.pos, &here, &data, 1);
        data.iov_len = next.pos-here.pos; // truncate.

        // Print out the HTTP header.
        fprintf(stderr, "'%*s': '%*s'.\n",
                (int)name.iov_len, (const char*)name.iov_base,
                (int)data.iov_len, (const char*)data.iov_base);

        // Locate the next header.
        evbuffer_ptr_set(connection->head, &next, 1, EVBUFFER_PTR_ADD);
        here = next;
        next = evbuffer_search(connection->head, "\0", 1, &next);
    }

    fprintf(stderr, "Starting response.\n");

    // Start responding.
    struct evbuffer * output = bufferevent_get_output(connection->stream);
    char response[] =
        "Status: 200 OK" "\r\n"
        "Content-Type: text/plain" "\r\n"
        "\r\n"
        "hello world\n"
        ;
    evbuffer_add(output, response, sizeof(response)-1);
}

static size_t accept_body (struct scgi_parser * parser,
			   const char * data, size_t size)
{
    struct connection_t * connection = parser->object;

    // TODO: continue responding (if necessary).

    return (size);
}

static void read_cb (struct bufferevent * stream, void * context)
{
    struct connection_t * connection = context;

    // Extract all data from the buffer event.
    struct evbuffer * input = bufferevent_get_input(stream);
    size_t size = evbuffer_get_length(input);
    char * data = malloc(size);
    fprintf(stderr, "Reading request data (%d bytes).\n", (int)size);
    if (evbuffer_remove(input, data, size) == -1)
    {
        // Log the error.
        fprintf(stderr, "Could not read data from the buffer.\n");

        // Drop connection.
        release_connection(connection);

        // Release our copy of the input data.
        free(data), data = 0;

        // Exit
        return;
    }
 
    fprintf(stderr, "Feeding request data:\n%*s\n",
            (int)size, (const char*)data);

    // Feed the input data to the SCGI request parser.
    //   All actual processing is done inside the SCGI callbacks
    //   registered by our application.  Callbacks are always
    //   invoked by a call to "scgi_consume()".
    size_t used = scgi_consume(&connection->parser, data, size);
    if (connection->parser.error != scgi_error_ok)
    {
        // Log the error.
        fprintf(stderr, "SCGI request error: \"%s\".\n",
                scgi_error_message(connection->parser.error));

        // Drop connection.
        release_connection(connection);
    }

    fprintf(stderr, "Dropping request data.\n");

    // Release our copy of the input data.
    free(data), data = 0;
}

static void write_cb (struct bufferevent * stream, void * context)
{
    struct connection_t * connection = context;
    struct evbuffer * output = bufferevent_get_output(stream);
    struct evbuffer * input  = bufferevent_get_input(stream);

    if (evbuffer_get_length(output) == 0 &&
        evbuffer_get_length(input) == 0) {
        //fprintf(stderr, "Done with connection...\n");
        //release_connection(connection);
    }
}

static void echo_event_cb (struct bufferevent * stream,
                           short events, void * context)
{
    struct connection_t * connection = context;

    fprintf(stderr, "Error on socket.\n");

    if (events & BEV_EVENT_ERROR) {
        perror("Error from bufferevent");
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        release_connection(connection);
    }
}

static void accept_conn_cb (struct evconnlistener * listener,
                            evutil_socket_t socket,
                            struct sockaddr * peer, int size, void * context)
{
    fprintf(stdout, "Accepting connection!\n");

    // Prepare an SCGI request parser for the new connection.
    struct connection_t * connection = prepare_connection();
    if (connection == NULL) {
        fprintf(stderr, "Failed to allocate connection object.\n");
    }

    fprintf(stderr, "Getting stream.\n");

    // Prepare to read and write over the connected socket.
    struct event_base * base = evconnlistener_get_base(listener);
    struct bufferevent * stream = bufferevent_socket_new
        (base, socket, BEV_OPT_CLOSE_ON_FREE);
    if (stream == NULL) {
        fprintf(stderr, "Failed to allocate stream.\n");
    }
    fprintf(stderr, "Configuring stream.\n");
    connection->stream = stream;
    bufferevent_setcb(stream, read_cb, write_cb, echo_event_cb, connection);
    bufferevent_enable(stream, EV_READ|EV_WRITE);
    fprintf(stderr, "Stream ready.\n");
}

static void accept_error_cb (struct evconnlistener * listener, void * context)
{
    struct event_base * base = evconnlistener_get_base(listener);

    // Log the error.
    const int error = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", error, evutil_socket_error_to_string(error));

    // Kill the application's main event loop.
    event_base_loopexit(base, NULL);
}

int main (int argc, char ** argv)
{
    struct event_base *base = event_base_new();
    if (!base) {
        puts("Couldn't open event base");
        return EXIT_FAILURE;
    }

    // Configure to listen on *:9000.
    struct sockaddr_in host;
    memset(&host, 0, sizeof(host));
    host.sin_family      = AF_INET;
    host.sin_addr.s_addr = htonl(0);
    host.sin_port        = htons(9000);

    // Start listening for incomming connections.
    struct evconnlistener * listener = evconnlistener_new_bind(
        base, accept_conn_cb, NULL,
        LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,
        -1,
        (struct sockaddr*)&host, sizeof(host));
    if (!listener) {
        perror("Couldn't create listener");
        return 1;
    }

    // Handle errors with the listening socket (e.g. network goes down).
    evconnlistener_set_error_cb(listener, accept_error_cb);

    // Process event notifications forever.
    event_base_dispatch(base);
    return EXIT_SUCCESS;
}
