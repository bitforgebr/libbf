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

#include "ncwidget.h"
#include "ncwindow.h"

NCWidget::NCWidget(NCWindowRef parent, int x, int y):
    m_parent(parent), m_x(x), m_y(y)
{
    //m_parent->addWidget(shared_from_this());
}

bool NCWidget::keyEvent(int key)
{
    (void)(key);
    return false;
}

void NCWidget::redraw()
{
    m_needRedraw = false;
}

WINDOW* NCWidget::getWindow()
{
    return m_parent->m_window;
}

void NCWidget::setFocus()
{
    m_parent->clearFocus();
    m_hasFocus = true;
}
