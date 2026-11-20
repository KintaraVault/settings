// SPDX-License-Identifier: BSD-3-Clause
#include "widgets/inputpreview.h"

#include "widgets/picture.h"

using namespace cutefish::ui;

namespace settings {

InputPreview::InputPreview(Device device)
{
    setName("input:preview");
    setLayoutHints({.fillWidth = true, .preferredHeight = 144});
    setSpacing(8);
    for (const char* asset : {device == Device::Mouse ? "mouse.svg" : "trackpad.svg", "desktop.svg"}) {
        auto* image = addChild<Picture>(dataPath(std::string("images/input/") + asset));
        image->setCornerRadius(12);
        image->setLayoutHints({.fillWidth = true, .fillHeight = true, .preferredWidth = 160});
    }
}

} // namespace settings
