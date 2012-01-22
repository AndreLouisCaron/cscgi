// Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "scgi.hpp"

#include <algorithm>
#include <iostream>

namespace {

    const char DATA[] = "70:"
        "CONTENT_LENGTH\0" "27\0"
        "SCGI\0" "1\0"
        "REQUEST_METHOD\0" "POST\0"
        "REQUEST_URI\0" "/deepthought\0"
        ","
        "What is the answer to life?"
        ;
    const size_t SIZE = sizeof(DATA);

    class Print
    {
        std::ostream& myStream;
    public:
        Print ( std::ostream& stream )
            : myStream(stream)
        {}
        void operator() ( const scgi::Headers::value_type& header )
        {
            myStream << header.first << "='" << header.second << "', ";
        }
    };

    std::ostream& operator<<
        ( std::ostream& stream, const scgi::Headers& headers )
    {
        std::for_each(headers.begin(), headers.end(), Print(stream));
        return (stream);
    }

}

#include <iostream>

int main ( int, char ** )
try
{
    scgi::Request parser;
    std::cout
        << "Used: '" << parser.feed(DATA, SIZE) << "'."
        << std::endl;
    std::cout
        << "Head: " << parser.headers()
        << std::endl;
    std::cout
        << "Head: REQUEST_METHOD='" << parser.header("REQUEST_METHOD") << "'."
        << std::endl;
    std::cout
        << "Body: '" << parser.body() << "'."
        << std::endl;
}
catch ( const std::exception& error )
{
    std::cerr
        << "Fail: '" << error.what() << "'."
        << std::endl;
    return (EXIT_FAILURE);
}
