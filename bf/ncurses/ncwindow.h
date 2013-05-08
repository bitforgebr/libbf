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

#ifndef NCWINDOW_H
#define NCWINDOW_H

#include <ncurses.h>

#include <vector>
#include <memory>

class NCWindow;
typedef std::shared_ptr<NCWindow> NCWindowRef;
typedef std::vector<NCWindowRef> NCWindowRefVector;

class NCWidget;
typedef std::shared_ptr<NCWidget> NCWidgetRef;
typedef std::vector<NCWidgetRef> NCWidgetRefVector;

class NCWindow
{
    friend class NCApplication;
    friend class NCWidget;
    
protected:
    WINDOW *m_window = nullptr;
    
    NCWidgetRefVector   m_widgets;
    
    int m_x = 0;
    int m_y = 0;
    
    bool m_needRedraw = true;

    void clearFocus();
    
public:
    NCWindow(int x, int y, int width, int height);
    virtual ~NCWindow();
    
    void addWidget(NCWidgetRef widget);
    
    virtual void redraw();
    virtual bool keyEvent(int key);
    
    int x() const { return m_x; }
    int y() const { return m_y; }

    bool needRedraw() const;
    void setNeedRedraw() { m_needRedraw = true; }
};


#endif // NCWINDOW_H
