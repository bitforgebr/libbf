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

#ifndef BFSERIAL_H
#define BFSERIAL_H

#include <bf/bfio.h>

#include <termios.h>

#include <vector>

namespace bitforge {

class BFSerial : public BFSimpleFd
{
private:
    std::unique_ptr<struct termios> m_oldtio;
    std::vector<char> m_readBuffer;

public:
    BFSerial(const std::string& modemDevice, int baudRate = B1152000);
    virtual ~BFSerial();

    virtual ssize_t read(void *buffer, size_t bufferSize);
    virtual std::string readLine();

    BFIO& operator>>(std::string &str)
    {
        str = readLine();
        return *this;
    }

    enum SerialSigs
    {
        SIGSUSP = 032,
    };
};

}

#endif // BFSERIAL_H
