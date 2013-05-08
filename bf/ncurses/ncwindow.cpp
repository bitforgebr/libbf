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

#include "ncwindow.h"
#include "ncwidget.h"

NCWindow::NCWindow(int x, int y, int width, int height):
    m_x(x), m_y(y)
{
    m_window = newwin(height, width, x, y);
    keypad(m_window, TRUE);
}
    
NCWindow::~NCWindow()
{
    delwin(m_window);
}

void NCWindow::clearFocus()
{
    for(NCWidgetRef widget : m_widgets)
    {
        if (widget->hasFocus())
            widget->clearFocus();
    }
}

void NCWindow::addWidget(NCWidgetRef widget)
{
    if (m_widgets.empty())
        widget->setFocus();
    
    m_widgets.push_back(widget);
}

void NCWindow::redraw()
{
    for(NCWidgetRef widget : m_widgets)
        widget->redraw();
    
    wrefresh(m_window);
}

bool NCWindow::keyEvent(int key)
{
    for(NCWidgetRef widget : m_widgets)
    {
        if (widget->hasFocus())
        {
            return widget->keyEvent(key);
        }
    }
    return false;
}

bool NCWindow::needRedraw() const
{
    for(auto widget : m_widgets)
        if (widget->needRedraw())
            return true;
        
    return false;
}

