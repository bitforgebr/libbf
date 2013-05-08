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
        
        init_pair(1, COLOR_BLUE, COLOR_BLACK);
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
        auto it = m_windows.rbegin(), end = m_windows.rend();
        for(; it != end; it++)
        {
            auto window = *it;
            
            if (window->keyEvent(wgetch(window->m_window)))
            {
                if (m_hasColours)
                    attron(COLOR_PAIR(1));
                    
                window->redraw();
                refresh();
                
                if (m_hasColours)
                    attroff(COLOR_PAIR(1));
                
                break;
            }
        }
    }
    
    return 0;
}
