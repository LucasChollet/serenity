/*
 * Copyright (c) 2025, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "ChildWatcher.h"
#include <AK/HashTable.h>
#include <AK/NonnullRefPtr.h>
#include <LibCore/EventLoop.h>
#include <LibCore/Notifier.h>

namespace Core {

static HashTable<ChildWatcher*> s_watchers;

NonnullRefPtr<ChildWatcher> ChildWatcher::create()
{
    auto watcher = adopt_ref(*new ChildWatcher);
    s_watchers.set(watcher.ptr());
    return watcher;
}

ChildWatcher::~ChildWatcher()
{
    s_watchers.remove(this);
}

void ChildWatcher::register_event(ChildWatcherEvent const& event, NonnullRefPtr<Notifier> notifier)
{
    for (auto& watcher : s_watchers) {
        if (watcher->on_change)
            watcher->on_change(event);
    }

    // This is the last time the notifier fired, let's make sure it doesn't get leak.
    EventLoop::current().deferred_invoke([strong_ref = notifier]() {
        // Remove the reference cycle, while holding a reference ()
        strong_ref->on_activation = [] { };
    });
}

}
