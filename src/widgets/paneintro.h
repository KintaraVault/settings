// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "widgets/card.h"
#include "widgets/listrow.h"

namespace settings {

class NavigationItem;
class SettingsWindow;

// Informational tiles at the top of a preferences pane. Overview panes use
// the centred version; single settings panes use the compact leading icon.
class PaneIntro : public Card
{
public:
    PaneIntro(std::string title, std::string description, std::string icon, cutefish::ui::Color tint,
              bool centered = false);
};

class PaneSwitch : public SwitchRow
{
public:
    PaneSwitch(std::string title, std::string description, std::string icon, cutefish::ui::Color tint);
};

NavigationItem* addPageLink(Card* card, SettingsWindow& window, const std::string& id, bool direct = false);

} // namespace settings
