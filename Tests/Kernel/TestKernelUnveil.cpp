/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibTest/TestCase.h>
#include <errno.h>
#include <unistd.h>

TEST_CASE(test_argument_validation)
{
    auto res = unveil("/etc", "aaaaaaaaaaaa");
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, E2BIG);

    res = unveil("/etc", "aaaaa");
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, EINVAL);

    res = unveil(nullptr, "r");
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, EINVAL);

    res = unveil("/etc", nullptr);
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, EINVAL);

    res = unveil("", "r");
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, EINVAL);

    res = unveil("test", "r");
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, EINVAL);

    res = unveil("/etc", "f");
    EXPECT_EQ(res, -1);
    EXPECT_EQ(errno, EINVAL);
}

static void run_in_other_process(auto&& test_case)
{
    EXPECT_NO_CRASH("unveil", [test_case = move(test_case)] {
        test_case();
        return Test::Crash::Failure::DidNotCrash;
    });
}

TEST_CASE(test_failures)
{
    auto test = []() {
        auto res = unveil("/etc", "r");
        if (res < 0)
            FAIL("unveil read only failed");

        res = unveil("/etc", "w");
        if (res >= 0)
            FAIL("unveil write permitted after unveil read only");

        res = unveil("/etc", "x");
        if (res >= 0)
            FAIL("unveil execute permitted after unveil read only");

        res = unveil("/etc", "c");
        if (res >= 0)
            FAIL("unveil create permitted after unveil read only");

        res = unveil("/tmp/doesnotexist", "c");
        if (res < 0)
            FAIL("unveil create on non-existent path failed");

        res = unveil("/home", "b");
        if (res < 0)
            FAIL("unveil browse failed");

        res = unveil("/home", "w");
        if (res >= 0)
            FAIL("unveil write permitted after unveil browse only");

        res = unveil("/home", "x");
        if (res >= 0)
            FAIL("unveil execute permitted after unveil browse only");

        res = unveil("/home", "c");
        if (res >= 0)
            FAIL("unveil create permitted after unveil browse only");

        res = unveil(nullptr, nullptr);
        if (res < 0)
            FAIL("unveil state lock failed");

        res = unveil("/bin", "w");
        if (res >= 0)
            FAIL("unveil permitted after unveil state locked");

        res = access("/bin/id", F_OK);
        if (res == 0)
            FAIL("access(..., F_OK) permitted after locked veil without relevant unveil");
    };

    run_in_other_process(move(test));
}
