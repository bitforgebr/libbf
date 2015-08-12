/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Gianni Rossi <gianni.rossi@bitforge.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "bfserial.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace bitforge {

BFSerial::BFSerial(const std::string& modemDevice, int baudRate)
{
    /*
    * Open modem device for reading and writing and not as controlling tty
    * because we don't want to get killed if linenoise sends CTRL-C.
    */

    m_fd = FileDescriptor::make(open(modemDevice.c_str(), O_RDWR | O_NOCTTY | O_SYNC));
    if (m_fd.get() < 0)
        throw IOException(errno);

    m_oldtio.reset(new struct termios);
    bzero(m_oldtio.get(), sizeof(struct termios));

    tcgetattr(m_fd.get(), m_oldtio.get()); /* save current serial port settings */

    struct termios newtio;
    bzero(&newtio, sizeof(struct termios)); /* clear struct for new port settings */

    /*
    * BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
    * CRTSCTS : output hardware flow control (only used if the cable has
    *           all necessary lines. See sect. 7 of Serial-HOWTO)
    * CS8     : 8n1 (8bit,no parity,1 stopbit)
    * CLOCAL  : local connection, no modem contol
    * CREAD   : enable receiving characters
    */
    newtio.c_cflag = baudRate | CS8 | CLOCAL | CREAD;

    /*
    * IGNPAR  : ignore bytes with parity errors
    * ICRNL   : map CR to NL (otherwise a CR input on the other computer
    *           will not terminate input)
    * otherwise make device raw (no other input processing)
    */
    newtio.c_iflag = 0;

    /*
    * Raw output.
    */
    newtio.c_oflag = 0;

    /*
    * ICANON  : enable canonical input
    * disable all echo functionality, and don't send signals to calling program
    */
    newtio.c_lflag = ICANON;

    /*
    * initialize all control characters
    * default values can be found in /usr/include/termios.h, and are given
    * in the comments, but we don't need them here
    */
    newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
    newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
    newtio.c_cc[VERASE]   = 0;     /* del */
    newtio.c_cc[VKILL]    = 0;     /* @ */
    newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
    newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
    newtio.c_cc[VSWTC]    = 0;     /* '\0' */
    newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
    newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
    newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z - */
    newtio.c_cc[VEOL]     = 0;     /* '\0' */
    newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
    newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
    newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
    newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
    newtio.c_cc[VEOL2]    = 0;     /* '\0' */

    /*
    * now clean the modem line and activate the settings for the port
    */
    tcflush(m_fd.get(), TCIFLUSH);
    tcsetattr(m_fd.get(), TCSANOW, &newtio);
}

BFSerial::~BFSerial()
{
    if (m_fd.get())
    {
        /* restore the old port settings */
        tcsetattr(m_fd.get(), TCSANOW, m_oldtio.get());
        m_oldtio.reset();
    }
}

ssize_t BFSerial::read(void *buffer, size_t bufferSize)
{
    if (m_readBuffer.empty())
        return BFSimpleFd::read(buffer, bufferSize);
    else
    {
        auto sz = std::min(m_readBuffer.size(), bufferSize);

        memcpy(buffer, m_readBuffer.data(), sz);

        bufferSize -= sz;
        m_readBuffer.erase(m_readBuffer.begin(), m_readBuffer.begin() + sz);

        if (bufferSize > 0)
        {
            buffer = ((char*)buffer) + sz;
            ssize_t remain = BFSimpleFd::read(buffer, bufferSize);

            if (remain < 0)
                return remain;

            sz += remain;
        }

        return sz;
    }
}

std::string BFSerial::readLine()
{
    std::string result;

    if (!m_readBuffer.empty())
    {
        size_t end = m_readBuffer.size();
        for(size_t i = 0; i != end; i++)
        {
            if (m_readBuffer[i] == '\n')
            {
                result = std::string(m_readBuffer.data(), i + 1);
                m_readBuffer.erase(m_readBuffer.begin(), m_readBuffer.begin() + i);
                return result;
            }
        }

        result = std::string(m_readBuffer.data(), end);
        m_readBuffer.clear();
    }

    while(true)
    {
        char buffer[1024];
        auto sz = BFSimpleFd::read(buffer, sizeof(buffer));

        if (sz <= 0)
            return result;

        for(int i = 0; i != sz; i++)
        {
            if (buffer[i] == '\n')
            {
                int lb = 0;
                while(buffer[i - lb] == '\r' || buffer[i - lb] == '\n')
                    lb++;

                size_t rsz = (i - lb) + 1;
                if (rsz > 0)
                    result = std::string(buffer, rsz);

                size_t remain = sz - (i + 1);
                if (remain > 0)
                {
                    m_readBuffer.resize(remain);
                    memcpy(m_readBuffer.data(), &buffer[i + 1], remain);
                }

                return result;
            }
        }

        result.append(buffer, sz);
    }
}

}
