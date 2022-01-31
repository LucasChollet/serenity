/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "NewtonMethod.h"

namespace Crypto {

NewtonMethod::NewtonMethod(Function&& function, Function&& derivative, unsigned precision, BigFraction const& first_guess)
    : m_y(first_guess)
    , m_y_old(first_guess)
    , m_function(move(function))
    , m_derivative(move(derivative))
    , m_needed_precision(precision)

{
}

void NewtonMethod::set_needed_precision(unsigned precision)
{
    if (precision > m_needed_precision)
        m_needed_precision = precision;
}

BigFraction NewtonMethod::value()
{
    dbgln("Newton: m_y({}), m_y_old({}), prec({}/{})", m_y.to_string(m_needed_precision), m_y_old.to_string(m_needed_precision), m_actual_precision, m_needed_precision);

    bool looping = false;
    // Note: +1 to ensure correct rounding
    while (m_actual_precision <= m_needed_precision + 1) {

        auto const tmp = m_y;
        m_y = m_y_old - (m_function(m_y_old) / m_derivative(m_y_old));
        m_y_old = tmp;
        dbgln("Newton: m_y({}), m_y_old({}), prec({}/{})", m_y.to_string(m_needed_precision + 2), m_y_old.to_string(m_needed_precision + 2), m_actual_precision, m_needed_precision);

        if (m_y == m_y_old) {
            if (looping) {
                warnln("Newton Algorithm is not converging, aborting");
                break;
            }
            looping = true;
            continue;
        }

        looping = false;

        // Note: +2 is a heuristic as the (m_y - m_y_old).abs() isn't a sufficient condition.
        BigFraction const power = BigFraction { NumberTheory::Power("1"_sbigint, SignedBigInteger::create_from(-(m_actual_precision + 2))) };
        if ((m_y - m_y_old).abs() < power && m_function(m_y).abs() < power)
            m_actual_precision++;
    }

    return m_y;
}

}
