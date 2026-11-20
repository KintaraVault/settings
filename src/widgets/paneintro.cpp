// SPDX-License-Identifier: BSD-3-Clause
#include "widgets/paneintro.h"

#include "base/i18n.h"
#include "ui/label.h"
#include "ui/switch.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowicon.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

PaneIntro::PaneIntro(std::string title, std::string description, std::string icon, Color tint, bool centered)
{
    ColumnLayout* text = this;
    if (centered) {
        setPadding({10, 25, 10, 18});
        setSpacing(8);
        auto* badge = addChild<RowIcon>(std::move(icon), tint);
        badge->setLayoutHints({.preferredWidth = 48, .preferredHeight = 48, .horizontalAlignment = Alignment::Center});
    } else {
        auto* row = addChild<RowLayout>();
        row->setLayoutHints({.fillWidth = true});
        row->setSpacing(10);
        auto* badge = row->addChild<RowIcon>(std::move(icon), tint);
        badge->setLayoutHints({.preferredWidth = 28, .preferredHeight = 28, .verticalAlignment = Alignment::Start});
        text = row->addChild<ColumnLayout>();
        text->setLayoutHints({.fillWidth = true});
    }
    auto* heading = text->addChild<Label>(std::move(title));
    heading->setWeight(650);
    heading->setFontSize(centered ? 22 : 13);
    auto* note = text->addChild<Label>(std::move(description));
    note->setColor(ColorId::DisabledText);
    note->setFontSize(12);
    note->setWrap(true);
    if (centered)
        note->setLayoutHints({.fillWidth = true, .maximumWidth = 420, .horizontalAlignment = Alignment::Center});
    for (auto* label : {heading, note}) {
        if (centered)
            label->setTextAlignment(Label::TextAlignment::Center);
        if (!centered || label != note)
            label->setLayoutHints({.fillWidth = true});
    }
}

PaneSwitch::PaneSwitch(std::string title, std::string description, std::string icon, Color tint)
{
    auto* leading = setLeading<RowIcon>(std::move(icon), tint);
    leading->setLayoutHints({.preferredWidth = 27, .preferredHeight = 27});
    auto* text = addContent<ColumnLayout>();
    text->setLayoutHints({.fillWidth = true, .margins = {0, 8, 0, 8}});
    contentRow()->raiseChild(control());
    text->setSpacing(1);
    text->addChild<Label>(std::move(title))->setLayoutHints({.fillWidth = true});
    auto* note = text->addChild<Label>(std::move(description));
    note->setWrap(true);
    note->setFontSize(12);
    note->setColor(ColorId::DisabledText);
    note->setLayoutHints({.fillWidth = true});
    auto hints = control()->layoutHints();
    hints.verticalAlignment = Alignment::Start;
    hints.margins.top = 8;
    control()->setLayoutHints(hints);
    setMinimumRowHeight(66);
}

NavigationItem* addPageLink(Card* card, SettingsWindow& window, const std::string& id, bool direct)
{
    const auto* page = findPage(id);
    if (!page || !page->create)
        return nullptr;
    auto* row = card->addChild<NavigationItem>(tr(page->title.c_str()));
    row->setMinimumRowHeight(metrics::twoLineRowHeight);
    row->setLeading<RowIcon>(page->icon, page->iconColor);
    row->onClicked = [&window, id, direct] {
        if (direct)
            window.switchToPage(id);
        else
            window.pushPage(id);
    };
    return row;
}

} // namespace settings
