#ifndef NCSTATUSBAR_H
#define NCSTATUSBAR_H

#include "ncwindow.h"

class NCStatusBar : public NCWindow
{
public:
    NCStatusBar(int _x, int _y, int _width, int _height, std::function<std::string()> _statusCalback);
    virtual ~NCStatusBar();
    virtual void redraw() override;
private:
    std::function<std::string()> m_statusCalback;

};

#endif // NCSTATUSBAR_H
