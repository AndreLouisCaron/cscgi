#ifndef _scgi_hpp__
#define _scgi_hpp__

// Copyright (c) 2011-2012, Andre Caron (andre.l.caron@gmail.com)
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

/*!
 * @file
 * @brief Parser for Simple Common Gateway Interface (SCGI) requests.
 */

#include "scgi.h"
#include <iosfwd>
#include <string>
#include <map>

namespace scgi {

    class Error :
        public std::exception
    {
        /* data. */
    private:
      ::scgi_parser_error myCode;

        /* construction. */
    public:
        Error (::scgi_parser_error code)
          : myCode(code)
        {}

        /* methods. */
    public:
        virtual const char * what () const throw() {
            return (::scgi_error_message(myCode));
        }
    };

    /*!
     * @brief Representation of SCGI request headers.
     */
    typedef std::map<std::string, std::string> Headers;

    /*!
     * @brief Streaming parser for SCGI requests.
     *
     * @note This class is a request @e parser.  It cannot be used to format
     *  outgoing requests.
     */
    class Request
    {
        /* data. */
    private:
        enum State {
            Null,
            Head,
            Body,
            Done,
        };
        ::scgi_limits myLimits;
        ::scgi_parser myParser;
        std::string myField;
        std::string myValue;
        Headers myHeaders;
        std::string myContent;
        State myState;
        std::size_t myContentLength;

        /* construction. */
    public:
        /*!
         * @brief Create a parser with a default maximum length for strings.
         *
         * @note The default limits are set through macros defined by the
         *  build script.  When unspecified in the build script, rather high
         *  (but unspecified) limits are used.
         */
        Request ();

        /* methods. */
    public:
        /*!
         * @brief Prepare to start parsing a new request.
         *
         * @note This method clears the string contents but does not release
         *  allocated buffers.  Re-using parser instances allows reduced
         *  memory allocation in long running processes.
         */
        void clear ();

        /*!
         * @brief Feed the parser some data.
         * @param data Start of buffer.
         * @param size Size of valid data in the buffer (in bytes).
         * @param return Number of bytes processed.
         * @throws std::exception The parser reported an error with the data
         *  provided as the netstring.
         *
         * This method allows to parse the data as it is made available.
         * This is an important property for high-performance networking
         * applications.
         */
        size_t feed (const char * data, size_t size);

        /*!
         * @brief Get the all headers defined in the request.
         * @return A string to string mapping containing headers and their
         *  values.
         */
        const Headers& headers () const;

        /*!
         * @brief Check for presence of a specific header.
         * @param field Header name.
         * @return @c true if the header is defined and non-empty, @c false
         *  otherwise.
         */
        bool hasheader (const std::string& field) const;

        /*!
         * @brief Lookup a specific header's value.
         * @param field Header name.
         * @return An empty string if the header is not defined or empty,
         *  the header's value otherwise.
         */
        const std::string header (const std::string& field) const;

        /*!
         * @brief Access the parsed request body.
         * @return A buffer containing the character data.
         */
        const std::string& body () const;

        bool head_complete () const;
        bool body_complete () const;

        std::size_t body_size () const;

        /* class methods. */
    private:
        static void accept_field
            (::scgi_parser* parser, const char * data, size_t size);
        static void accept_value
            (::scgi_parser* parser, const char * data, size_t size);
        static void finish_value (::scgi_parser * parser);
        static void finish_head (::scgi_parser* parser);
        static size_t accept_body
            (::scgi_parser* parser, const char * data, size_t size);
    };

    std::istream& operator>> (std::istream& stream, Request& request);

}

#endif /* _scgi_hpp__ */
