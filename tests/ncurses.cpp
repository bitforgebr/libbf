
#include <ncurses.h>
#include <boost/concept_check.hpp>

#include <bf/ncurses/ncapplication.h>
#include <bf/ncurses/ncwindow.h>
#include <bf/ncurses/ncmenu.h>
#include <bf/ncurses/ncform.h>

void fn1(NCApplication *app)
{
    mvprintw(24, 0, "Function 1");
    refresh();
    
    auto window = std::make_shared<NCWindow>(4, 4, 40, 20);
    
    auto form = std::make_shared<NCForm>(window, 2, 2);
    
    NCFormFieldVector fields;
    
    NCFormField f;
    
    f.text = "IP:";
    fields.push_back(f);
    
    f.text = "Mask:";
    fields.push_back(f);
    
    f.text = "Gateway:";
    fields.push_back(f);
    
    form->addFields(std::move(fields));
    
    window->addWidget(form);
    
    app->addWindow(window);
}

void fn2()
{
    mvprintw(24, 0, "Function 2");
    refresh();
}

int main()
{   
    NCApplication app;

    {
        auto window = std::make_shared<NCWindow>(2, 2, 20, 20);
        
        NCMenuItemVector menuItems;
        
        NCMenuItem item;
        
        item.text = "Hello";
        item.callbck = std::bind(fn1, &app);;
        menuItems.push_back(item);
        
        item.text = "World";
        item.callbck = fn2;
        menuItems.push_back(item);
        
        item.text = "Exit";
        item.callbck = std::bind(&NCApplication::terminate, &app);
        menuItems.push_back(item);
        
        
        auto menu = std::make_shared<NCMenu>(window);
        menu->addMenuOptions(std::move(menuItems));
        
        window->addWidget(menu);
        
        app.addWindow(window);
    }
    

    return app.exec();
}
