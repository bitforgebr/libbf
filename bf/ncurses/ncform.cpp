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
    int x = m_x, y = m_y;
    
    /*
     FIELD *new_field(int height, int width,
                        int toprow, int leftcol,     *
                        int offscreen, int nbuffers);
    */

    m_ncFields.reserve(m_fields.size() + 1);
    
    for(auto &field : m_fields)
    {
        FIELD *f = new_field(1, field.width, y, x + 10, 0, 0);
        
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
        
        mvprintw(y, x, field.text.c_str());
        
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
    
    // Set main window and sub window 
    set_form_win(m_form, getWindow());
    set_form_sub(m_form, derwin(getWindow(), rows, cols, 2, 2));
    
    box(m_formWindow, 0, 0);
    
    post_form(m_form);
    refresh();
    
    x = m_x, y = m_y;
      
    for(auto &field : m_fields)
    {
        mvprintw(y, x, field.text.c_str());
        y += 2;
    }
    refresh();
}

void NCForm::redraw()
{
    if (m_ncFields.empty())
        initialize();
    
    wrefresh(m_formWindow);
}

bool NCForm::keyEvent(int key)
{
    switch(key)
    {
        case KEY_DOWN:
            /* Go to next field */
            form_driver(m_form, REQ_NEXT_FIELD);
            /* Go to the end of the present buffer */
            /* Leaves nicely at the last character */
            form_driver(m_form, REQ_END_LINE);
            return true;
        case KEY_UP:
            /* Go to previous field */
            form_driver(m_form, REQ_PREV_FIELD);
            form_driver(m_form, REQ_END_LINE);
            return true;
        default:
            /* If this is a normal character, it gets */
            /* Printed                */    
            form_driver(m_form, key);
            return true;
    }
    return NCWidget::keyEvent(key);
}
