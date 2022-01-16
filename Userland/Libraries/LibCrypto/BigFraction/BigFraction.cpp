/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "BigFraction.h"
#include <AK/Math.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>

namespace Crypto {

BigFraction::BigFraction(Crypto::SignedBigInteger const& numerator, Crypto::UnsignedBigInteger const& denominator)
    : m_numerator(numerator)
    , m_denominator(m_numerator == Crypto::SignedBigInteger { 0 } ? 1 : denominator)
{
    VERIFY(m_denominator != 0);
    reduce();
}

BigFraction::BigFraction(Crypto::SignedBigInteger const& value)
    : BigFraction(value, 1)
{
}

BigFraction::BigFraction(StringView sv)
{
    String str = sv.to_string(); // TODO: Once we have a StringView equivalent for this C API, we won't need to create a copy for this anymore.
    size_t first_index = 0;
    char* dot_ptr;
    auto const int_part = Crypto::SignedBigInteger::create_from(strtoll(&str[first_index], &dot_ptr, 10));
    size_t dot_index = dot_ptr - str.characters();
    if ((dot_index < str.length()) && (str[dot_index] == '.')) {
        size_t second_index = dot_index + 1;
        char* end_ptr;
        auto const frac_part = Crypto::SignedBigInteger::create_from(strtoll(&str[second_index], &end_ptr, 10));
        size_t end_index = end_ptr - str.characters();
        auto const frac_length = Crypto::UnsignedBigInteger::create_from(end_index - second_index);
        *this = BigFraction { int_part } + BigFraction { frac_part, frac_length };
    } else {
        *this = BigFraction { int_part };
    }
};

BigFraction BigFraction::operator+(BigFraction const& rhs) const
{
    if (rhs.m_numerator == "0"_bigint)
        return *this;

    auto tmp = *this;
    tmp.m_numerator.set_to(m_numerator.multiplied_by(rhs.m_denominator).plus(rhs.m_numerator.multiplied_by(m_denominator)));
    tmp.m_denominator.set_to(m_denominator.multiplied_by(rhs.m_denominator));

    tmp.reduce();

    return tmp;
}

BigFraction BigFraction::operator-(BigFraction const& rhs) const
{
    return *this + (-rhs);
}

BigFraction BigFraction::operator*(BigFraction const& rhs) const
{
    auto tmp = *this;
    tmp.m_numerator.set_to(tmp.m_numerator.multiplied_by(rhs.m_numerator));
    tmp.m_denominator.set_to(tmp.m_denominator.multiplied_by(rhs.m_denominator));

    tmp.reduce();

    return tmp;
}

BigFraction BigFraction::operator-(void) const
{
    return { m_numerator.negated_value(), m_denominator };
}

BigFraction BigFraction::invert(void) const
{
    return BigFraction { 1 } / *this;
}

BigFraction BigFraction::operator/(BigFraction const& rhs) const
{
    VERIFY(rhs.m_numerator != "0"_bigint);

    auto tmp = *this;
    tmp.m_numerator.set_to(m_numerator.multiplied_by(rhs.m_denominator));
    tmp.m_denominator.set_to(m_denominator.multiplied_by(rhs.m_numerator.unsigned_value()));

    if (rhs.m_numerator.is_negative())
        tmp.m_numerator.negate();

    tmp.reduce();

    return tmp;
}

bool BigFraction::operator<(BigFraction const& rhs) const
{
    return (*this - rhs).m_numerator.is_negative();
}

bool BigFraction::operator==(BigFraction const& rhs) const
{
    return m_numerator == rhs.m_numerator && m_denominator == rhs.m_denominator;
}

BigFraction::BigFraction(double d)
{
    bool negative = false;
    if (d < 0) {
        negative = true;
        d = -d;
    }
    i8 current_pow = 0;
    while (AK::pow(10.0, (double)current_pow) <= d)
        current_pow += 1;
    current_pow -= 1;
    double epsilon = 1e-6;
    unsigned decimal_places = 0;
    while (d >= epsilon || current_pow >= 0) {
        m_numerator.set_to(m_numerator.multiplied_by(Crypto::SignedBigInteger { 10 }));
        i8 digit = (u64)(d * AK::pow(0.1, (double)current_pow)) % 10;
        m_numerator.set_to(m_numerator.plus(Crypto::UnsignedBigInteger::create_from(digit)));
        d -= digit * AK::pow(10.0, (double)current_pow);
        if (current_pow < 0) {
            ++decimal_places;
            m_denominator.set_to(Crypto::NumberTheory::Power("10"_bigint, Crypto::UnsignedBigInteger::create_from(decimal_places)));
        }
        current_pow -= 1;
        if (decimal_places > 6)
            break;
    }
    m_numerator.set_to(negative ? (m_numerator.negated_value()) : m_numerator);
}

double BigFraction::to_double() const
{
    // FIXME: very naive implementation
    return m_numerator.to_double() / m_denominator.to_double();
}

void BigFraction::set_to_0()
{
    m_numerator.set_to_0();
    m_denominator.set_to(1);
}

BigFraction BigFraction::rounded(unsigned rounding_threshold) const
{
    auto const get_last_digit = [](auto const& integer) {
        return integer.divided_by("10"_bigint).remainder;
    };

    auto const res = m_numerator.divided_by(m_denominator);
    BigFraction tmp { res.quotient };

    auto const needed_power = Crypto::NumberTheory::Power("10"_bigint, Crypto::UnsignedBigInteger { rounding_threshold });
    // We get one more digit to do proper rounding
    auto const frac_value = res.remainder.multiplied_by(needed_power.multiplied_by("10"_bigint)).divided_by(m_denominator).quotient;

    tmp.m_numerator.set_to(tmp.m_numerator.multiplied_by(needed_power));
    tmp.m_numerator.set_to(tmp.m_numerator.plus(frac_value.divided_by("10"_bigint).quotient));
    if (get_last_digit(frac_value) > "4"_bigint)
        tmp.m_numerator.set_to(tmp.m_numerator.plus("1"_bigint));

    tmp.m_denominator.set_to(tmp.m_denominator.multiplied_by(needed_power));

    return tmp;
}

void BigFraction::reduce()
{
    auto const GCD = Crypto::NumberTheory::GCD(m_numerator.unsigned_value(), m_denominator);

    auto const numerator_divide = m_numerator.divided_by(GCD);
    VERIFY(numerator_divide.remainder == "0"_bigint);
    m_numerator = numerator_divide.quotient;

    auto const denominator_divide = m_denominator.divided_by(GCD);
    VERIFY(denominator_divide.remainder == "0"_bigint);
    m_denominator = denominator_divide.quotient;
}

String BigFraction::to_string(unsigned rounding_threshold) const
{
    StringBuilder builder;
    if (m_numerator.is_negative() && m_numerator != "0"_bigint)
        builder.append("-");

    auto const number_of_digits = [](auto& integer) {
        unsigned size = 1;
        for (auto division_result = integer.divided_by(Crypto::UnsignedBigInteger { 10 });
             division_result.remainder == Crypto::UnsignedBigInteger { 0 } && division_result.quotient != Crypto::UnsignedBigInteger { 0 };
             division_result = division_result.quotient.divided_by(Crypto::UnsignedBigInteger { 10 })) {
            ++size;
        }
        return size;
    };

    auto const rounded_fraction = rounded(rounding_threshold);

    // We take the unsigned value as we already manage the '-'
    auto const full_value = rounded_fraction.m_numerator.unsigned_value().to_base(10);
    int split = full_value.length() - (number_of_digits(rounded_fraction.m_denominator) - 1);

    if (split < 0)
        split = 0;

    auto const remove_tailing_zeros = [](StringView value) -> StringView {
        auto n = value.length();
        while (value.characters_without_null_termination()[n - 1] == '0')
            --n;
        return { value.characters_without_null_termination(), n };
    };

    auto const raw_frac_value = full_value.substring(split, full_value.length() - split);

    String const integer_value = split == 0 ? "0" : full_value.substring(0, split);
    String const frac_value = rounding_threshold == 0 ? "0" : remove_tailing_zeros(raw_frac_value);

    builder.append(integer_value);

    bool const has_decimal_part = frac_value.length() > 0 && frac_value != "0";

    if (has_decimal_part) {
        builder.append('.');

        auto number_pre_zeros = number_of_digits(rounded_fraction.m_denominator) - full_value.length() - 1;
        if (number_pre_zeros > rounding_threshold || frac_value == "0")
            number_pre_zeros = 0;

        builder.append_repeated('0', number_pre_zeros);

        if (frac_value != "0")
            builder.append(frac_value);
    }

    return builder.to_string();
}

BigFraction BigFraction::inexact_sqrt() const
{
    return BigFraction { AK::sqrt(to_double()) };
}

}
