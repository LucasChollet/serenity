/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Vector.h>
#include <LibCrypto/BigFraction/Algorithms/NewtonMethod.h>
#include <LibCrypto/BigFraction/BigFraction.h>

class Calculation {
public:
    Calculation() = default;
    Calculation(Calculation&&) = default;
    Calculation(Calculation const&) = default;
    Calculation& operator=(Calculation&&) = default;
    Calculation& operator=(Calculation const&) = default;
    ~Calculation() = default;

    enum class Operation {
        None,
        Add,
        Subtract,
        Multiply,
        Divide,

        Sqrt,
        Inverse,
        Percent,
        ToggleSign,
    };

    struct NumberAndOperation {
        Optional<Crypto::BigFraction> perfect_value {};
        Optional<Crypto::NewtonMethod> approximate_value {};

        Operation operation;
        unsigned precision {};
    };

    bool is_empty();
    void set_precision(unsigned);
    void clear();

    void add_operation(NumberAndOperation const&);
    Crypto::BigFraction result();

private:
    void compute_result();

    Vector<NumberAndOperation> m_operation_list {};

    unsigned m_needed_precision {};

    Crypto::BigFraction m_result {};
    unsigned m_last_computed_index {};
};
