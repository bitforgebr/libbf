#include <gtest/gtest.h>

#include "../bf/buffers.h"

#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>

#include <cstdio>

using namespace std;
using namespace bitforge;

std::string getBiggestBinInDir()
{
    auto p = popen("ls -1s /usr/bin/* /bin/* | sort -nr | head -n 1 | awk '{print $2}'", "r");
    
    usleep(100);
    
    char buffer[8192];
    
    int read = fread(buffer, 1, sizeof(buffer), p);
    
    if (buffer[read - 1] == '\n')
        read--;
    
    buffer[read] = 0;
    
    
    pclose(p);
    
    return buffer;
}

TEST(Buffers, CircularBuffers)
{
    srandom(clock());

    size_t bufferSize = (random() / (RAND_MAX / 8000)) + 128;

    cout << bufferSize << endl;

    CircularBuffer<char> s_buffer(bufferSize);

    ifstream in(getBiggestBinInDir());
    fstream out("/tmp/out_testdata", ios_base::trunc);

    while(in.good())
    {
        char buffer[81920];
        
        size_t sz = (random() / (RAND_MAX / 800)) + 128;

        if (random() % 2)
        {
            if (sz > s_buffer.availableWriteSize())
            {
                cout << "No write space avail: " << sz << " < " << s_buffer.availableWriteSize() << endl;
                continue;
            }

            in.read(buffer, sz);
            s_buffer.push(buffer, in.gcount());
        }
        else
        {
            if (sz > s_buffer.availableReadSize())
            {
                cout << "No read bytes avail: " << sz << " < " << s_buffer.availableReadSize() << endl;
                continue;
            }

            char buffer[sz];

            size_t write = s_buffer.pop(buffer, sz);
            out.write(buffer, write);
        }
    }

    while(s_buffer.availableReadSize())
    {
        size_t sz = (random() / (RAND_MAX / 800)) + 128;
        char buffer[sz];

        size_t write = s_buffer.pop(buffer, sz);
        out.write(buffer, write);
    }
    
    in.seekg(0);
    out.seekg(0);
    
    while(in.good() && out.good())
    {
        static const int bufferSize = 81920;
        char bufferIn[bufferSize], bufferOut[bufferSize];
        
        zero_init(bufferIn);
        zero_init(bufferOut);
        
        in.read(bufferIn, bufferSize - 1);
        out.read(bufferOut, bufferSize - 1);
        
        ASSERT_STREQ(bufferIn, bufferOut);
    }
    
    ASSERT_EQ(in.good(), out.good());
}
