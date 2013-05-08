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

#ifndef NCWIDGET_H
#define NCWIDGET_H

#include <memory>
#include <ncurses.h>

class NCWindow;
typedef std::shared_ptr<NCWindow> NCWindowRef;

enum NCOrientation
{
    Horizontal,
    Vertical
};

class NCWidget
{
    friend class NCWindow;
    
protected:
    NCWindowRef m_parent;
    bool        m_hasFocus = false;
    bool        m_needRedraw = true;
    
    int m_x = 0;
    int m_y = 0;
    
    void clearFocus()
    {
        m_hasFocus = false;
    }
    
public:
    NCWidget(NCWindowRef parent, int x, int y);
    
    virtual void redraw();
    virtual bool keyEvent(int key);
    
    bool hasFocus() const
    {
        return m_hasFocus;
    }
        
    void setFocus();
    
    WINDOW* getWindow();

    bool needRedraw() const { return m_needRedraw; }
};

#endif // NCWIDGET_H
