// Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "scgi.hpp"
#include <iostream>
#include <sstream>

namespace scgi {

    Request::Request ()
        : myState(Null),
          myContentLength(0)
    {
        ::scgi_setup(&myLimits, &myParser);
        myParser.object = this;
        myParser.accept_field = &Request::accept_field;
        myParser.accept_value = &Request::accept_value;
        myParser.finish_head = &Request::finish_head;
        myParser.accept_body = &Request::accept_body;
    }

    void Request::clear ()
    {
        myContent.clear();
        ::scgi_clear(&myParser);
        myState = Null;
        myContentLength = 0;
    }

    size_t Request::feed ( const char * data, size_t size )
    {
        const size_t used =
            ::scgi_consume(&myLimits, &myParser, data, size);
        if ( myParser.state == scgi_parser_fail )
        {
            const char * message =
                ::scgi_error_message(myParser.error);
            throw (std::exception(message));
        }
        return (used);
    }

    const Headers& Request::headers () const
    {
        return (myHeaders);
    }

    bool Request::hasheader ( const std::string& field ) const
    {
        const Headers::const_iterator match = myHeaders.find(field);
        return ((match != myHeaders.end()) && !match->second.empty());
    }

    const std::string Request::header ( const std::string& field ) const
    {
        const Headers::const_iterator match = myHeaders.find(field);
        if ( match == myHeaders.end() ) {
            return ("");
        }
        return (match->second);
    }

    const std::string& Request::body () const
    {
        return (myContent);
    }

    bool Request::head_complete () const
    {
        return (myState >= Body);
    }

    bool Request::body_complete () const
    {
        return (myState == Done);
    }

    std::size_t Request::body_size () const
    {
        return (myContentLength);
    }

    void Request::accept_field
        ( ::scgi_parser* parser, const char * data, size_t size )
    {
        Request& request = *static_cast<Request*>(parser->object);
        if ( !request.myValue.empty() ) {
            request.myHeaders[request.myField] = request.myValue;
            request.myField.clear();
            request.myValue.clear();
        }
        request.myField.append(data, size);
    }

    void Request::accept_value
        ( ::scgi_parser* parser, const char * data, size_t size )
    {
        Request& request = *static_cast<Request*>(parser->object);
        request.myValue.append(data, size);
    }

    void Request::finish_head ( ::scgi_parser * parser )
    {
        Request& request = *static_cast<Request*>(parser->object);
        request.myHeaders[request.myField] = request.myValue;
        request.myField.clear();
        request.myValue.clear();
        request.myState = Body;
        // Pre-parse content length.
        const std::string content_length = request.header("CONTENT_LENGTH");
        if (content_length.empty()) {
            request.myContentLength = 0;
        }
        else {
            std::istringstream parser(content_length);
            if (!(parser >> request.myContentLength)) {
                // error.
            }
        }
    }

    void Request::accept_body
        ( ::scgi_parser* parser, const char * data, size_t size )
    {
        Request& request = *static_cast<Request*>(parser->object);
        size = std::max
            (size, request.myContentLength-request.myContent.size());
        request.myContent.append(data, size);
        if (request.myContent.size() >= request.myContentLength) {
            request.myState = Done;
        }
    }

    std::istream& operator>> ( std::istream& stream, Request& request )
    {
        request.clear();
        char data[1024];
        do {
            stream.read(data, sizeof(data));
            request.feed(data, stream.gcount());
        }
        while (!request.body_complete() && (stream.gcount() > 0));
        return (stream);
    }

}