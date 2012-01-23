// Copyright(c) Andre Caron <andre.l.caron@gmail.com>, 2011
//
// This document is covered by the an Open Source Initiative approved license. A
// copy of the license should have been provided alongside this software package
// (see "LICENSE.txt"). If not, terms of the license are available online at
// "http://www.opensource.org/licenses/mit".

#include "scgi.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace {

    class Print
    {
        std::ostream& myStream;
    public:
        Print ( std::ostream& stream )
            : myStream(stream)
        {}
        void operator() ( const scgi::Headers::value_type& header )
        {
            myStream
                << header.first << '=' << header.second
                << std::endl;
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

int main ( int argc, char ** argv )
try
{
    scgi::Request request;
    if (argc == 1) {
        std::cin >> request;
    }
    else {
        std::ifstream file(argv[1]);
        if (!file.is_open())
        {
            std::cerr
                << "Could not open input file."
                << std::endl;
            return (EXIT_FAILURE);
        }
        file >> request;
    }
    std::cout
        << request.body()
        << std::endl;
}
catch ( const std::exception& error )
{
    std::cerr
        << error.what()
        << std::endl;
    return (EXIT_FAILURE);
}
catch ( ... )
{
    std::cerr
        << "Unknown error."
        << std::endl;
    return (EXIT_FAILURE);
}
