/*
 * Copyright (c) 2025, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibCore/ChildWatcher.h>
#include <LibCore/EventLoop.h>
#include <LibCore/Process.h>
#include <LibCore/Timer.h>
#include <LibTest/TestCase.h>

TEST_CASE(child_watcher)
{
    Core::EventLoop event_loop;
    auto timeout = Core::Timer::create_single_shot(2000, [&]() { event_loop.quit(-1); });
    auto watcher = Core::ChildWatcher::create();
    Optional<pid_t> child_pid;
    watcher->on_change = [&](Core::ChildWatcherEvent event) {
        EXPECT(event.type == Core::ChildWatcherEvent::EventType::Exited);
        EXPECT_EQ(event.exit_value, 0);
        event_loop.quit(0);
    };

    {
        auto process = TRY_OR_FAIL(Core::Process::spawn({ .executable = "/bin/true",
            .keep_as_child = Core::KeepAsChild::Yes }));
        child_pid = TRY_OR_FAIL(process.get_events_on_current_event_loop());
    }

    EXPECT_EQ(event_loop.exec(), 0);
}
