/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/String.h>
#include <LibCrypto/BigInt/SignedBigInteger.h>

namespace Crypto {

class BigFraction {
public:
    BigFraction() = default;
    explicit BigFraction(Crypto::SignedBigInteger const&);
    BigFraction(Crypto::SignedBigInteger const& numerator, Crypto::UnsignedBigInteger const& denominator);

    explicit BigFraction(StringView);
    explicit BigFraction(double);

    BigFraction operator+(BigFraction const&) const;
    BigFraction operator-(BigFraction const&) const;
    BigFraction operator*(BigFraction const&) const;
    BigFraction operator/(BigFraction const&) const;

    BigFraction& operator+=(BigFraction const&);
    BigFraction& operator-=(BigFraction const&);
    BigFraction& operator*=(BigFraction const&);
    BigFraction& operator/=(BigFraction const&);

    BigFraction operator-(void) const;

    bool operator<(BigFraction const&) const;
    bool operator==(BigFraction const&) const;

    BigFraction invert() const;
    BigFraction inexact_sqrt() const;

    void set_to_0();

    // Return a KeypadValue value in "scientific notation", as an example with:
    //      - m_numerator = 2
    //      - m_denominator = 3
    //      - rounding_threshold = 4
    // The returned KeypadValue will have:
    //      - m_numerator = 6667
    //      - m_denominator = 10000
    BigFraction rounded(unsigned rounding_threshold) const;

    String to_string(unsigned rounding_threshold) const;
    double to_double() const;

private:
    void reduce();

    // This class uses a pair of integers to store a value. The purpose is to
    // support any rational number without any numerical errors.
    // For example, if we were to represent the value -123.55 in this format,
    // the values could be -12355 for and 100 for m_denominator. However, this
    // pair of value is not unique and the value will be reduced to -2471/20.
    // This way, most operations don't have to be performed on doubles, but can
    // be performed without loss of precision on this class.
    Crypto::SignedBigInteger m_numerator { 0 };
    Crypto::UnsignedBigInteger m_denominator { 1 };
};

}
