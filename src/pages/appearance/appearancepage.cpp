// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

#include "base/i18n.h"
#include "cutefish/appearance/appearance.h"
#include "ui/check_box.h"
#include "ui/header_button_group.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/search_field.h"
#include "ui/theme.h"
#include "ui/toolbar_surface.h"
#include "ui/slider.h"
#include "ui/view_glass.h"
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

constexpr std::array kSwatchOrder{-1, 0, 3, 4, 1, 5, 7, 2, 6, 8};
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
    explicit AccentSwatch(int index)
        : m_color(swatchColor(index)), m_multicolor(index < 0)
    {
        m_check = addChild<Icon>("check");
        if (m_multicolor) {
            m_caption = addChild<Label>(tr("Multicolor"));
            m_caption->setTextAlignment(Label::TextAlignment::Center);
            m_caption->setFontSize(11);
        }
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
        return {30, 30 + (m_multicolor ? 17.0 : 0.0)};
    }
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const RectF dot{4, 4, 22, 22};
        const double size = dot.height * 0.6;
        m_check->setSize(size);
        m_check->setColor((m_color.r * .2126 + m_color.g * .7152 + m_color.b * .0722) > 150
                          ? Color::rgb(0x24262c) : Color::rgb(0xffffff));
        m_check->setVisible(m_checked && !m_multicolor);
        m_check->setGeometry({dot.x + (dot.width - size) / 2, dot.y + (dot.height - size) / 2, size, size});
        if (m_caption) {
            m_caption->setColor(theme.disabledText);
            const double width = m_caption->sizeHint().width + 4;
            m_caption->setGeometry({(30 - width) / 2, 31, width, 16});
        }
    }

    void paint(Painter& painter) override
    {
        const RectF ring{0, 0, 30, 30};
        const RoundedShape circle{ring.width / 2, 0, Corners::Circular};
        if (m_checked || m_pressed || hasVisibleFocus())
            painter.drawRoundedRect(ring, circle, Color{0, 0, 0, 0}, 2, m_color);
        const auto dot = ring.shrunk(Margins::all(3));
        if (m_multicolor) {
            painter.fillRoundedRect(dot, 12, LinearGradient::vertical(dot, Color::rgb(0x673ce0), Color::rgb(0x1fcf6a)));
            painter.save();
            painter.clipRoundedRect(dot, 12);
            painter.fillRoundedRect(dot, 12, LinearGradient{
                {dot.x, dot.y}, {dot.right(), dot.y}, {{0, Color{255, 226, 17, 0}}, {1, Color::rgb(0xff7140)}}});
            painter.restore();
        } else {
            painter.drawRoundedRect(dot, {circle.radius, 3, Corners::Circular}, m_color);
        }
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
    static Color swatchColor(int index)
    {
        // Display tones follow the reference palette; the semantic UI accents
        // can be darker to keep selected text and control glyphs readable.
        constexpr std::array display{0x007aff, 0xe23336, 0x5bb746, 0x913991, 0xf54998,
                                     0xf98316, 0x99999b, 0xffc425, 0x96a8d7};
        static_assert(display.size() == accent::palette.size());
        return index >= 0 && index < int(display.size()) ? Color::rgb(display[index]) : accent::blue;
    }

    Color m_color;
    bool m_multicolor;
    Icon* m_check;
    Label* m_caption = nullptr;
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
            painter.save();
            painter.clipRoundedRect(localRect(), 5);
            const auto wash = dark ? Color::rgb(0x0d052d).withAlpha(.42)
                                   : Color::rgb(0x54afe9).withAlpha(.12);
            painter.fillRect(localRect(), wash);
            const RectF toolbar{4, 5, 38, 11};
            toolbar_surface::capsule(painter, toolbar, dark);
            toolbar_surface::highlight(painter, toolbar.shrunk(Margins::all(2)), dark, false, true);
            const RectF window{19, 22, 46, 20};
            painter.drawSmoothRectangle(window, 5, Color::rgb(dark ? 0x25272c : 0xf8f8f9));
            for (const auto [n, color] : std::array{std::pair{0, 0xf7756a}, std::pair{1, 0xfbc83a},
                                                    std::pair{2, 0x79ce4b}})
                painter.drawSmoothRectangle({26.0 + n * 8, 30, 5, 5}, 2.5, Color::rgb(color));
            painter.restore();
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

class GlassSearch : public SearchField
{
public:
    using SearchField::SearchField;
    bool acceptsPointer() const override { return false; }
    bool acceptsFocus() const override { return false; }
};

// Keep the preview local while dragging, then commit the final value to the
// appearance service so dragging doesn't sync its config file per pixel.
class GlassSetting : public View
{
    class Footer : public View
    {
        void paint(Painter& painter) override
        {
            painter.save();
            painter.clipRoundedRect(localRect(), 16);
            painter.fillRect({0, geometry().height - 34, geometry().width, 34}, Color::rgb(0xe8e4df));
            painter.restore();
        }
    };

public:
    GlassSetting()
    {
        m_title = addChild<Label>(tr("Liquid Glass"));
        m_title->setFontSize(13);
        m_title->setWrap(true);
        m_picture = addChild<DesktopWallpaper>();
        m_picture->setName("appearance:wallpaper");
        m_picture->setPreviewRadius(16);
        m_footer = addChild<Footer>();
        m_caption = addChild<Label>(tr("Liquid Glass Preview"));
        m_caption->setVerticallyCentered(true);
        m_caption->setColor(Color::rgb(0x202124));
        m_caption->setFontSize(15);
        m_caption->setWeight(650);
        m_lenses = addChild<View>();
        m_lenses->setName("appearance:previewGlass");
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
        m_slider = addChild<Slider>();
        m_slider->setName("appearance:glass");
        m_slider->setRange(0, 1);
        m_slider->setValue(0.5);
        m_slider->setToolTip(tr("Adjust the opacity and refraction of Liquid Glass"));
        m_slider->onMoved = [this](double value) {
            updateLenses();
            if (!m_slider->isPressed() && onChanged)
                onChanged(value);
        };
        m_slider->onPressedChanged = [this] {
            if (m_slider->isPressed()) {
                m_start = m_slider->value();
            } else if (onChanged && m_start != m_slider->value()) {
                onChanged(m_slider->value());
            }
        };
        setLayoutHints({.fillWidth = true});
    }

    double heightForWidth(double width) const override { return std::ceil(previewWidth(width) * 0.67) + 33; }
    SizeF sizeHint() const override { return {380, heightForWidth(448)}; }

    void setValue(double strength, bool reduceTransparency)
    {
        if (!m_slider->isPressed())
            m_slider->setValue(appearance_style::glassValue(strength));
        m_reduceTransparency = reduceTransparency;
        m_slider->setEnabled(!reduceTransparency);
        updateLenses();
    }

    std::function<void(double)> onChanged;

protected:
    void layout() override
    {
        const double width = previewWidth(geometry().width);
        const double x = geometry().width - width;
        const double height = std::ceil(width * 0.67);
        m_title->setGeometry({0, 0, std::max(0.0, x - 10), 38});
        m_picture->setGeometry({x, 0, width, height});
        m_footer->setGeometry({x, 0, width, height});
        m_caption->setGeometry({x + 16, height - 22, std::max(0.0, width - 32), 22});
        m_lenses->setGeometry(localRect());
        const double bar = std::min(36.0, width / 6);
        const double gap = std::min(8.0, width / 27);
        m_actions->setGeometry({x + gap, gap, bar * 2, bar});
        m_stack->setGeometry({x + 2 * gap + 2 * bar - 1, gap - 1, bar + 2, bar + 2});
        const double searchStart = 3 * gap + 3 * bar;
        m_search->setGeometry({x + searchStart, gap, std::max(0.0, width - searchStart - gap), bar});
        m_slider->setGeometry({x + 17, height + 9, width - 34, 24});
        updateLenses();
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

    void updateLenses()
    {
        std::vector<ViewGlass> panes;
        for (const View* control : {static_cast<View*>(m_actions), static_cast<View*>(m_stack),
                                     static_cast<View*>(m_search)}) {
            if (control->geometry().width <= 0 || control->geometry().height <= 0)
                continue;
            auto pane = toolbar_surface::previewGlass(control->geometry(), m_slider->value(),
                                                       m_reduceTransparency);
            if (Theme::current().darkMode)
                pane.tint[0] = pane.tint[1] = pane.tint[2] = 0.14;
            pane.key = panes.size() + 1;
            panes.push_back(pane);
        }
        m_lenses->setGlass(std::move(panes));
    }

    Label* m_title;
    Label* m_caption;
    View* m_footer;
    DesktopWallpaper* m_picture;
    View* m_lenses;
    HeaderButtonGroup* m_actions;
    HeaderButton* m_stack;
    SearchField* m_search;
    Slider* m_slider;
    double m_start = 0.5;
    bool m_reduceTransparency = false;
};

class AppearanceChoices : public ListRow
{
public:
    explicit AppearanceChoices(std::string text) : ListRow(std::move(text))
    {
        setInteractive(false);
        label()->setFontSize(13);
        label()->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Start,
                                 .margins = {0, 10, 0, 0}});
    }
};

class StylePreview : public View
{
public:
    explicit StylePreview(IconStyle style) : m_style(style) {}
    SizeF sizeHint() const override { return {29, 29}; }

protected:
    void paint(Painter& painter) override
    {
        auto style = Theme::appearanceStyle();
        style.icons = m_style;
        const auto& theme = Theme::current();
        const auto rect = localRect();
        auto layers = appearance_style::layers(style, theme.darkMode, Color::rgb(0x008cff), theme.highlight);
        if (m_style == IconStyle::Clear) {
            layers.top = Color::rgb(0x8e8e8e);
            layers.bottom = Color::rgb(0x686868);
            layers.ink = Color::rgb(0xf7f8fa);
        }
        if (m_style == IconStyle::Tinted && Theme::appearanceStyle().icons == IconStyle::Dark) {
            style.tone = IconTone::Dark;
            layers = appearance_style::layers(style, theme.darkMode, Color::rgb(0x008cff), theme.highlight);
        }
        const Color sun = m_style == IconStyle::Default || m_style == IconStyle::Dark
                ? Color::rgb(0xffcc36) : layers.ink.withAlpha(.68);
        painter.fillRoundedRect(rect, 7, LinearGradient::vertical(rect, layers.top, layers.bottom));
        painter.drawSmoothRectangle(rect, 7, Color{0, 0, 0, 0}, 0.8, layers.rim);
        painter.fillRoundedRect({14, 5, 9, 9}, 4.5, LinearGradient::vertical(
            {14, 5, 9, 9}, appearance_style::blend(sun, Color::rgb(0xffffff), .30), sun));
        Path cloud;
        cloud.moveTo(8.4, 22.6);
        cloud.cubicTo(1.8, 22.6, 2.4, 13.7, 8.1, 13.7);
        cloud.cubicTo(9.1, 7.9, 17.2, 8.1, 18.1, 13.7);
        cloud.cubicTo(25.2, 12.9, 27.0, 22.6, 20.6, 22.6);
        cloud.close();
        const Color cloudTop = m_style == IconStyle::Dark ? Color::rgb(0xf9f9fb) : layers.ink;
        const Color cloudBottom = appearance_style::blend(cloudTop,
            m_style == IconStyle::Dark || m_style == IconStyle::Clear ? Color::rgb(0xa0a6b0)
                                                                    : layers.top, .68);
        painter.fillPath(cloud, LinearGradient::vertical({4, 10, 21, 13}, cloudTop, cloudBottom));
    }

private:
    IconStyle m_style;
};

class ColorChip : public View
{
public:
    SizeF sizeHint() const override { return {16, 16}; }
    void setColor(Color color, bool multicolor = false)
    {
        const bool colorChanged = std::exchange(m_color, color) != color;
        if ((std::exchange(m_multicolor, multicolor) != multicolor) || colorChanged)
            requestPaint();
    }

protected:
    void paint(Painter& painter) override
    {
        if (m_multicolor) {
            painter.fillRoundedRect(localRect(), 8,
                LinearGradient{{0, 8}, {16, 8}, {{0, Color::rgb(0x919af3)},
                                                  {.55, Color::rgb(0xce9cd6)},
                                                  {1, Color::rgb(0xfba18c)}}});
        } else {
            painter.fillRoundedRect(localRect(), 8, m_color);
        }
    }

private:
    Color m_color;
    bool m_multicolor = false;
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
        m_glass = glass->addChild<GlassSetting>();
        m_glass->onChanged = [this](double strength) { m_appearance.setGlassStrength(strength); };

        Section* accents = content()->addChild<Section>(tr("Theme"));
        accents->setLayoutHints({.fillWidth = true, .margins = {0, 22, 0, 0}});
        Card* accentCard = accents->addChild<Card>(Card::Style::List);
        AppearanceChoices* color = accentCard->addChild<AppearanceChoices>(tr("Color"));
        color->setMinimumRowHeight(62);
        FittingRow* swatches = color->addContent<FittingRow>(6, 1);
        for (const int index : kSwatchOrder) {
            AccentSwatch* swatch = swatches->addChild<AccentSwatch>(index);
            swatch->setToolTip(index < 0 ? tr("Multicolor") : tr(kAccentNames[index]));
            swatch->setName("appearance:accent:" + std::to_string(index));
            swatch->onClicked = [this, index] { m_appearance.setAccentColor(index); };
            m_swatches.push_back(swatch);
        }
        swatches->setLayoutHints({.minimumWidth = swatches->minimumWidth()});
        tiles->setLayoutHints({.minimumWidth = tiles->minimumWidth()});

        auto* highlight = accentCard->addChild<ListRow>(tr("Text highlight color"));
        highlight->setInteractive(false);
        m_highlightChip = highlight->addContent<ColorChip>();
        m_highlightPicker = highlight->addContent<RowComboBox>(colorNames());
        m_highlightPicker->setName("appearance:textHighlight");
        m_highlightPicker->onActivated = [this](int i) { m_appearance.setTextHighlight(i); };

        Card* icons = content()->addChild<Card>(Card::Style::List);
        AppearanceChoices* iconStyle = icons->addChild<AppearanceChoices>(tr("Icon & widget style"));
        auto* styleColumn = iconStyle->addContent<ColumnLayout>();
        styleColumn->setSpacing(5);
        styleColumn->setLayoutHints({.verticalAlignment = Alignment::Start, .margins = {0, 10, 0, 0}});
        FittingRow* styles = styleColumn->addChild<FittingRow>(12, 3);
        styles->setLayoutHints({.horizontalAlignment = Alignment::End});
        const std::array<const char*, 4> labels{tr("Default"), tr("Dark"), tr("Clear"), tr("Tinted")};
        for (size_t i = 0; i < labels.size(); ++i) {
            auto* choice = styles->addChild<ImageSelectionItem>(
                 std::make_unique<StylePreview>(IconStyle(i)), labels[i], SizeF{38, 38}, 1, 10);
            choice->setName("appearance:iconStyle:" + std::to_string(i));
            choice->onClicked = [this, i] { m_appearance.setIconStyle(int(i)); };
            m_styles[i] = choice;
        }
        m_tones = styleColumn->addChild<FittingRow>(9, 3);
        m_tones->setName("appearance:iconTones");
        m_tones->setLayoutHints({.horizontalAlignment = Alignment::End});
        const std::array<const char*, 3> tones{tr("Light"), tr("Dark"), tr("Auto")};
        for (size_t i = 0; i < tones.size(); ++i) {
            auto* choice = m_tones->addChild<RadioButton>(tones[i]);
            choice->setGroup(&m_toneGroup);
            choice->setName("appearance:iconTone:" + std::to_string(i));
            m_toneChoices[i] = choice;
        }
        m_toneGroup.onChanged = [this](int i) { m_appearance.setIconTone(i); };
        iconStyle->setMinimumRowHeight(91);
        m_folderColor = icons->addChild<ListRow>(tr("Folder color"));
        m_folderColor->setInteractive(false);
        m_folderChip = m_folderColor->addContent<ColorChip>();
        m_folderPicker = m_folderColor->addContent<RowComboBox>(colorNames());
        m_folderPicker->setName("appearance:folderColor");
        m_folderPicker->onActivated = [this](int i) {
            if (m_tinted)
                m_appearance.setIconTint(i);
            else
                m_appearance.setFolderColor(i);
        };

        Card* transparency = content()->addChild<Card>(Card::Style::List);
        m_wallpaperTinting = transparency->addChild<SwitchRow>(tr("Allow wallpaper tinting in windows"));
        m_wallpaperTinting->setName("appearance:wallpaperTint");
        m_wallpaperTinting->onToggled = [this](bool checked) { m_appearance.setWallpaperTinting(checked); };
        m_reduceTransparency = transparency->addChild<SwitchRow>(tr("Reduce Transparency"));
        m_reduceTransparency->setName("appearance:reduceTransparency");
        m_reduceTransparency->onToggled = [this](bool checked) { m_appearance.setReduceTransparency(checked); };
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
    static std::vector<std::string> colorNames()
    {
        return {tr("Automatic"), tr("Blue"), tr("Purple"), tr("Pink"), tr("Red"), tr("Orange"),
                tr("Yellow"), tr("Green"), tr("Graphite"), tr("Indigo")};
    }

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
        const auto& fallback = Theme::appearanceStyle();
        const int accent = m_appearance.isAvailable() ? m_appearance.accentColor() : fallback.accentColor;
        for (size_t i = 0; i < m_swatches.size(); ++i)
            m_swatches[i]->setChecked(accent == kSwatchOrder[i]);
        const int highlight = m_appearance.isAvailable() ? m_appearance.textHighlight() : fallback.textHighlight;
        m_highlightPicker->setCurrentIndex(appearance_style::colorIndex(highlight));
        m_highlightChip->setColor(appearance_style::color(highlight, theme.highlight), highlight == 0 && accent == -1);
        const auto style = m_appearance.isAvailable() ? appearance_style::iconStyle(m_appearance.iconStyle())
                                                     : fallback.icons;
        const auto tone = m_appearance.isAvailable() ? appearance_style::iconTone(m_appearance.iconTone())
                                                     : fallback.tone;
        const bool reduced = m_appearance.isAvailable() ? m_appearance.reduceTransparency()
                                                       : fallback.reduceTransparency;
        m_glass->setValue(m_appearance.isAvailable() ? m_appearance.glassStrength() : fallback.glass, reduced);
        for (size_t i = 0; i < m_styles.size(); ++i)
            m_styles[i]->setChecked(IconStyle(i) == style);
        m_tinted = style == IconStyle::Tinted;
        m_tones->setVisible(m_tinted || style == IconStyle::Clear);
        m_toneChoices[size_t(tone)]->setChecked(true);
        m_folderColor->setText(m_tinted ? tr("Icon, widget & folder color") : tr("Folder color"));
        const int index = m_appearance.isAvailable() ? (m_tinted ? m_appearance.iconTint() : m_appearance.folderColor())
                                                     : (m_tinted ? fallback.tintColor : fallback.folderColor);
        m_folderPicker->setCurrentIndex(appearance_style::colorIndex(index));
        m_folderChip->setColor(appearance_style::color(index, m_tinted ? theme.highlight : Color::rgb(0x008cff)));
        m_wallpaperTinting->setChecked(m_appearance.wallpaperTinting());
        m_reduceTransparency->setChecked(reduced);
        m_coloredWindowControls->setChecked(m_appearance.coloredWindowControls());
    }

    Appearance m_appearance;
    cutefish::base::Subscription m_subscription;
    ImageSelectionItem* m_light;
    ImageSelectionItem* m_dark;
    ImageSelectionItem* m_auto;
    GlassSetting* m_glass;
    std::array<ImageSelectionItem*, 4> m_styles;
    ButtonGroup m_toneGroup;
    FittingRow* m_tones;
    std::array<RadioButton*, 3> m_toneChoices;
    ListRow* m_folderColor;
    ColorChip* m_folderChip;
    RowComboBox* m_folderPicker;
    ColorChip* m_highlightChip;
    RowComboBox* m_highlightPicker;
    bool m_tinted = false;
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
