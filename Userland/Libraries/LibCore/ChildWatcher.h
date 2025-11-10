/*
 * Copyright (c) 2025, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Badge.h>
#include <AK/Error.h>
#include <AK/Function.h>
#include <AK/NonnullRefPtr.h>
#include <AK/RefCounted.h>
#include <LibCore/Forward.h>

namespace Core {

struct ChildWatcherEvent {
    enum class EventType : u8 {
        Exited,
        ReceivedSignal,
    };
    pid_t child_pid {};
    EventType type { EventType::Exited };

    Optional<int> exit_value {};
    Optional<int> signal_value {};
};

class ChildWatcher : public RefCounted<ChildWatcher> {
    AK_MAKE_NONCOPYABLE(ChildWatcher);

public:
    ~ChildWatcher();

    static NonnullRefPtr<ChildWatcher> create();

    static void register_event(ChildWatcherEvent const&, NonnullRefPtr<Notifier>);

    Function<void(ChildWatcherEvent const&)> on_change;

private:
    ChildWatcher() = default;
};
};
