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

#ifndef NCMENU_H
#define NCMENU_H

#include "ncwidget.h"

#include <vector>
#include <functional>
#include <string>

struct NCMenuItem
{
    std::string text;
    std::function<void()> callback;
};
typedef std::vector<NCMenuItem> NCMenuItemVector;

class NCMenu: public NCWidget
{
private:
    NCMenuItemVector m_menuItems;
    unsigned int     m_focusedItem = 0;
    NCOrientation    m_orientation = Vertical;
    
    std::size_t      m_margin = 0;
public:
    NCMenu(NCWindowRef parent, NCOrientation orientation = Vertical);
    NCMenu(NCWindowRef parent, int x, int y, NCOrientation orientation = Vertical);
    virtual ~NCMenu();
    
    void addMenuOptions(NCMenuItemVector items);
    virtual void redraw() override;
    
protected:
    virtual bool keyEvent(int key);
};

#endif // NCMENU_H
