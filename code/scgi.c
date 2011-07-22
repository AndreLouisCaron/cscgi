/* Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

#include "scgi.h"

static const char * scgi_error_messages[] =
{
    "so far, so good",
    "string too long",
    "expected digit",
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
    while ((used < size) && (parser->state != scgi_parser_fail))
    {
        if ( parser->state == scgi_parser_field )
        {
            peek = scgi_seek(data+used, size-used);
            parser->accept_field(parser, data+used, peek);
            used += peek;
            if ((used < size) && (data[used] == '\0')) {
                ++used;
                parser->state = scgi_parser_value;
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

int scgi_head_overflow
    ( const struct scgi_limits * limits, size_t parsed )
{
    return (limits->maximum_head_length != 0)
        && (parsed > limits->maximum_head_length);
}

int scgi_body_overflow
    ( const struct scgi_limits * limits, size_t parsed )
{
    return (limits->maximum_body_length != 0)
        && (parsed > limits->maximum_body_length);
}

void scgi_setup ( struct scgi_limits * limits, struct scgi_parser * parser )
{
    limits->maximum_head_length = 0;
    limits->maximum_body_length = 0;
      /* nestring parser setup. */
    parser->header_limits.maximum_length = limits->maximum_head_length;
    netstring_setup(&parser->header_limits, &parser->header_parser);
    parser->header_parser.accept = &scgi_accept_netstring;
    parser->header_parser.finish = &scgi_finish_netstring;
    parser->header_parser.object = parser;
      /* global parser setup. */
    parser->state = scgi_parser_field;
    parser->error = scgi_error_ok;
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
    if ((parser->state == scgi_parser_field) ||
        (parser->state == scgi_parser_value))
    {
        used = netstring_consume(&parser->header_limits,
            &parser->header_parser, data, size);
        if ( parser->header_parser.state == netstring_parser_done )
        {
            parser->state = scgi_parser_body;
        }
    }
    if ( parser->state == scgi_parser_body )
    {
        parser->accept_body(parser, data+used, size-used);
    }
    return (used);
}
