
#include <ncurses.h>

#include <bf/ncurses/ncapplication.h>
#include <bf/ncurses/ncwindow.h>
#include <bf/ncurses/ncmenu.h>
#include <bf/ncurses/ncform.h>

void formOK(NCApplication *app, NCWindowRef formWindow)
{
    mvprintw(40, 0, "OK Pressed!");
    app->removeWindow(formWindow);
}

void formClose(NCApplication *app, NCWindowRef formWindow)
{
    mvprintw(40, 0, "Close Pressed!");
    app->removeWindow(formWindow);
}

void fn1(NCApplication *app)
{
    mvprintw(40, 0, "Function 1");
    refresh();
    
    auto window = std::make_shared<NCWindow>(4, 4, 40, 20);
    
    auto form = std::make_shared<NCForm>(window, 2, 2);
    
    NCFormFieldVector fields;
    
    NCFormField f;
    
    f.text = "IP:";
    f.width = 16;
    f.regexp = "[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}";
    f.validation = fvRegExp;
    fields.push_back(f);
    
    f.text = "Mask:";
    fields.push_back(f);
    
    f.text = "Gateway:";
    fields.push_back(f);
    
    form->addFields(std::move(fields));
    
    NCFormButtonVector buttons;
    
    NCFormButton b;
    
    b.text = "OK";
    b.callback = std::bind(formOK, app, window);
    
    buttons.push_back(b);
    
    b.text = "Cancel";
    b.callback = std::bind(formClose, app, window);
    
    buttons.push_back(b);
    
    form->addButtons(std::move(buttons));
    
    form->setTitle("Configurações de Rede");
    
    window->addWidget(form);
    
    app->addWindow(window);
}

void fn2()
{
    mvprintw(40, 0, "Function 2");
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
        item.callback = std::bind(fn1, &app);
        menuItems.push_back(item);
        
        item.text = "World";
        item.callback = fn2;
        menuItems.push_back(item);
        
        item.text = "Exit";
        item.callback = std::bind(&NCApplication::terminate, &app);
        menuItems.push_back(item);
                
        auto menu = std::make_shared<NCMenu>(window);
        menu->addMenuOptions(std::move(menuItems));
        
        window->addWidget(menu);
        
        app.addWindow(window);
    }
    

    return app.exec();
}
