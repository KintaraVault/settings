// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "base/signal.h"
#include "ui/image_view.h"

namespace settings {

class DesktopBackgroundSource;

// A crop of the live desktop background, or its exact solid colour.
class DesktopWallpaper : public cutefish::ui::ImageView
{
public:
    DesktopWallpaper();

    void setPreviewRadius(double radius);
    const std::string& wallpaperPath() const;
    bool isSolidColor() const;

protected:
    void paint(cutefish::ui::Painter& painter) override;

private:
    void updateBackground();

    std::shared_ptr<DesktopBackgroundSource> m_background;
    cutefish::base::Subscription m_subscription;
    double m_radius = 0;
};

} // namespace settings
