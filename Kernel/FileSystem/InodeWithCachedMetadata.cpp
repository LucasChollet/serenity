/*
 * Copyright (c) 2026, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "InodeWithCachedMetadata.h"

#include <AK/Singleton.h>
#include <Kernel/FileSystem/FileSystem.h>
#include <Kernel/FileSystem/InodeWithCachedMetadata.h>

namespace Kernel {

static Singleton<SpinlockProtected<InodeWithCachedMetadata::AllInstancesList, LockRank::None>> s_all_instances;

SpinlockProtected<InodeWithCachedMetadata::AllInstancesList, LockRank::None>& InodeWithCachedMetadata::all_instances()
{
    return s_all_instances;
}

void InodeWithCachedMetadata::sync_all()
{
    Vector<NonnullRefPtr<InodeWithCachedMetadata>, 32> inodes;
    all_instances().with([&](auto& all_inodes) {
        for (auto& inode : all_inodes) {
            if (inode.is_metadata_dirty()) {
                inodes.try_append(inode).release_value_but_fixme_should_propagate_errors();
            }
        }
    });

    for (auto& inode : inodes) {
        (void)inode->flush_metadata();
    }
}

void InodeWithCachedMetadata::sync()
{
    (void)flush_metadata();
    auto result = fs().flush_writes();
    if (result.is_error()) {
        // TODO: Figure out how to propagate error to a higher function.
    }
}

InodeWithCachedMetadata::InodeWithCachedMetadata(FileSystem& fs, InodeIndex index)
    : Inode(fs, index)
{
    all_instances().with([&](auto& all_inodes) { all_inodes.append(*this); });
}

bool InodeWithCachedMetadata::unref() const
{
    bool did_hit_zero = all_instances().with([this](auto&) {
        if (deref_base())
            return false;
        m_inode_list_node.remove();
        const_cast<InodeWithCachedMetadata&>(*this).revoke_weak_ptrs();
        return true;
    });
    if (did_hit_zero) {
        const_cast<InodeWithCachedMetadata&>(*this).will_be_destroyed();
        delete this;
    }
    return did_hit_zero;
}

void InodeWithCachedMetadata::will_be_destroyed()
{
    MutexLocker locker(m_inode_lock);
    if (m_metadata_dirty)
        (void)flush_metadata();
}

void InodeWithCachedMetadata::set_metadata_dirty(bool metadata_dirty)
{
    MutexLocker locker(m_inode_lock);

    if (metadata_dirty) {
        // Sanity check.
        VERIFY(!fs().is_readonly());
    }

    m_metadata_dirty = metadata_dirty;
}

void InodeWithCachedMetadata::did_modify_metadata()
{
    set_metadata_dirty(true);
    Inode::did_modify_metadata();
}

}
