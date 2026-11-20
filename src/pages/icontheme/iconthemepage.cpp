// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "base/i18n.h"
#include "cutefish/appearance/appearance.h"
#include "pages/icontheme/iconthemes.h"
#include "ui/image_view.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "widgets/tilegrid.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;
using cutefish::appearance::Appearance;
using cutefish::i18n::tr;

namespace settings {

namespace {

constexpr double kCell = 22;
constexpr double kGap = 6;

// Four of the theme's icons in a square: folder, text, image and trash.
class IconThemePreview : public View
{
public:
    explicit IconThemePreview(const std::vector<std::string>& files)
    {
        for (const std::string& file : files) {
            ImageView* icon = addChild<ImageView>(file);
            icon->setVisible(!file.empty());
            m_icons.push_back(icon);
        }
    }

    SizeF sizeHint() const override { return {kCell * 2 + kGap, kCell * 2 + kGap}; }

protected:
    void layout() override
    {
        for (size_t i = 0; i < m_icons.size(); ++i)
            m_icons[i]->setGeometry({double(i % 2) * (kCell + kGap), double(i / 2) * (kCell + kGap), kCell, kCell});
    }

private:
    std::vector<ImageView*> m_icons;
};

class IconThemePage : public Page
{
public:
    explicit IconThemePage(SettingsWindow& window)
        : Page(window, tr("Icon Theme"))
        , m_themes(IconTheme::installed())
    {
        Section* section = content()->addChild<Section>(tr("Icon theme"));
        Card* card = section->addChild<Card>();
        TileGrid* grid = card->addChild<TileGrid>(SizeF{104 + metrics::tileSpacing, 104}, metrics::m);
        grid->setVisible(!m_themes.empty());
        for (const IconTheme& theme : m_themes) {
            auto* item = grid->addChild<ImageSelectionItem>(std::make_unique<IconThemePreview>(theme.previewFiles),
                                                            theme.name, SizeF{104, 64}, 0);
            item->setName("icontheme:theme");
            item->setEnabled(false);
            m_items.push_back(item);
        }
        m_subscription = m_appearance.changed.connect([this] { updateChecks(); });
        updateChecks();
    }

private:
    std::string currentTheme() const
    {
        const std::string theme = m_appearance.iconTheme();
        return theme.empty() ? "Cutefish" : theme;
    }

    void updateChecks()
    {
        const std::string current = currentTheme();
        for (size_t i = 0; i < m_items.size(); ++i)
            m_items[i]->setChecked(m_themes[i].id == current);
    }

    Appearance m_appearance;
    cutefish::base::Subscription m_subscription;
    std::vector<IconTheme> m_themes;
    std::vector<ImageSelectionItem*> m_items;
};

} // namespace

static const PageRegistration registration("icontheme", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<IconThemePage>(window);
});

} // namespace settings
