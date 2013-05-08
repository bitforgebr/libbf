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

#ifndef NCAPPLICATION_H
#define NCAPPLICATION_H

#include <ncurses.h>
#include "ncwindow.h"
#include <map>

enum NCStyleType {
    stNormal = 1,
    stHighlight
};

enum NCStyleColor {
    scBlack = COLOR_BLACK,
    scWhite = COLOR_WHITE,
    scBlue = COLOR_BLUE
};

typedef std::map<NCStyleType, std::pair<NCStyleColor, NCStyleColor>> NCStyleMap;

class NCApplication
{
private:
    bool m_terminated = false;
    bool m_hasColours = false;
    
    NCWindowRefVector m_windows;
    
    NCStyleMap m_styleMap;
    
public:
    NCApplication();
    virtual ~NCApplication();
    
    void setStyle(NCStyleType _type, NCStyleColor _color1, NCStyleColor _color2)
    {
        m_styleMap[_type] = std::make_pair(_color1, _color2);
        init_pair(_type, _color1, _color2);
    }
    
    void terminate();
    
    void addWindow(NCWindowRef window);
    bool removeWindow(const NCWindowRef &window);
    
    int exec();
};

#endif // NCAPPLICATION_H
