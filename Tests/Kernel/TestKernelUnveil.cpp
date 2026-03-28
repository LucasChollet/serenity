/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibCore/File.h>
#include <LibCore/System.h>
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
        TRY_OR_FAIL(Core::System::unveil("/etc", "r"));

        // unveil permitted after unveil read only
        EXPECT(Core::System::unveil("/etc", "w").is_error());
        EXPECT(Core::System::unveil("/etc", "x").is_error());
        EXPECT(Core::System::unveil("/etc", "c").is_error());

        TRY_OR_FAIL(Core::System::unveil("/tmp/doesnotexist", "c"));

        TRY_OR_FAIL(Core::System::unveil("/home", "b"));

        // unveil permitted after unveil browse only
        EXPECT(Core::System::unveil("/home", "w").is_error());
        EXPECT(Core::System::unveil("/home", "x").is_error());
        EXPECT(Core::System::unveil("/home", "c").is_error());

        TRY_OR_FAIL(Core::System::unveil(nullptr, nullptr));

        // unveil permitted after unveil state locked
        EXPECT(Core::System::unveil("/bin", "w").is_error());

        // access permitted after locked veil without relevant unveil
        EXPECT(Core::System::access("/bin/id"sv, F_OK).is_error());
    };

    run_in_other_process(move(test));
}

TEST_CASE(symlinks)
{
    auto test = []() {
        rmdir("/tmp/foo/1");
        rmdir("/tmp/foo");
        unlink("/tmp/bar");

        TRY_OR_FAIL(Core::System::mkdir("/tmp/foo"sv, 0755));
        TRY_OR_FAIL(Core::System::mkdir("/tmp/foo/1"sv, 0755));

        TRY_OR_FAIL(Core::System::symlink("/tmp/foo"sv, "/tmp/bar"sv));
        TRY_OR_FAIL(Core::System::unveil("/tmp", "x"));
        TRY_OR_FAIL(Core::System::unveil("/tmp/foo", "r"));
        TRY_OR_FAIL(Core::System::unveil(nullptr, nullptr));

        TRY_OR_FAIL(Core::System::access("/tmp/foo/1"sv, R_OK));
        EXPECT(Core::System::access("/tmp/bar/1"sv, R_OK).is_error());

        TRY_OR_FAIL(Core::System::chdir("/tmp"sv));

        TRY_OR_FAIL(Core::System::access("./foo/1"sv, R_OK));
        EXPECT(Core::System::access("./bar/1"sv, R_OK).is_error());
    };

    run_in_other_process(move(test));
}

// TEST_CASE(symlink_file)
//{
//     auto setup = [] () {
//         unlink("/tmp/a");
//         unlink("/tmp/b");
//
//         {
//             auto file = TRY_OR_FAIL(Core::File::open("/tmp/a"sv, Core::File::OpenMode::ReadWrite));
//         }
//         TRY_OR_FAIL(Core::System::symlink("/tmp/a"sv, "/tmp/b"sv));
//     };
//
//     run_in_other_process([&]() {
//         setup();
//
//         TRY_OR_FAIL(Core::System::unveil("/tmp/a", "r"));
//         TRY_OR_FAIL(Core::System::unveil("/tmp/b", "r"));
//         TRY_OR_FAIL(Core::System::unveil(nullptr, nullptr));
//
//         TRY_OR_FAIL(Core::System::access("/tmp/a"sv, R_OK));
//         TRY_OR_FAIL(Core::System::access("/tmp/b"sv, R_OK));
//     });
// }
