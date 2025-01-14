/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Vector.h>
#include <LibGfx/Matrix4x4.h>
#include <LibWeb/Painting/InlinePaintable.h>
#include <LibWeb/Painting/Paintable.h>

namespace Web::Painting {

class StackingContext {
public:
    StackingContext(Paintable&, StackingContext* parent, size_t index_in_tree_order);

    StackingContext* parent() { return m_parent; }
    StackingContext const* parent() const { return m_parent; }

    Paintable const& paintable() const { return *m_paintable; }
    PaintableBox const& paintable_box() const { return verify_cast<PaintableBox>(*m_paintable); }
    InlinePaintable const& inline_paintable() const { return verify_cast<InlinePaintable>(*m_paintable); }

    enum class StackingContextPaintPhase {
        BackgroundAndBorders,
        Floats,
        BackgroundAndBordersForInlineLevelAndReplaced,
        Foreground,
        FocusAndOverlay,
    };

    static void paint_node_as_stacking_context(Paintable const&, PaintContext&);
    static void paint_descendants(PaintContext&, Paintable const&, StackingContextPaintPhase);
    void paint(PaintContext&) const;
    Optional<HitTestResult> hit_test(CSSPixelPoint, HitTestType) const;

    Gfx::AffineTransform affine_transform_matrix() const;

    void dump(int indent = 0) const;

    void sort();

private:
    JS::NonnullGCPtr<Paintable> m_paintable;
    StackingContext* const m_parent { nullptr };
    Vector<StackingContext*> m_children;
    size_t m_index_in_tree_order { 0 };

    static void paint_child(PaintContext&, StackingContext const&);
    void paint_internal(PaintContext&) const;
};

}
