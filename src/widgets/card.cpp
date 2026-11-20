// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/card.h"

#include "ui/color_provider.h"
#include "ui/painter.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;

namespace settings {

Card::Card(Style style)
{
    if (style == Style::Padded) {
        setPadding({metrics::cardHorizontalPadding, metrics::cardVerticalPadding, metrics::cardHorizontalPadding,
                    metrics::cardVerticalPadding});
        setSpacing(metrics::cardSpacing);
    } else {
        setSpacing(0);
    }
    setLayoutHints({.fillWidth = true});
}

void Card::paint(Painter& painter)
{
    painter.drawRoundedRect(localRect(), {metrics::cardRadius, 0, themeCorners()}, color(SettingsColor::CardBackground));
}


} // namespace settings
