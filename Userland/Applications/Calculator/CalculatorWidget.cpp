/*
 * Copyright (c) 2019-2020, Sergey Bugaev <bugaevc@serenityos.org>
 * Copyright (c) 2021, Glenford Williams <gw_dev@outlook.com>
 * Copyright (c) 2021, Max Wipfli <mail@maxwipfli.ch>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "CalculatorWidget.h"
#include <Applications/Calculator/CalculatorGML.h>
#include <LibCrypto/BigFraction/BigFraction.h>
#include <LibGUI/Button.h>
#include <LibGUI/Label.h>
#include <LibGUI/TextBox.h>
#include <LibGfx/Font.h>
#include <LibGfx/Palette.h>

CalculatorWidget::CalculatorWidget()
{
    load_from_gml(calculator_gml);

    m_entry = *find_descendant_of_type_named<GUI::TextBox>("entry_textbox");
    m_entry->set_relative_rect(5, 5, 244, 26);
    m_entry->set_text_alignment(Gfx::TextAlignment::CenterRight);

    m_label = *find_descendant_of_type_named<GUI::Label>("label");

    m_label->set_frame_shadow(Gfx::FrameShadow::Sunken);
    m_label->set_frame_shape(Gfx::FrameShape::Container);
    m_label->set_frame_thickness(2);

    for (int i = 0; i < 10; i++) {
        m_digit_button[i] = *find_descendant_of_type_named<GUI::Button>(String::formatted("{}_button", i));
        add_digit_button(*m_digit_button[i], i);
    }

    m_mem_add_button = *find_descendant_of_type_named<GUI::Button>("mem_add_button");
    add_operation_button(*m_mem_add_button, Calculator::MemoryOperation::MemAdd);

    m_mem_save_button = *find_descendant_of_type_named<GUI::Button>("mem_save_button");
    add_operation_button(*m_mem_save_button, Calculator::MemoryOperation::MemSave);

    m_mem_recall_button = *find_descendant_of_type_named<GUI::Button>("mem_recall_button");
    add_operation_button(*m_mem_recall_button, Calculator::MemoryOperation::MemRecall);

    m_mem_clear_button = *find_descendant_of_type_named<GUI::Button>("mem_clear_button");
    add_operation_button(*m_mem_clear_button, Calculator::MemoryOperation::MemClear);

    m_clear_button = *find_descendant_of_type_named<GUI::Button>("clear_button");
    m_clear_button->on_click = [this](auto) {
        m_keypad.set_to_0();
        m_calculator.clear_operation();
        update_display();
    };

    m_clear_error_button = *find_descendant_of_type_named<GUI::Button>("clear_error_button");
    m_clear_error_button->on_click = [this](auto) {
        m_keypad.set_to_0();
        update_display();
    };

    m_backspace_button = *find_descendant_of_type_named<GUI::Button>("backspace_button");
    m_backspace_button->on_click = [this](auto) {
        m_keypad.type_backspace();
        update_display();
    };

    m_decimal_point_button = *find_descendant_of_type_named<GUI::Button>("decimal_button");
    m_decimal_point_button->on_click = [this](auto) {
        m_keypad.type_decimal_point();
        update_display();
    };

    m_sign_button = *find_descendant_of_type_named<GUI::Button>("sign_button");
    add_operation_button(*m_sign_button, Calculation::Operation::ToggleSign);

    m_add_button = *find_descendant_of_type_named<GUI::Button>("add_button");
    add_operation_button(*m_add_button, Calculation::Operation::Add);

    m_subtract_button = *find_descendant_of_type_named<GUI::Button>("subtract_button");
    add_operation_button(*m_subtract_button, Calculation::Operation::Subtract);

    m_multiply_button = *find_descendant_of_type_named<GUI::Button>("multiply_button");
    add_operation_button(*m_multiply_button, Calculation::Operation::Multiply);

    m_divide_button = *find_descendant_of_type_named<GUI::Button>("divide_button");
    add_operation_button(*m_divide_button, Calculation::Operation::Divide);

    m_sqrt_button = *find_descendant_of_type_named<GUI::Button>("sqrt_button");
    add_operation_button(*m_sqrt_button, Calculation::Operation::Sqrt);

    m_inverse_button = *find_descendant_of_type_named<GUI::Button>("inverse_button");
    add_operation_button(*m_inverse_button, Calculation::Operation::Inverse);

    m_percent_button = *find_descendant_of_type_named<GUI::Button>("mod_button");
    add_operation_button(*m_percent_button, Calculation::Operation::Percent);

    m_equals_button = *find_descendant_of_type_named<GUI::Button>("equal_button");
    m_equals_button->on_click = [this](auto) {
        Crypto::BigFraction res = m_calculator.operate(Calculation::Operation::None, m_keypad.value());
        m_keypad.set_value(res);
        update_display();
    };
}

CalculatorWidget::~CalculatorWidget()
{
}

template<typename Operation>
void CalculatorWidget::add_operation_button(GUI::Button& button, Operation operation)
{
    button.on_click = [this, operation](auto) {
        Crypto::BigFraction res = m_calculator.operate(operation, m_keypad.value());
        m_keypad.set_value(move(res));
        update_display();
    };
}

void CalculatorWidget::add_digit_button(GUI::Button& button, int digit)
{
    button.on_click = [this, digit](auto) {
        auto const nb = m_keypad.type_digit(digit);
        if (!m_calculator.m_current.is_empty())
            m_calculator.m_current.clear();
        update_rounding(nb);
        update_display();
    };
}

String CalculatorWidget::get_entry()
{
    return m_entry->text();
}

void CalculatorWidget::set_entry(Crypto::BigFraction value)
{
    m_keypad.set_value(value);
    update_display();
}

void CalculatorWidget::update_display()
{
    m_entry->set_text(m_keypad.to_string());
    if (m_calculator.has_error())
        m_label->set_text("E");
    else
        m_label->set_text("");
}

void CalculatorWidget::keydown_event(GUI::KeyEvent& event)
{
    // Clear button selection when we are typing
    m_equals_button->set_focus(true);
    m_equals_button->set_focus(false);

    if (event.key() == KeyCode::Key_Return || event.key() == KeyCode::Key_Equal) {
        m_keypad.set_value(m_calculator.operate(Calculation::Operation::None, m_keypad.value()));
    } else if (event.code_point() >= '0' && event.code_point() <= '9') {
        auto const nb = m_keypad.type_digit(event.code_point() - '0');
        if (!m_calculator.m_current.is_empty())
            m_calculator.m_current.clear();
        update_rounding(nb);
    } else if (event.code_point() == '.') {
        m_keypad.type_decimal_point();
    } else if (event.key() == KeyCode::Key_Escape) {
        m_keypad.set_to_0();
        m_calculator.clear_operation();
    } else if (event.key() == KeyCode::Key_Backspace) {
        m_keypad.type_backspace();
    } else {
        switch (event.code_point()) {
        case '+':
            m_add_button->click();
            break;
        case '-':
            m_subtract_button->click();
            break;
        case '*':
            m_multiply_button->click();
            break;
        case '/':
            m_divide_button->click();
            break;
        case '%':
            m_percent_button->click();
            break;
        default:
            return;
        }
    }

    update_display();
}

void CalculatorWidget::shrink(unsigned shrink_threshold)
{
    m_keypad.shrink(shrink_threshold);
    update_display();
}

void CalculatorWidget::set_precision(unsigned precision)
{
    m_keypad.set_precision(precision);
    m_calculator.set_precision(precision);
    m_keypad.set_value(m_calculator.m_current.result());
    update_display();
}

void CalculatorWidget::set_rounding_custom(GUI::Action& action)
{
    m_rounding_custom = action;
}

void CalculatorWidget::update_rounding(unsigned needed_rounding)
{
    if (m_keypad.rounding_length() < needed_rounding) {
        m_rounding_custom->set_text(String::formatted("&Custom: {}", needed_rounding));
        m_rounding_custom->set_checked(true);
        set_precision(needed_rounding);
    }
}
