/* Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

#include "scgi.h"

static size_t scgi_min ( size_t lhs, size_t rhs )
{
    return ((lhs < rhs)? lhs : rhs);
}

static const char * scgi_error_messages[] =
{
    "so far, so good",
    "bad request head syntax",
    "request head too long",
    "request body too long",
};

const char * scgi_error_message ( enum scgi_parser_error error )
{
    return (scgi_error_messages[error]);
}

static size_t scgi_seek ( const char * data, size_t size )
{
    size_t peek = 0;
    while ((peek < size) && (data[peek] != '\0')) {
        ++peek;
    }
    return (peek);
}

static void scgi_accept_head
    ( struct scgi_parser * parser, const char * data, size_t size )
{
    size_t used = 0;
    size_t peek = 0;
    while ((used < size) && (parser->error == scgi_error_ok))
    {
        if ( parser->state == scgi_parser_field )
        {
            peek = scgi_seek(data+used, size-used);
            parser->accept_field(parser, data+used, peek);
            used += peek;
            if ((used < size) && (data[used] == '\0')) {
                ++used;
                parser->state = scgi_parser_value;
                /* let the owner know they can stop buffering. */
                if (parser->finish_field) {
                    parser->finish_field(parser);
                }
            }
        }
        if ( parser->state == scgi_parser_value )
        {
            peek = scgi_seek(data+used, size-used);
            parser->accept_value(parser, data+used, peek);
            used += peek;
            if ((used < size) && (data[used] == '\0')) {
                ++used;
                parser->state = scgi_parser_field;
                /* let the owner know they can stop buffering. */
                if (parser->finish_value) {
                    parser->finish_value(parser);
                }
            }
        }
    }
}

static void scgi_finish_head ( struct scgi_parser * parser )
{
    parser->finish_head(parser);
}

static void scgi_accept_netstring
    ( struct netstring_parser * parser, const char * data, size_t size )
{
    scgi_accept_head((struct scgi_parser*)parser->object, data, size);
}

static void scgi_finish_netstring
    ( struct netstring_parser * parser )
{
    scgi_finish_head((struct scgi_parser*)parser->object);
}

static int scgi_head_overflow
    ( const struct scgi_limits * limits, size_t size )
{
    return (limits->max_head_size != 0)
        && (size > limits->max_head_size);
}

static int scgi_body_overflow
    ( const struct scgi_limits * limits, size_t size )
{
    return (limits->max_body_size != 0)
        && (size > limits->max_body_size);
}

void scgi_setup ( struct scgi_limits * limits, struct scgi_parser * parser )
{
      /* nestring parser setup. */
    parser->header_limits.max_size = limits->max_head_size;
    netstring_setup(&parser->header_limits, &parser->header_parser);
    parser->header_parser.accept = &scgi_accept_netstring;
    parser->header_parser.finish = &scgi_finish_netstring;
    parser->header_parser.object = parser;
      /* store limits. */
    parser->limits.max_head_size = limits->max_head_size;
    parser->limits.max_body_size = limits->max_body_size;
      /* global parser setup. */
    parser->state = scgi_parser_field;
    parser->error = scgi_error_ok;
    parser->body_size = 0;
    parser->finish_field = 0;
    parser->finish_value = 0;
}

void scgi_clear ( struct scgi_parser * parser )
{
    parser->state = scgi_parser_field;
    parser->error = scgi_error_ok;
}

size_t scgi_consume ( const struct scgi_limits * limits,
    struct scgi_parser * parser, const char * data, size_t size )
{
    size_t used = 0;
    size_t pass = 0;
    if ((parser->state == scgi_parser_field) ||
        (parser->state == scgi_parser_value))
    {
        used = netstring_consume(&parser->header_limits,
            &parser->header_parser, data, size);
        if ( parser->header_parser.error != netstring_error_ok )
        {
            /* translate netstring errors. */
            if ( parser->header_parser.error == netstring_error_overflow ) {
                parser->error = scgi_error_head_overflow;
            }
            if ( parser->header_parser.error == netstring_error_syntax ) {
                /* TODO: handle this. */
            }
            return (used);
        }
        if ( parser->header_parser.state == netstring_parser_done )
        {
            parser->state = scgi_parser_body;
        }
    }
    if ( parser->state == scgi_parser_body )
    {
        /* grab as much data as we possibly can without exceeding limits. */
        pass = size-used;
        if (parser->limits.max_body_size > 0) {
            pass = scgi_min(pass,
                            parser->limits.max_body_size-parser->body_size);
        }
        /* try to consume the chunk. */
        pass = parser->accept_body(parser, data+used, pass);
        used += pass, parser->body_size += pass;
        /* overflow if:
           - there is data left to consume; and
           - we have an upper bound on the body size; and
           - the total amount consumed exceeds the upper bound.
         */
        if (((size-used) > 0) && (parser->limits.max_body_size > 0)
            && (parser->body_size == parser->limits.max_body_size))
        {
            parser->error = scgi_error_body_overflow;
        }
    }
    return (used);
}
