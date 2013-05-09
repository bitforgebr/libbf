#include "ncstatusbar.h"

#include <string>
#include "ncapplication.h"

NCStatusBar::NCStatusBar(int _x, int _y, int _width, int _height, std::function<std::string()> _statusCalback):
    NCWindow(_x, _y, _width, _height)
{
    m_statusCalback = _statusCalback;

    m_needRedraw = true;
}

NCStatusBar::~NCStatusBar()
{
}

void NCStatusBar::redraw()
{
    std::string status = m_statusCalback();
    wclear(m_window);
    mvwprintw(m_window, 0, 0, "%s", status.c_str());
    wbkgd(m_window, COLOR_PAIR(stHighlight));
    wrefresh(m_window);
    m_needRedraw = true;
}
