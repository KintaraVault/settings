// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>

#include "ui/icon.h"

namespace settings {

// The sidebar's tile: a coloured rounded square with a white glyph, for the
// sidebar and a list row's leading slot. The glyph is one of settings'
// sidebar pictures, or a toolkit glyph when the name has no ".svg".
class RowIcon : public cutefish::ui::Icon
{
public:
    RowIcon(std::string glyph, cutefish::ui::Color tint);

    void set(std::string glyph, cutefish::ui::Color tint);

    cutefish::ui::SizeF sizeHint() const override { return {24, 24}; }

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;

private:
    cutefish::ui::Color m_tint;
};

} // namespace settings
