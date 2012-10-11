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
    const size_t SIZE = sizeof(DATA) - 1;

    class Print
    {
        std::ostream& myStream;
    public:
        Print (std::ostream& stream)
            : myStream(stream)
        {}
        void operator() (const scgi::Headers::value_type& header)
        {
            myStream << header.first << "='" << header.second << "', ";
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

int main (int, char **)
try
{
    scgi::Request parser;
    std::cout
        << "Size: '" << SIZE << "'."
        << std::endl;
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
catch (const std::exception& error)
{
    std::cerr
        << "Fail: '" << error.what() << "'."
        << std::endl;
    return (EXIT_FAILURE);
}
