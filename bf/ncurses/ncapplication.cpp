/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Gianni Rossi <gianni.rossi@gmail.com>
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

#include "ncapplication.h"

#include <unistd.h>
#include <sys/select.h>

NCApplication::NCApplication()
{
    initscr();
    clear();
    noecho();
    cbreak();   /* Line buffering disabled. pass on everything */
    
    m_hasColours = has_colors();
    if (m_hasColours)
    {
        start_color();
        setStyle(stNormal, scWhite, scBlack);
        setStyle(stHighlight, scWhite, scBlue);
    }
}
    
NCApplication::~NCApplication()
{
    endwin();
}
    
void NCApplication::terminate()
{
    m_terminated = true;
}

void NCApplication::addWindow(NCWindowRef window)
{
    m_windows.push_back(std::move(window));
}

bool NCApplication::removeWindow(const NCWindowRef& window)
{
    auto it = m_windows.begin(), end = m_windows.end();
    for(; it != end; it++)
    {
        if (*it == window)
        {
            m_windows.erase(it);

            m_terminated = m_windows.empty();
            if (!m_windows.empty())
                (*m_windows.rbegin())->setNeedRedraw();

            clear();
            refresh();

            return true;
        }
    }
    
    return false;
}

int NCApplication::exec()
{
    clear();
    refresh();
    // First pass to draw the screen
    for(auto window : m_windows)
    {
        window->redraw();
    }
    
    // Main loop
    while(!m_terminated)
    {
        fd_set read, write, except;
        FD_ZERO(&read);
        FD_ZERO(&write);
        FD_ZERO(&except);
        
        FD_SET(STDIN_FILENO, &read);
        
        int maxFd = STDIN_FILENO + 1;
        
        struct timeval timeout = { 1, 0 };
        int ret = select(maxFd, &read, &write, &except, &timeout);
        
        if (ret >= 0)
        {
            if (ret > 0)
            {
                auto window = *m_windows.rbegin();
                window->keyEvent(wgetch(window->m_window));
            }
            
            auto window = *m_windows.rbegin();

            bool needRedraw = false;
            for(auto it : m_windows)
            {
                needRedraw |= it->needRedraw();

                if (needRedraw)
                    window->redraw();
            }
        }
    }
    
    return 0;
}
