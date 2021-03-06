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

#ifndef NCFORM_H
#define NCFORM_H

#include "ncwidget.h"

#include <string>
#include <vector>

#include <form.h>

class NCForm;
typedef std::shared_ptr<NCForm> NCFormPtr;

enum NCFormFieldValidation
{
    fvNone,
    fvAlpha,
    fvAlphaNumeric,
    fvNumeric,
    fvRegExp
};

enum NCFormFieldType
{
    ftNormalInput,
    ftCheckbox
};

struct NCFormField
{
    NCFormField() {};
    ~NCFormField() {};

    NCFormFieldType type = ftNormalInput;

    std::string text;
    std::string value;

    NCFormFieldValidation validation = fvNone;

    std::size_t width = 10;

    // Validations

    // AlphaNumeric
    std::size_t max_length = -1;

    // Integer
    int intPadding = 0;
    int minIntValue = 0;
    int maxIntValue = 100;

    // RegExp
    std::string regexp;
};
typedef std::vector<NCFormField> NCFormFieldVector;

struct NCFormButton
{
    std::string text;
    std::function<void()> callback;
};
typedef std::vector<NCFormButton> NCFormButtonVector;

class NCForm: public NCWidget
{
private:
    FORM   *m_form = nullptr;
    WINDOW *m_formWindow = nullptr;

    std::vector<FIELD*> m_ncFields;

    NCFormFieldVector   m_fields;
    NCFormButtonVector  m_buttons;

    unsigned int        m_focusedItem = 0;
    std::string m_title = "Form";

public:
    NCForm(NCWindowRef parent, int x, int y);
    virtual ~NCForm();

    void addFields(NCFormFieldVector fields);
    void addButtons(NCFormButtonVector buttons);

    virtual void redraw() override;
    virtual void setTitle(std::string _title) { m_title = _title; };

    std::string getFieldData(int index);

protected:
    void initialize();
    virtual bool keyEvent(int key);
};


#endif // NCFORM_H
