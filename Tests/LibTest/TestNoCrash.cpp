/*
 * Copyright (c) 2021, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibTest/TestCase.h>
#include <LibTest/TestSuite.h>

TEST_CASE(raise)
{
    EXPECT_NO_CRASH("This should never crash", [] {
        return Test::Crash::Failure::DidNotCrash;
    });
}

TEST_CASE(no_crash_fail)
{
    auto& test_suite = Test::TestSuite::the();

    test_suite.disable_reporting();

    EXPECT_NO_CRASH("This should never crash, but fail", [] {
        FAIL("expected failure");
        return Test::Crash::Failure::DidNotCrash;
    });

    // The FAIL in the crash test should make this test fail.
    if (test_suite.current_test_result() == Test::TestResult::Failed)
        test_suite.set_current_test_result(Test::TestResult::Passed);
    else
        test_suite.set_current_test_result(Test::TestResult::Failed);
}
