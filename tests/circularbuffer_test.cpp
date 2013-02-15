
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>

#include "circularbuffer.h"

using namespace std;
using namespace bitforge;

int main()
{
    srandom(clock());

    size_t bufferSize = (random() / (RAND_MAX / 8000)) + 128;

    cout << bufferSize << endl;

    CircularBuffer<char> s_buffer(bufferSize);

    ifstream in("/tmp/testdata.tar");
    ofstream out("/tmp/out_testdata.tar", ios_base::trunc);

    char buffer[81920];

    while(in.good())
    {
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

    return 0;
}
