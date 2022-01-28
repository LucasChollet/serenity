/*
 * Copyright (c) 2019-2020, Sergey Bugaev <bugaevc@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "Calculation.h"
#include <LibCrypto/BigFraction/BigFraction.h>

class CalculatorWidget;

class Calculator final {
public:
    Calculator() = default;
    Calculator(Calculator&&) = default;
    Calculator(Calculator const&) = default;
    Calculator& operator=(Calculator&&) = default;
    Calculator& operator=(Calculator const&) = default;
    ~Calculator() = default;

    enum class MemoryOperation {
        None,
        MemClear,
        MemRecall,
        MemSave,
        MemAdd
    };

    Crypto::BigFraction operate(Calculation::Operation, Crypto::BigFraction const&);
    Crypto::BigFraction operate(MemoryOperation, Crypto::BigFraction const&);

    bool has_error() const { return m_has_error; }

    void set_precision(unsigned precision);

    void clear_operation();
    void clear_error() { m_has_error = false; }

private:
    friend CalculatorWidget;

    unsigned m_precision;
    Calculation m_current;
    Calculation::Operation m_current_operation { Calculation::Operation::None };
    Calculation m_mem;
    bool m_has_error { false };
};
