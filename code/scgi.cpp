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
        myLimits.max_head_size = 0;
        myLimits.max_body_size = 0;
        ::scgi_setup(&myLimits, &myParser);
        myParser.object = this;
        myParser.accept_field = &Request::accept_field;
        myParser.accept_value = &Request::accept_value;
        myParser.finish_value = &Request::finish_value;
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
        if ( myParser.error != scgi_error_ok ) {
            throw (Error(myParser.error));
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
        request.myField.append(data, size);
    }

    void Request::accept_value
        ( ::scgi_parser* parser, const char * data, size_t size )
    {
        Request& request = *static_cast<Request*>(parser->object);
        request.myValue.append(data, size);
    }

    void Request::finish_value ( ::scgi_parser * parser )
    {
        Request& request = *static_cast<Request*>(parser->object);
        // Pre-parse content length.
        if (::scgi_is_content_length(request.myField.data(),
                                     request.myField.size()))
        {
            if (request.myContentLength != 0) {
                // TODO: Reject multiple definitions.
                std::cerr << "Multiple content lengths." << std::endl;
            }
            const ::ssize_t content_length =
                ::scgi_parse_content_length(request.myValue.data(),
                                            request.myValue.size());
            if (content_length < 0) {
                // TODO: Reject invalid values.
                std::cerr << "Invalid content length." << std::endl;
            }
            else {
                request.myContentLength = content_length;
            }
        }
        // Consume header value.
        request.myHeaders[request.myField] = request.myValue;
        request.myField.clear();
        request.myValue.clear();
    }

    void Request::finish_head ( ::scgi_parser * parser )
    {
        Request& request = *static_cast<Request*>(parser->object);
        request.myState = Body;
    }

    size_t Request::accept_body
        ( ::scgi_parser* parser, const char * data, size_t size )
    {
        Request& request = *static_cast<Request*>(parser->object);
        const size_t used = std::min
            (size, request.myContentLength-request.myContent.size());
        request.myContent.append(data, used);
        if (request.myContent.size() >= request.myContentLength) {
            request.myState = Done;
        }
        return (used);
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
