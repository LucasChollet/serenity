/*
 * Copyright (c) 2026, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/FileSystem/Inode.h>

namespace Kernel {

class InodeWithCachedMetadata : public Inode {
public:
    virtual bool unref() const override;

    static void sync_all();
    virtual void sync() override;

    virtual void will_be_destroyed() override;

    virtual ErrorOr<void> flush_metadata() = 0;

    void set_metadata_dirty(bool);
    bool is_metadata_dirty() const { return m_metadata_dirty; }

protected:
    InodeWithCachedMetadata(FileSystem&, InodeIndex);

private:
    bool m_metadata_dirty { false };

    mutable IntrusiveListNode<InodeWithCachedMetadata> m_inode_list_node;

public:
    using AllInstancesList = IntrusiveList<&InodeWithCachedMetadata::m_inode_list_node>;
    static SpinlockProtected<InodeWithCachedMetadata::AllInstancesList, LockRank::None>& all_instances();
};

}
