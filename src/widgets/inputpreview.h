// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "widgets/card.h"

namespace settings {

class InputPreview : public cutefish::ui::RowLayout
{
public:
    enum class Device { Mouse, Trackpad };
    explicit InputPreview(Device device);
};

} // namespace settings
