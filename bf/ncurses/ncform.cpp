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

#include "ncform.h"
#include <cassert>

NCForm::NCForm(NCWindowRef parent, int x, int y): 
    NCWidget(parent, x, y)
{
}

NCForm::~NCForm()
{
    unpost_form(m_form);
    free_form(m_form);
    
    for(auto f : m_ncFields)
        free_field(f);
}

void NCForm::addFields(NCFormFieldVector fields)
{
    assert(m_ncFields.empty()); // can't add fields after initialize.
    
    if (m_fields.empty())
        m_fields = std::move(fields);
    else
    {
        for(auto &item : fields)
        {
            m_fields.push_back(std::move(item));
        }
    }
}

void NCForm::addButtons(NCFormButtonVector buttons)
{
    assert(m_ncFields.empty()); // can't add buttons after initialize.
    
    if (m_buttons.empty())
        m_buttons = std::move(buttons);
    else
    {
        for(auto &item : buttons)
        {
            m_buttons.push_back(std::move(item));
        }
    }
}

void NCForm::initialize()
{
    
    /*
     FIELD *new_field(int height, int width,
                        int toprow, int leftcol,     *
                        int offscreen, int nbuffers);
    */

    std::size_t textWidth = 0;
    for(auto &field : m_fields)
        textWidth = std::max(textWidth, field.text.length());
    textWidth += 2;
        
    int x = m_x + textWidth, y = m_y;
    
    m_ncFields.reserve(m_fields.size() + 1);
    
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    
    for(auto &field : m_fields)
    {
        FIELD *f = new_field(1, field.width, y, x, 0, 0);
        
        set_field_back(f, A_UNDERLINE);
        field_opts_off(f, O_AUTOSKIP);   // Don't go to next field when this Field is filled up  
        
        switch(field.validation)
        {
            case fvNone: 
                break;
                
            case fvAlpha:
                set_field_type(f, TYPE_ALPHA, std::max(field.width, field.max_length));
                break;
                
            case fvAlphaNumeric:
                set_field_type(f, TYPE_ALNUM, std::max(field.width, field.max_length));
                break;
                
            case fvNumeric:
                set_field_type(f, TYPE_INTEGER, field.intPadding, field.minIntValue, field.maxIntValue);
                break;
                
            case fvRegExp:
                set_field_type(f, TYPE_REGEXP, field.regexp.c_str());
                break;
        }

        m_ncFields.push_back(f);
        
        y += 2;
    }
    
    m_ncFields.push_back(nullptr);
    
    m_form = new_form(m_ncFields.data());
    
    // Calculate the area required for the form 
    int rows, cols;
    scale_form(m_form, &rows, &cols);
    
    // Create the window to be associated with the form 
    //m_formWindow = newwin(rows + 4, cols + 4, m_y, m_x);
    //keypad(m_formWindow, TRUE);
    m_formWindow = getWindow();
    
    const int formMargin = 1;
    
    // Set main window and sub window 
    set_form_win(m_form, m_formWindow);
    set_form_sub(m_form, derwin(m_formWindow, rows, cols, formMargin, formMargin));
    
    box(m_formWindow, 0, 0);
    
    post_form(m_form);
    
    x = m_x + formMargin, y = m_y + formMargin;
    
    for(auto &field : m_fields)
    {
        mvwprintw(m_formWindow, y, x, "%s", field.text.c_str());
        y += 2;
    }
    
    wrefresh(m_formWindow);
}

void NCForm::redraw()
{
    if (m_ncFields.empty())
        initialize();
    
    int index = 0;
    int x = m_x, y = m_y + (m_fields.size() * 2);
    
    auto window = getWindow();
    box(window, 0, 0);
    
    for(auto &field : m_ncFields)
    {
        if (index == m_focusedItem)
        {
            set_field_fore(field, COLOR_PAIR(2));
            set_field_back(field, COLOR_PAIR(2));
        }
        else
        {
            set_field_fore(field, COLOR_PAIR(4));
            set_field_back(field, COLOR_PAIR(4));
        }
        index++;
    }
    
    for(auto &button : m_buttons)
    {
        if (index == m_focusedItem) wattron(window, A_REVERSE);
        if (!button.text.empty())     mvwprintw(window, y, x, "< %s >", button.text.c_str());
        if (index == m_focusedItem) wattroff(window, A_REVERSE);
        
        index++;
        
        x += button.text.length() + 6;
    }
       
    wrefresh(window);
}

bool NCForm::keyEvent(int key)
{
    switch(key)
    {
        case KEY_UP:
            if (m_focusedItem > 0)
                m_focusedItem--;
            
            /* Go to previous field */
            form_driver(m_form, REQ_PREV_FIELD);
            form_driver(m_form, REQ_END_LINE);
            return true;
            
        case KEY_DOWN:
            if (m_focusedItem < m_fields.size() + m_buttons.size() - 1)
                m_focusedItem++;
            
            /* Go to next field */
            form_driver(m_form, REQ_NEXT_FIELD);
            /* Go to the end of the present buffer */
            /* Leaves nicely at the last character */
            form_driver(m_form, REQ_END_LINE);
            return true;
        
        case 10:
        {
            auto buttonIndex = m_focusedItem - m_fields.size();
            if (buttonIndex >= 0)
            {
                auto fn = m_buttons[buttonIndex].callback;
                if (fn)
                    fn();
            }
            
            return true;
        }
            
        default:
            if (m_focusedItem < m_fields.size())
            {
                /* If this is a normal character, it gets */
                /* Printed                */    
                form_driver(m_form, key);
            }
            return true;
    }
    return NCWidget::keyEvent(key);
}
