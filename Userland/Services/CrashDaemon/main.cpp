/*
 * Copyright (c) 2020, Itamar S. <itamar8910@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/LexicalPath.h>
#include <AK/Time.h>
#include <LibCore/ChildWatcher.h>
#include <LibCore/EventLoop.h>
#include <LibCore/FileWatcher.h>
#include <LibCore/MappedFile.h>
#include <LibCore/Process.h>
#include <LibCore/System.h>
#include <LibMain/Main.h>
#include <sys/stat.h>
#include <unistd.h>

static void wait_until_coredump_is_ready(ByteString const& coredump_path)
{
    while (true) {
        struct stat statbuf;
        if (stat(coredump_path.characters(), &statbuf) < 0) {
            perror("stat");
            VERIFY_NOT_REACHED();
        }
        if (statbuf.st_mode & 0400) // Check if readable
            break;

        usleep(10000); // sleep for 10ms
    }
}

static Optional<pid_t> launch_crash_reporter(ByteString const& coredump_path)
{
    auto process = Core::Process::spawn({ .executable = "/bin/CrashReporter"sv,
        .arguments = Vector<ByteString> { "--unlink", coredump_path.characters() },
        .keep_as_child = Core::KeepAsChild::Yes });

    if (process.is_error()) {
        dbgln("Failed to launch CrashReporter");
        return OptionalNone {};
    }

    auto pid = process.value().get_events_on_current_event_loop();

    if (pid.is_error()) {
        dbgln("Error while trying to get events: {}", pid.error());
        return OptionalNone {};
    }

    return pid.value();
}

enum class ChildStatus : u8 {
    Started,
    Crashed,
};

struct ChildData {
    ChildStatus status;
    Optional<MonotonicTime> time {};
};

static bool should_prevent_launch(HashMap<pid_t, ChildData>& statuses)
{
    // FIXME: GCC is tripping on this one.
    AK_IGNORE_DIAGNOSTIC("-Wunused-but-set-variable", static u32 time_limit = 3)

    auto now = MonotonicTime::now();

    statuses.remove_all_matching(
        [=](auto, auto const& value) {
            return value.time.has_value() && (now - *value.time).to_seconds() > time_limit;
        });

    return any_of(statuses, [=](auto const& entry) {
        return entry.value.status == ChildStatus::Crashed && (now - *entry.value.time).to_seconds() <= time_limit;
    });
}

ErrorOr<int> serenity_main(Main::Arguments)
{
    TRY(Core::System::pledge("stdio rpath wpath cpath proc exec thread"));

    Core::EventLoop event_loop {};

    HashMap<pid_t, ChildData> statuses;

    auto child_watcher = Core::ChildWatcher::create();
    child_watcher->on_change = [&](auto const& event) {
        if (!event.did_exited_abnormally()) {
            statuses.remove(event.child_pid);
            return;
        }
        dbgln("CrashReporter pid({}) exited abnormally"sv, event.child_pid);
        statuses.set(event.child_pid, { ChildStatus::Crashed, MonotonicTime::now() });
    };

    auto file_watcher = TRY(Core::FileWatcher::create());
    TRY(file_watcher->add_watch("/tmp/coredump", Core::FileWatcherEvent::Type::ChildCreated));

    file_watcher->on_change = [&](auto const& event) {
        if (event.type != Core::FileWatcherEvent::Type::ChildCreated)
            return;
        auto& coredump_path = event.event_path;
        dbgln("New coredump file: {}", coredump_path);
        wait_until_coredump_is_ready(coredump_path);

        auto file_or_error = Core::MappedFile::map(coredump_path);
        if (file_or_error.is_error()) {
            dbgln("Unable to map coredump {}: {}", coredump_path, file_or_error.error());
            return;
        }

        // For some reason, for a same CrashReporter crash, the FileEvent is
        // always scheduled before the ChildEvent. So let's defer the invocation
        // of this code to ensure we the proper order of execution.
        event_loop.deferred_invoke([=, &statuses]() {
            if (should_prevent_launch(statuses)) {
                dbgln("Preventing CrashReporter launch due to past failure");
                return;
            }

            auto pid = launch_crash_reporter(coredump_path);
            if (pid.has_value())
                statuses.set(*pid, { .status = ChildStatus::Started });
        });
    };

    return event_loop.exec();
}
