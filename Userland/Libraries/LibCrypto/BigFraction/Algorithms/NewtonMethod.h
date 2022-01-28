/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <LibCrypto/BigFraction/BigFraction.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>

namespace Crypto {

class NewtonMethod {
public:
    using Function = AK::Function<BigFraction(BigFraction)>;
    NewtonMethod(Function&& function, Function&& derivative, unsigned precision, BigFraction const& first_guess = {});
    NewtonMethod(NewtonMethod const&) = delete;
    NewtonMethod(NewtonMethod&&) = default;
    NewtonMethod& operator=(NewtonMethod const&) = delete;
    NewtonMethod& operator=(NewtonMethod&&) = default;
    ~NewtonMethod() = default;

    void set_needed_precision(unsigned precision);

    BigFraction value();

private:
    BigFraction m_y {};
    BigFraction m_y_old {};

    Function m_function;
    Function m_derivative;
    unsigned m_actual_precision {};
    unsigned m_needed_precision {};
};

}
