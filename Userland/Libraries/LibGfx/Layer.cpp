/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "Layer.h"
#include <AK/Function.h>
#include <LibGfx/Bitmap.h>

namespace Gfx {

Layer::Layer(NonnullRefPtr<Bitmap> target, Gfx::Color color, IntRect const& outer_rect)
    : m_target(move(target))
    , m_bitmap_top_left(outer_rect.top_left())
    , m_color(color)
{
    m_points = MUST(Bitmap::try_create(BitmapFormat::Indexed1, outer_rect.size()));
}

void Layer::add_point(Gfx::IntPoint const& position, Gfx::Color color)
{
    VERIFY(color == m_color);

    m_points->scanline_u8(position.y())[position.x()] = 1;
}

void Layer::draw()
{
    for (int i {}; i < m_points->height(); ++i) {
        for (int j {}; j < m_points->width(); ++j) {
            if (m_points->scanline_u8(i)[j] == 1) {
                auto& pixel = m_target->scanline(i + m_bitmap_top_left.y())[j + m_bitmap_top_left.x()];
                pixel = Color::from_argb(pixel).blend(m_color).value();
            }
        }
    }
}

Layer::~Layer()
{
    draw();
}

}
