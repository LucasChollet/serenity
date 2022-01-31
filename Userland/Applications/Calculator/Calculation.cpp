/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Calculation.h"

void Calculation::add_operation(const NumberAndOperation& pack)
{
    m_operation_list.append(pack);
}

Crypto::BigFraction Calculation::result()
{
    if (m_last_computed_index != m_operation_list.size() - 1)
        compute_result();
    return m_result;
}

void Calculation::set_precision(unsigned precision)
{
    m_needed_precision = precision;
    m_last_computed_index = 0;
}

void Calculation::compute_result()
{
    if (m_operation_list.is_empty())
        return;

    Crypto::BigFraction result {};
    dbgln("Vector list:");
    // TODO: Remove dbgln
    for (unsigned i = m_last_computed_index; i < m_operation_list.size(); ++i) {
        auto& value = m_operation_list[i];

        String val;
        if (value.perfect_value.has_value())
            val = value.perfect_value.value().to_string(1);
        else
            val = "{}";
        dbgln("{}: Value({}), Operation({})", i, val, (int)m_operation_list[i].operation);

        // Note: Only used for Operation::Sqrt
        const auto last_value = [&value, &result]() -> Crypto::BigFraction {
            if (value.approximate_value.has_value())
                return value.approximate_value.value().value();
            return result / Crypto::BigFraction { 2 };
        };

        // Pour z = x * y
        // delta z = x * y (delta x / x + delta y / y)

        switch (m_operation_list[i].operation) {
        case Operation::None:
            VERIFY(i == 0);
            result = value.perfect_value.value();
            break;
        case Operation::Add:
            result += value.perfect_value.value();
            break;
        case Operation::Subtract:
            result -= value.perfect_value.value();
            break;
        case Operation::Multiply:
            result *= value.perfect_value.value();
            break;
        case Operation::Divide:
            result /= value.perfect_value.value();
            break;
        case Operation::Sqrt:
            // Note: This hardcoded value is needed as the Newton algorithm requires
            //  the derivative to be non-null at the evaluation point.
            if (result == Crypto::BigFraction { 0 })
                break;

            value.approximate_value = Crypto::NewtonMethod([=](auto const& x) { return x * x - result; }, [](auto const& x) { return Crypto::BigFraction { 2 } * x; }, m_needed_precision, last_value());
            result = value.approximate_value->value();
            m_operation_list[i].precision = m_needed_precision;
            break;
        case Operation::Inverse:
            result = Crypto::BigFraction { 1 } / result;
            break;
        case Operation::Percent:
            result /= Crypto::BigFraction { 100 };
            break;
        case Operation::ToggleSign:
            result = -result;
            break;
        default:
            VERIFY_NOT_REACHED();
        }
    }

    m_result = result;
}

bool Calculation::is_empty()
{
    return m_operation_list.is_empty();
}

void Calculation::clear()
{
    m_operation_list.clear();
}
