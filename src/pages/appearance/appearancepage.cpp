// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

#include "base/i18n.h"
#include "cutefish/appearance/appearance.h"
#include "ui/header_button_group.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/painter.h"
#include "ui/search_field.h"
#include "ui/theme.h"
#include "ui/toolbar_surface.h"
#include "ui/slider.h"
#include "widgets/card.h"
#include "widgets/colors.h"
#include "widgets/desktopwallpaper.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowicon.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::appearance::Appearance;
using cutefish::i18n::tr;
using cutefish::i18n::trNoop;

namespace settings {

namespace {

constexpr std::array<int, accent::palette.size()> kSwatchOrder{0, 3, 4, 1, 5, 7, 2, 6, 8};
constexpr std::array<const char*, accent::palette.size()> kAccentNames{
    trNoop("Blue"), trNoop("Red"), trNoop("Green"), trNoop("Purple"), trNoop("Pink"),
    trNoop("Orange"), trNoop("Graphite"), trNoop("Yellow"), trNoop("Indigo")
};

// Children at their preferred sizes, left to right, with as much of the
// spacing as the width leaves, down to the minimum.
class FittingRow : public View
{
public:
    FittingRow(double spacing, double minimumSpacing)
        : m_spacing(spacing)
        , m_minimumSpacing(minimumSpacing)
    {
    }

    SizeF sizeHint() const override { return {childrenWidth() + gaps() * m_spacing, childrenHeight()}; }
    double heightForWidth(double) const override { return childrenHeight(); }
    double minimumWidth() const { return childrenWidth() + gaps() * m_minimumSpacing; }

protected:
    void layout() override
    {
        const double fitted = gaps() > 0 ? (geometry().width - childrenWidth()) / gaps() : 0;
        const double spacing = std::clamp(fitted, m_minimumSpacing, m_spacing);
        double x = 0;
        for (const auto& child : children()) {
            const SizeF size = child->sizeHint();
            child->setGeometry({std::round(x), 0, size.width, size.height});
            x += size.width + spacing;
        }
    }

private:
    double childrenWidth() const
    {
        double width = 0;
        for (const auto& child : children())
            width += child->sizeHint().width;
        return width;
    }

    double childrenHeight() const
    {
        double height = 0;
        for (const auto& child : children())
            height = std::max(height, child->sizeHint().height);
        return height;
    }

    double gaps() const { return children().empty() ? 0 : double(children().size() - 1); }

    double m_spacing;
    double m_minimumSpacing;
};

// One accent colour: a dot, ringed in its own colour while chosen or pressed.
// The dot is a parallel curve of the ring, so both snap to the same circle
// and the gap is even all round.
class AccentSwatch : public View
{
public:
    explicit AccentSwatch(Color color)
        : m_color(color)
    {
        m_check = addChild<Icon>("check");
    }

    void setChecked(bool checked)
    {
        if (checked == m_checked)
            return;
        m_checked = checked;
        requestLayout();
        requestPaint();
    }

    // The ring sits smallSpacing inside the cell, the dot as much inside the ring.
    SizeF sizeHint() const override
    {
        const double side = 28;
        return {side, side};
    }
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const RectF dot = localRect().shrunk(Margins::all(4));
        const double size = dot.height * 0.6;
        m_check->setSize(size);
        m_check->setColor(theme.highlightedText);
        m_check->setVisible(m_checked);
        m_check->setGeometry({dot.x + (dot.width - size) / 2, dot.y + (dot.height - size) / 2, size, size});
    }

    void paint(Painter& painter) override
    {
        const RectF ring = localRect();
        const RoundedShape circle{ring.width / 2, 0, Corners::Circular};
        if (m_checked || m_pressed || hasVisibleFocus())
            painter.drawRoundedRect(ring, circle, Color{0, 0, 0, 0}, 2, m_color);
        painter.drawRoundedRect(ring.shrunk(Margins::all(3)),
                                {circle.radius, 3, Corners::Circular}, m_color);
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

    bool keyPress(const KeyEvent& event) override
    {
        if (event.key != key::Return && event.key != key::Space && event.key != key::KeypadEnter)
            return false;
        if (onClicked)
            onClicked();
        return true;
    }

private:
    Color m_color;
    Icon* m_check;
    bool m_checked = false;
    bool m_pressed = false;
};

class ModePreview : public View
{
public:
    enum class Mode { Light, Dark, Auto };

    explicit ModePreview(Mode mode) : m_mode(mode)
    {
        m_wallpaper = addChild<DesktopWallpaper>();
        m_wallpaper->setPreviewRadius(5);
        m_desktop = addChild<DesktopOverlay>(*this);
    }

    SizeF sizeHint() const override { return {66, 43}; }

protected:
    void layout() override
    {
        m_wallpaper->setGeometry(localRect());
        m_desktop->setGeometry(localRect());
    }

    void paintOverlay(Painter& painter)
    {
        const auto draw = [&](bool dark) {
            const RectF toolbar{4, 5, 38, 11};
            toolbar_surface::capsule(painter, toolbar, dark);
            toolbar_surface::highlight(painter, toolbar.shrunk(Margins::all(2)), dark, false, true);
            const RectF window{19, 22, 46, 20};
            painter.drawSmoothRectangle(window, 5, Color::rgb(dark ? 0x25272c : 0xf8f8f9));
            for (const auto [n, color] : std::array{std::pair{0, 0xf7756a}, std::pair{1, 0xfbc83a},
                                                    std::pair{2, 0x79ce4b}})
                painter.drawSmoothRectangle({26.0 + n * 8, 30, 5, 5}, 2.5, Color::rgb(color));
        };
        if (m_mode != Mode::Auto) {
            draw(m_mode == Mode::Dark);
            return;
        }
        for (bool dark : {false, true}) {
            painter.save();
            painter.clip({dark ? 33.0 : 0.0, 0, 33, 43});
            draw(dark);
            painter.restore();
        }
    }

private:
    class DesktopOverlay : public View
    {
    public:
        explicit DesktopOverlay(ModePreview& owner) : m_owner(owner) {}
    protected:
        void paint(Painter& painter) override { m_owner.paintOverlay(painter); }
    private:
        ModePreview& m_owner;
    };

    Mode m_mode;
    DesktopWallpaper* m_wallpaper;
    View* m_desktop;
};

class GlassCaption : public Label
{
public:
    using Label::Label;

protected:
    void paint(Painter& painter) override
    {
        const RectF box = localRect();
        painter.fillRoundedRect(box.adjusted(-7, 2, 7, -2), 7, Color::rgb(0x000000).withAlpha(0.54));
        Label::paint(painter);
    }
};

class GlassSearch : public SearchField
{
public:
    using SearchField::SearchField;
    bool acceptsPointer() const override { return false; }
    bool acceptsFocus() const override { return false; }
};

// The preview uses actual toolbar controls; the material slider is read-only.
class GlassSetting : public View
{
public:
    GlassSetting()
    {
        m_title = addChild<Label>(tr("Liquid Glass"));
        m_title->setWrap(true);
        m_picture = addChild<DesktopWallpaper>();
        m_picture->setName("appearance:wallpaper");
        m_picture->setPreviewRadius(16);
        m_actions = addChild<HeaderButtonGroup>();
        m_actions->setName("appearance:previewActions");
        m_actions->setToolbarAppearance(true);
        m_actions->addButton("share", 19);
        m_actions->addButton("ellipsis", 19);
        m_stack = addChild<HeaderButton>(std::string(), 36);
        m_stack->setToolbarAppearance(true);
        m_stack->setGlyph("window-overlap", 22);
        m_search = addChild<GlassSearch>(tr("Search"));
        m_search->setName("appearance:previewSearch");
        m_search->setReadOnly(true);
        m_search->setIconMetrics(9, 4, 5);
        m_search->setGlyphColor(ColorId::Text);
        m_search->setPlaceholderColor(ColorId::Text);
        m_caption = addChild<GlassCaption>(tr("Desktop Wallpaper"));
        m_caption->setWeight(650);
        m_caption->setFontSize(15);
        m_slider = addChild<Slider>();
        m_slider->setName("appearance:glass");
        m_slider->setRange(0, 1);
        m_slider->setValue(0.5);
        m_slider->setEnabled(false);
        setLayoutHints({.fillWidth = true});
    }

    double heightForWidth(double width) const override { return std::ceil(previewWidth(width) * 0.67) + 26; }
    SizeF sizeHint() const override { return {380, heightForWidth(448)}; }

protected:
    void layout() override
    {
        const double width = previewWidth(geometry().width);
        const double x = geometry().width - width;
        const double height = std::ceil(width * 0.67);
        m_title->setGeometry({0, 0, std::max(0.0, x - 10), 38});
        m_picture->setGeometry({x, 0, width, height});
        const double bar = std::min(36.0, width / 6);
        const double gap = std::min(8.0, width / 27);
        m_actions->setGeometry({x + gap, gap, bar * 2, bar});
        m_stack->setGeometry({x + 2 * gap + 2 * bar - 1, gap - 1, bar + 2, bar + 2});
        const double searchStart = 3 * gap + 3 * bar;
        m_search->setGeometry({x + searchStart, gap, std::max(0.0, width - searchStart - gap), bar});
        m_caption->setColor(Color::rgb(0xffffff));
        m_caption->setGeometry({x + 16, height * 0.80, std::min(width - 28, m_caption->sizeHint().width), height * 0.19});
        m_slider->setGeometry({x + 17, height + 2, width - 34, 24});
    }

    void paint(Painter& painter) override
    {
        painter.drawHorizontalLine(0, -10, geometry().width, color(SettingsColor::CardSeparator));
        const auto bounds = m_slider->geometry();
        const double y = bounds.y + bounds.height / 2;
        const Color ink = Theme::current().disabledText;
        for (double x : {bounds.x - 14, bounds.right() + 2}) {
            painter.drawSmoothRectangle({x, y - 5, 10, 8}, 2, Color{0, 0, 0, 0}, 0.8, ink);
            painter.drawSmoothRectangle({x + 3, y - 2, 10, 8}, 2,
                x < bounds.x ? Color{0, 0, 0, 0} : ink, 0.8, ink);
        }
    }

private:
    static double previewWidth(double width) { return std::min(268.0, width * 0.64); }

    Label* m_title;
    DesktopWallpaper* m_picture;
    HeaderButtonGroup* m_actions;
    HeaderButton* m_stack;
    SearchField* m_search;
    Label* m_caption;
    Slider* m_slider;
};

class AppearanceChoices : public ListRow
{
public:
    explicit AppearanceChoices(std::string text) : ListRow(std::move(text))
    {
        setInteractive(false);
        label()->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Start,
                                 .margins = {0, 12, 0, 0}});
    }
};

class StylePreview : public View
{
public:
    explicit StylePreview(size_t style) : m_style(style) {}
    SizeF sizeHint() const override { return {29, 29}; }

protected:
    void paint(Painter& painter) override
    {
        static constexpr std::array<uint32_t, 4> tile{0x2e9bea, 0x212126, 0x939495, 0x22a3d0};
        static constexpr std::array<uint32_t, 4> sun{0xffd459, 0xe5bd67, 0xdedee0, 0x85dcef};
        static constexpr std::array<uint32_t, 4> cloud{0xedf8ff, 0xada9b1, 0xf6f6f6, 0xd1f9ff};
        const Color base = Color::rgb(tile[m_style]);
        painter.drawSmoothRectangle(localRect(), 7, base, 0.8, Color::rgb(0xffffff).withAlpha(0.34));
        painter.drawSmoothRectangle({14, 5, 9, 9}, 4.5, Color::rgb(sun[m_style]));
        const Color white = Color::rgb(cloud[m_style]);
        painter.drawSmoothRectangle({4, 13, 21, 9}, 4.5, white);
        painter.drawSmoothRectangle({8, 10, 10, 10}, 5, white);
    }

private:
    size_t m_style;
};

class AppearancePage : public Page
{
public:
    explicit AppearancePage(SettingsWindow& window)
        : Page(window, tr("Appearance"))
    {
        Card* modeCard = content()->addChild<Card>(Card::Style::List);
        AppearanceChoices* modes = modeCard->addChild<AppearanceChoices>(tr("Appearance"));
        FittingRow* tiles = modes->addContent<FittingRow>(7, 4);
        const SizeF preview{74, 51};
        m_light = tiles->addChild<ImageSelectionItem>(std::make_unique<ModePreview>(ModePreview::Mode::Light), tr("Light"), preview);
        m_light->onClicked = [this] { chooseDarkMode(false); };
        m_dark = tiles->addChild<ImageSelectionItem>(std::make_unique<ModePreview>(ModePreview::Mode::Dark), tr("Dark"), preview);
        m_dark->onClicked = [this] { chooseDarkMode(true); };
        m_auto = tiles->addChild<ImageSelectionItem>(std::make_unique<ModePreview>(ModePreview::Mode::Auto), tr("Auto"), preview);
        m_auto->onClicked = [this] { m_appearance.setDarkModeSchedule(Appearance::SunsetToSunrise); };
        modes->setMinimumRowHeight(86);
        Card* glass = modeCard->addChild<Card>();
        glass->addChild<GlassSetting>();

        Section* accents = content()->addChild<Section>(tr("Theme"));
        accents->setLayoutHints({.fillWidth = true, .margins = {0, 26, 0, 0}});
        Card* accentCard = accents->addChild<Card>(Card::Style::List);
        AppearanceChoices* color = accentCard->addChild<AppearanceChoices>(tr("Color"));
        color->setMinimumRowHeight(57);
        FittingRow* swatches = color->addContent<FittingRow>(8, 3);
        for (const int index : kSwatchOrder) {
            AccentSwatch* swatch = swatches->addChild<AccentSwatch>(accent::fromIndex(index));
            swatch->setToolTip(tr(kAccentNames[index]));
            swatch->onClicked = [this, index] { m_appearance.setAccentColor(index); };
            m_swatches.push_back(swatch);
        }
        swatches->setLayoutHints({.minimumWidth = swatches->minimumWidth()});
        tiles->setLayoutHints({.minimumWidth = tiles->minimumWidth()});

        accentCard->addChild<InfoRow>(tr("Text highlight color"), tr("Same as theme"));

        Card* icons = content()->addChild<Card>(Card::Style::List);
        AppearanceChoices* iconStyle = icons->addChild<AppearanceChoices>(tr("Icon & widget style"));
        FittingRow* styles = iconStyle->addContent<FittingRow>(9, 3);
        styles->setEnabled(false);
        const std::array<const char*, 4> labels{tr("Default"), tr("Dark"), tr("Clear"), tr("Tinted")};
        for (size_t i = 0; i < labels.size(); ++i) {
            auto* choice = styles->addChild<ImageSelectionItem>(std::make_unique<StylePreview>(i), labels[i],
                                                                 SizeF{39, 38}, 1);
            choice->setName("appearance:iconStyle:" + std::to_string(i));
            choice->setChecked(i == 0);
        }
        iconStyle->setMinimumRowHeight(84);
        ListRow* folderColor = icons->addChild<ListRow>(tr("Folder color"));
        folderColor->setInteractive(false);
        auto* folderPicker = folderColor->addContent<RowComboBox>(std::vector<std::string>{tr("Automatic")});
        folderPicker->setName("appearance:folderColor");
        folderPicker->setEnabled(false);

        Card* transparency = content()->addChild<Card>(Card::Style::List);
        m_wallpaperTinting = transparency->addChild<SwitchRow>(tr("Allow wallpaper tinting in windows"));
        m_wallpaperTinting->setName("appearance:wallpaperTint");
        m_wallpaperTinting->setEnabled(false);
        m_reduceTransparency = transparency->addChild<SwitchRow>(tr("Reduce Transparency"));
        m_reduceTransparency->setName("appearance:reduceTransparency");
        m_reduceTransparency->setEnabled(false);
        m_coloredWindowControls = transparency->addChild<SwitchRow>(tr("Colored Window Buttons"));
        m_coloredWindowControls->onToggled = [this](bool checked) { m_appearance.setColoredWindowControls(checked); };

        Card* links = content()->addChild<Card>(Card::Style::List);
        const std::array<std::tuple<const char*, const char*, Color, const char*>, 3> pages = {
                std::tuple{tr("Fonts"), "fonts.svg", Color::rgb(0xFE8433), "fonts"},
                {tr("Icon Theme"), "icontheme.svg", Color::rgb(0x2EC347), "icontheme"},
                {tr("Cursor Style"), "cursor.svg", Color::rgb(0x3385FF), "cursortheme"}};
        for (const auto& [title, icon, tint, id] : pages) {
            NavigationItem* item = links->addChild<NavigationItem>(title);
            item->setLeading<RowIcon>(icon, tint);
            const std::string page = id;
            item->onClicked = [&window, page] { window.pushPage(page); };
        }

        m_subscription = m_appearance.changed.connect([this] { update(); });
        update();
    }

protected:
    void layout() override
    {
        // The theme follows the service too; its colours decide the checks.
        update();
        Page::layout();
    }

private:
    bool autoDarkMode() const { return m_appearance.darkModeSchedule() == Appearance::SunsetToSunrise; }

    void chooseDarkMode(bool dark)
    {
        m_appearance.setDarkModeSchedule(Appearance::Never);
        m_appearance.setDarkMode(dark);
    }

    void update()
    {
        const Theme& theme = Theme::current();
        const bool automatic = autoDarkMode();
        m_light->setChecked(!automatic && !theme.darkMode);
        m_dark->setChecked(!automatic && theme.darkMode);
        m_auto->setChecked(automatic);
        for (size_t i = 0; i < m_swatches.size(); ++i)
            m_swatches[i]->setChecked(theme.highlight == accent::fromIndex(kSwatchOrder[i]));
        m_wallpaperTinting->setChecked(m_appearance.wallpaperTinting());
        m_reduceTransparency->setChecked(m_appearance.reduceTransparency());
        m_coloredWindowControls->setChecked(m_appearance.coloredWindowControls());
    }

    Appearance m_appearance;
    cutefish::base::Subscription m_subscription;
    ImageSelectionItem* m_light;
    ImageSelectionItem* m_dark;
    ImageSelectionItem* m_auto;
    std::vector<AccentSwatch*> m_swatches;
    SwitchRow* m_wallpaperTinting;
    SwitchRow* m_reduceTransparency;
    SwitchRow* m_coloredWindowControls;
};

} // namespace

static const PageRegistration registration("appearance", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AppearancePage>(window);
});

} // namespace settings
