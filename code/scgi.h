#ifndef _scgi_h__
#define _scgi_h__

/* Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
**
** This document is covered by the an Open Source Initiative approved license. A
** copy of the license should have been provided alongside this software package
** (see "LICENSE.txt"). If not, terms of the license are available online at
** "http://www.opensource.org/licenses/mit". */

/*!
 * @file scgi.h
 * @author Andre Caron <andre.l.caron@gmail.com>
 * @brief Parser for Simple Common Gateway Interface (SCGI) requests.
 */

#include "cnetstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*!
   * @brief Enumeration of error states the parser may report.
   */
enum scgi_parser_error
{
    scgi_error_ok=0,
    scgi_error_head_overflow,
    scgi_error_body_overflow,
};

  /*!
   * @brief Gets a human-readable description of the error.
   */
const char * scgi_error_message ( enum scgi_parser_error error );

  /*!
   * @brief Enumeration of parser states.
   */
enum scgi_parser_state
{
      /*! @private
       */
    scgi_parser_field,
      /*! @private
       */
    scgi_parser_value,
      /*!
       * @brief Headers have been completely parsed.
       */
    scgi_parser_body,
      /*!
       * @brief An error was detected.  Check the parser's @a error field.
       */
    scgi_parser_fail,
};

  /*!
   * @brief Customizable limits for SCGI request definitions.
   */
struct scgi_limits
{
      /*!
       * @brief Maximum length of the netstring containing headers.
       *
       * @note This limit excludes the two characters used as delimiters.
       */
    size_t maximum_head_length;

      /*!
       * @brief Maximum admissible of the request body.
       *
       * @note Because this parser does not interpret headers, it cannot
       *  validate the size provided as "Content-Length" header.  It will only
       *  report a hint to indicate the the maximum body length has been
       *  reached.  It is up to the client code to validate the length
       *  specified in the content header with the amount of data provided
       *  through the @c accept_body callback.
       */
    size_t maximum_body_length;
};

  /*!
   * @brief SCGI request parser state.
   *
   * The parser is implemented as a Finite State Machine (FSM).  By itself, it
   * does not buffer any data.  As soon as the syntax is validated, all content
   * is forwarded to the client code through the callbacks.
   */
struct scgi_parser
{
      /*! @public
       * @brief Current state of the parser.
       *
       * Client code should check the state after each call to @c scgi_consume()
       * to check for important state transitions.  For instance, headers may
       * be reliably interpreted after the parser passes into the
       * @c scgi_parser_body state.  In some situations, clever use of the
       * current state may allow the application to start responding even though
       * the request has not been completely received.
       *
       * @warning This field is provided to clients as read-only.  Any attempt
       *  to change it will cause unpredictable output (inconsistent headers and
       *  body content).
       */
    enum scgi_parser_state state;

      /*! @public
       * @brief Last error reported by the parser.
       *
       * @warning This field should only be interpreted if @c state is set to
       *  @c scgi_parser_fail.  Its value is undefined at all other times.
       */
    enum scgi_parser_error error;

      /*! @private
       * @brief Internal sub-parser for the netstring containing the headers.
       */
    struct netstring_limits header_limits;

      /*! @private
       * @brief Internal sub-parser for the netstring containing the headers.
       */
    struct netstring_parser header_parser;

      /*! @public
       * @brief Extra field for client code's use.
       *
       * The contents of this field are not interpreted in any way by the SCGI
       * request parser.  It is used for any purpose by client code.  Usually,
       * this field serves as link back to the owner object and used by
       * registered callbacks.
       */
    void * object;

      /*!
       * @brief Callback supplying data for a header field name.
       * @param parser The SCGI parser itself.  Useful for checking the parser
       *  state (amount of parsed data, etc.) or accessing the @c object field.
       * @param data Pointer to first byte of data.
       * @param size Size of @a data, in bytes.
       *
       * The SCGI request parser does not buffer any data.  Rather, it calls the
       * callback with any consumed data as soon as it is made available by
       * client code.  This means that this callback may be invoked several
       * times for the same header.
       */
    void(*accept_field)(struct scgi_parser*, const char *, size_t);

      /*!
       * @brief Callback supplying data for a header value.
       * @param parser The SCGI parser itself.  Useful for checking the parser
       *  state (amount of parsed data, etc.) or accessing the @c object field.
       * @param data Pointer to first byte of data.
       * @param size Size of @a data, in bytes.
       *
       * The SCGI request parser does not buffer any data.  Rather, it calls the
       * callback with any consumed data as soon as it is made available by
       * client code.  This means that this callback may be invoked several
       * times for the same header.
       */
    void(*accept_value)(struct scgi_parser*, const char *, size_t);

      /*!
       * @brief Callback indicating the end of SCGI headers.
       *
       * Once this callback is invoked, it is safe to read header values as all
       * headers are guaranteed to have been fully parsed.
       */
    void(*finish_head)(struct scgi_parser*);

      /*!
       * @brief Callback supplying data for body contents.
       * @param parser The SCGI parser itself.  Useful for checking the parser
       *  state (amount of parsed data, etc.) or accessing the @c object field.
       * @param data Pointer to first byte of data.
       * @param size Size of @a data, in bytes.
       *
       * The SCGI request parser does not buffer any data.  Rather, it calls the
       * callback with any consumed data as soon as it is made available by
       * client code.  This means that this callback may be invoked several
       * times for the same request body.
       */
    void(*accept_body)(struct scgi_parser*, const char *, size_t);
};

  /*!
   * @brief Initialize a parser and its limits.
   */
void scgi_setup ( struct scgi_limits * limits, struct scgi_parser * parser );

  /*!
   * @brief Clear errors and reset the parser state.
   *
   * This function does not clear the @c object and callback fields.  You may
   * call it to re-use any parsing context, such as allocated buffers for
   * headers and body data.
   */
void scgi_clear ( struct scgi_parser * parser );

  /*!
   * @brief Feed data to the parser.
   * @param data Pointer to first byte of data.
   * @param size Size of @a data, in bytes.
   * @return Number of bytes consumed.  Normally, this value is equal to
   *  @a size.  However, the parser may choose to interrupt parser early or stop
   *  processing data because of an error.
   *
   * You should @e always check the parser state after a call to this method.
   * In particular, all data may be consumed before an error is reported, so
   * a return value equal to @a size is not a reliable indicator of success.
   */
size_t scgi_consume ( const struct scgi_limits * limits,
    struct scgi_parser * parser, const char * data, size_t size );

#ifdef __cplusplus
}
#endif

#endif /* _scgi_h__ */
