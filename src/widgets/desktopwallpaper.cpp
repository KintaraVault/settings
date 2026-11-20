// SPDX-License-Identifier: BSD-3-Clause
#include "widgets/desktopwallpaper.h"

#include <charconv>

#include "base/paths.h"
#include "base/settings.h"
#include "base/url.h"
#include "cutefish/appearance/appearance.h"
#include "ui/theme.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

class DesktopBackgroundSource
{
public:
    DesktopBackgroundSource()
    {
        subscription = appearance.changed.connect([this] { update(); });
        update();
    }

    void update()
    {
        std::string wallpaper = appearance.wallpaper();
        bool solid = appearance.backgroundType() == 1;
        std::string rgb = appearance.backgroundColor();
        if (wallpaper.empty() && rgb.empty()) {
            // Also show the real wallpaper while the service's first reply
            // is pending or when the preview is run outside the session.
            base::Settings config(base::paths::cutefishConfig("services"));
            wallpaper = config.stringValue("Appearance/Wallpaper", "/usr/share/backgrounds/cutefishos/default.jpg");
            solid = config.intValue("Appearance/BackgroundType") == 1;
            rgb = config.stringValue("Appearance/BackgroundColor", "#2B8ADA");
        }
        path = base::url::toLocalFile(wallpaper).value_or(wallpaper);
        color.reset();
        if (solid) {
            unsigned value = 0;
            if (rgb.size() == 7 && rgb.front() == '#')
                std::from_chars(rgb.data() + 1, rgb.data() + rgb.size(), value, 16);
            color = Color::rgb(value);
        }
        changed.emit();
    }

    appearance::Appearance appearance;
    base::Subscription subscription;
    base::Signal<> changed;
    std::string path;
    std::optional<Color> color;
};

static std::shared_ptr<DesktopBackgroundSource> backgroundSource()
{
    static std::weak_ptr<DesktopBackgroundSource> previous;
    if (auto existing = previous.lock())
        return existing;
    auto source = std::make_shared<DesktopBackgroundSource>();
    previous = source;
    return source;
}

DesktopWallpaper::DesktopWallpaper()
    : m_background(backgroundSource())
    , m_subscription(m_background->changed.connect([this] { updateBackground(); }))
{
    setFillMode(FillMode::PreserveAspectCrop);
    setAutoTransform(true);
    updateBackground();
}

void DesktopWallpaper::setPreviewRadius(double radius)
{
    m_radius = radius;
    setCornerRadius(radius);
}

const std::string& DesktopWallpaper::wallpaperPath() const
{
    return m_background->path;
}

bool DesktopWallpaper::isSolidColor() const
{
    return m_background->color.has_value();
}

void DesktopWallpaper::updateBackground()
{
    const std::string sourcePath = isSolidColor() ? std::string() : wallpaperPath();
    if (sourcePath != source())
        setSource(sourcePath);
    requestPaint();
}

void DesktopWallpaper::paint(Painter& painter)
{
    // A neutral surface if the wallpaper is still loading or can't be read.
    painter.drawSmoothRectangle(localRect(), m_radius, m_background->color.value_or(Theme::current().alternateBackground));
    if (!isSolidColor())
        ImageView::paint(painter);
}

} // namespace settings
