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

#include "scgi.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace {

    class Print
    {
        std::ostream& myStream;
    public:
        Print (std::ostream& stream)
            : myStream(stream)
        {}
        void operator() (const scgi::Headers::value_type& header)
        {
            myStream
                << header.first << '=' << header.second
                << std::endl;
        }
    };

    std::ostream& operator<<
        (std::ostream& stream, const scgi::Headers& headers)
    {
        std::for_each(headers.begin(), headers.end(), Print(stream));
        return (stream);
    }

}

#include <iostream>

int main (int argc, char ** argv)
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
        << request.headers();
}
catch (const std::exception& error)
{
    std::cerr
        << error.what()
        << std::endl;
    return (EXIT_FAILURE);
}
catch (...)
{
    std::cerr
        << "Unknown error."
        << std::endl;
    return (EXIT_FAILURE);
}
