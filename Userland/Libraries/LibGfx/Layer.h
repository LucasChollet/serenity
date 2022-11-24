/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/HashTable.h>
#include <AK/RefCounted.h>
#include <AK/RefPtr.h>
#include <LibGfx/Color.h>
#include <LibGfx/Forward.h>
#include <LibGfx/Point.h>

namespace Gfx {

class Layer : public RefCounted<Layer> {
public:
    Layer(NonnullRefPtr<Bitmap> target, Color color, IntRect const& outer_rect);
    ~Layer();

    void add_point(IntPoint const& position, Color color);

private:
    void draw();

    NonnullRefPtr<Bitmap> m_target;

    RefPtr<Bitmap> m_points;
    IntPoint m_bitmap_top_left;
    Color m_color {};
};

}
