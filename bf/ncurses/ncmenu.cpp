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

#include "ncmenu.h"
#include "ncwindow.h"

NCMenu::NCMenu(NCWindowRef parent, NCOrientation orientation):
    NCWidget(parent, parent->x(), parent->y()),
    m_orientation(orientation)
{
}

NCMenu::NCMenu(NCWindowRef parent, int x, int y, NCOrientation orientation):
    NCWidget(parent, x, y),
    m_orientation(orientation)
{
}

NCMenu::~NCMenu()
{
}

void NCMenu::addMenuOptions(NCMenuItemVector items)
{
    if (m_menuItems.empty())
        m_menuItems = std::move(items);
    else
    {
        for(NCMenuItem &item : items)
        {
            m_menuItems.push_back(std::move(item));
        }
    }
    
    if (m_orientation == Horizontal)
    {
        m_margin = 0;
        
        for(NCMenuItem &item : m_menuItems)
            m_margin = std::max(m_margin, item.text.length());
        
        m_margin += 2;
    }
}

void NCMenu::redraw()
{
    unsigned int index = 0;
    int x = m_x, y = m_y;
    
    auto window = getWindow();
    box(window, 0, 0);
    
    for(NCMenuItem& item : m_menuItems)
    {
        if (index == m_focusedItem) wattron(window, A_REVERSE);
        if (!item.text.empty())     mvwprintw(window, y, x, "%s", item.text.c_str());
        if (index == m_focusedItem) wattroff(window, A_REVERSE);
        
        index++;
        
        if (m_orientation == Vertical)
            y++;
        else
            x += m_margin;
    }

    m_needRedraw = false;
}

bool NCMenu::keyEvent(int key)
{
    switch(key)
    {   
        case KEY_LEFT:
        {
            if (m_orientation == Horizontal)
            {
                if (m_focusedItem > 0)
                {
                    m_focusedItem--;
                    m_needRedraw = true;
                }
                return true;
            }
            break;
        }
        
        case KEY_RIGHT:
        {
            if (m_orientation == Horizontal)
            {
                if (m_focusedItem < m_menuItems.size() - 1)
                {
                    m_focusedItem++;
                    m_needRedraw = true;
                }
                return true;
            }
            break;
        }
        
        case KEY_UP:
        {
            if (m_orientation == Vertical)
            {
                if (m_focusedItem > 0)
                {
                    m_focusedItem--;
                    m_needRedraw = true;
                }
                return true;
            }
            break;
        }
        
        case KEY_DOWN:
        {
            if (m_orientation == Vertical)
            {
                if (m_focusedItem < m_menuItems.size() - 1)
                {
                    m_focusedItem++;
                    m_needRedraw = true;
                }
                return true;
            }
            break;
        }
        
        case 10:
        {
            auto fn = m_menuItems[m_focusedItem].callback;
            if (fn) fn();
            return true;
        }
    }
    
    return false;
}
