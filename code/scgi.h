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

#include <netstring.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// MSVC doesn't have <sys/types.h> but has something similar in <BaseTsd.h>.
#if defined(_WIN64)
typedef __int64 ssize_t;
#elif defined(_WIN32)
typedef __int32 ssize_t;
#else
 #include <sys/types.h>
#endif

  /*!
   * @brief Enumeration of error states the parser may report.
   */
enum scgi_parser_error
{
    scgi_error_ok=0,
    scgi_error_head_syntax,
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
       * @private
       * @brief Headers have been completely parsed.
       */
    scgi_parser_body,
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
    size_t max_head_size;

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
    size_t max_body_size;
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
       * You should check this after each call to @c scgi_consume().
       */
    enum scgi_parser_error error;

      /*! @private
       * @brief Internal sub-parser for the netstring containing the headers.
       */
    struct netstring_limits header_limits;

    struct scgi_limits limits;

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
       * @public
       * @brief Size of body processed so far, in bytes.
       *
       * This field is updated automatically and should be considered read-only
       * by applications and their callbacks.
       *
       * @warning In a call to @c accept_body(), the size of the body processed
       *  so far does not include the @c size argument passed in the call to @c
       *  accept_body().  The reason is that the callback may choose not to
       *  process all data and the value can only be updated after the call
       *  completes (e.g. the @c body_size value actually is equal to the sum
       *  of the return values of the @c accept_body() callback).
       */
    size_t body_size;

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
       *
       * @see finish_field
       */
    void(*accept_field)(struct scgi_parser*, const char *, size_t);

      /*!
       * @brief Informs the application that the header name is finished.
       * @param parser The SCGI parser itself.  Useful for checking the parser
       *  state (amount of parsed data, etc.) or accessing the @c object field.
       *
       * When all data has been passed to the @c accept_field callback, this
       * callback is invoked to let the application know it can process the
       * HTTP header name.
       *
       * @see accept_field
       */
    void(*finish_field)(struct scgi_parser*);

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
       *
       * @see finish_value
       */
    void(*accept_value)(struct scgi_parser*, const char *, size_t);

      /*!
       * @brief Informing the application that the header data is finished.
       * @param parser The SCGI parser itself.  Useful for checking the parser
       *  state (amount of parsed data, etc.) or accessing the @c object field.
       *
       * When all data has been passed to the @c accept_value callback, this
       * callback is invoked to let the application know it can process the
       * HTTP header data.
       *
       * @see accept_value
       */
    void(*finish_value)(struct scgi_parser*);

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
       * @return The amount of data processed.  Will be less or equal to @a
       *  size.
       *
       * The SCGI request parser does not buffer any data.  Rather, it calls the
       * callback with any consumed data as soon as it is made available by
       * client code.  This means that this callback may be invoked several
       * times for the same request body.
       */
    size_t(*accept_body)(struct scgi_parser*, const char *, size_t);
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
   * You should @e always check the parser's @c error field after a call to
   * this method.  In particular, all data may be consumed before an error is
   * reported, so a return value equal to @a size is not a reliable indicator
   * of success.
   */
size_t scgi_consume (struct scgi_parser * parser,
                     const char * data, size_t size);

/*!
 * @brief Check an HTTP header's name for the @c Content-Length header value.
 * @param data Buffered header name data.
 * @param size Buffered header name size.
 * @return 0 if the header name does not match, else non-zero.
 *
 * This function should be used in the @c finish_value() callback to check the
 * data buffered by one or more calls to @c accept_field() and @c
 * accept_value().
 */
int scgi_is_content_length (const char * data, size_t size);

/*!
 * @brief Parse the HTTP @c Content-Length header value.
 * @param data Buffered header value data.
 * @param size Buffered header value size.
 * @return -1 if the header value is not a non-negative integer, else the size
 *  of the HTTP request body in bytes.
 *
 * You typically use this once @c scgi_is_content_length() returns non-zero.
 */
ssize_t scgi_parse_content_length (const char * data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _scgi_h__ */
