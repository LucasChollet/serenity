/*
 * Copyright (c) 2019-2020, Sergey Bugaev <bugaevc@serenityos.org>
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Calculator.h"
#include <AK/Assertions.h>
#include <AK/Math.h>
#include <LibCrypto/BigFraction/BigFraction.h>

void Calculator::clear_operation()
{
    clear_error();

    m_current.clear();
    m_current_operation = Calculation::Operation::None;
}

static bool need_two_steps(Calculation::Operation operation)
{
    return operation == Calculation::Operation::Add || operation == Calculation::Operation::Subtract
        || operation == Calculation::Operation::Multiply || operation == Calculation::Operation::Divide;
}

Crypto::BigFraction Calculator::operate(Calculation::Operation operation, Crypto::BigFraction const& value)
{
    if (m_current.is_empty())
        m_current.add_operation({ value, {}, Calculation::Operation::None });
    if (need_two_steps(operation) || operation == Calculation::Operation::None) {
        if (m_current_operation == Calculation::Operation::None)
            m_current_operation = operation;
        else {
            VERIFY(m_current_operation != Calculation::Operation::None);
            m_current.add_operation({ value, {}, m_current_operation });
            m_current_operation = Calculation::Operation::None;
        }
    } else {
        m_current.add_operation({ {}, {}, operation });
    }
    return m_current.result();
}

Crypto::BigFraction Calculator::operate(MemoryOperation operation, Crypto::BigFraction const& value)
{
    switch (operation) {
    case MemoryOperation::None:
        VERIFY_NOT_REACHED();
    case MemoryOperation::MemClear:
        m_mem.clear();
        break;
    case MemoryOperation::MemRecall:
        m_current = m_mem;
        break;
    case MemoryOperation::MemSave:
        m_mem = m_current;
        break;
    case MemoryOperation::MemAdd:
        m_mem.add_operation({ value, {}, Calculation::Operation::Add });
        break;
    }

    return m_mem.result();
}

void Calculator::set_precision(unsigned int precision)
{
    m_precision = precision;
    m_current.set_precision(m_precision);
    m_mem.set_precision(m_precision);
}
