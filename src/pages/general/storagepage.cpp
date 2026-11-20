// SPDX-License-Identifier: BSD-3-Clause
#include <algorithm>
#include <filesystem>

#include <sys/stat.h>

#include "base/i18n.h"
#include "base/paths.h"
#include "pages/about/aboutinfo.h"
#include "ui/color_provider.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "widgets/listrow.h"
#include "widgets/paneintro.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {
namespace {

class StorageBar : public View
{
public:
    explicit StorageBar(double used) : m_used(used) { setLayoutHints({.fillWidth = true}); }
    SizeF sizeHint() const override { return {100, 12}; }

protected:
    void paint(Painter& painter) override
    {
        painter.drawRoundedRect(localRect(), {6}, color(ColorId::SliderTrack));
        const double width = geometry().width * m_used;
        if (width > 0)
            painter.drawRoundedRect({0, 0, width, geometry().height}, {6}, color(ColorId::Highlight));
    }

private:
    double m_used;
};

void volume(ColumnLayout* content, const std::string& path, const char* title)
{
    std::error_code error;
    const auto space = std::filesystem::space(path, error);
    if (error || !space.capacity)
        return;
    auto* summary = content->addChild<Card>();
    summary->addChild<Label>(title)->setWeight(600);
    summary->addChild<StorageBar>(1 - std::min(1.0, double(space.free) / double(space.capacity)));
    auto* info = content->addChild<Card>(Card::Style::List);
    info->addChild<InfoRow>(tr("Total capacity"), about::formatByteSize(double(space.capacity)));
    info->addChild<InfoRow>(tr("Available"), about::formatByteSize(double(space.available)));
}

class StoragePage : public Page
{
public:
    explicit StoragePage(SettingsWindow& window) : Page(window, tr("Storage"))
    {
        volume(content(), "/", tr("System Volume"));
        const auto home = cutefish::base::paths::home();
        struct stat rootInfo {}, homeInfo {};
        if (stat("/", &rootInfo) == 0 && stat(home.c_str(), &homeInfo) == 0 && rootInfo.st_dev != homeInfo.st_dev)
            volume(content(), home, tr("Home Volume"));
    }
};

const PageRegistration registration("storage", [](SettingsWindow& window) {
    return std::make_unique<StoragePage>(window);
});

} // namespace
} // namespace settings
