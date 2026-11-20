// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>

#include "base/i18n.h"
#include "cutefish/networkmanager/connections.h"
#include "cutefish/networkmanager/handler.h"
#include "pages/network/common/networklist.h"
#include "ui/cell_layout.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/paneintro.h"
#include "widgets/rowicon.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;
namespace nm = cutefish::networkmanager;

namespace settings::network {

namespace {

// Network services that aren't a device of their own, each on a page of its own.
class NetworkPage : public Page
{
public:
    explicit NetworkPage(SettingsWindow& window)
        : Page(window, tr("Network"))
    {
        auto* devices = content()->addChild<Card>(Card::Style::List);
        m_wifi = addPageLink(devices, window, "wlan", true);
        m_ethernet = addPageLink(devices, window, "ethernet", true);

        auto* services = content()->addChild<Section>(tr("Other Services"));
        Card* card = services->addChild<Card>(Card::Style::List);
        auto* vpn = addPageLink(card, window, "vpn", true);
        if (vpn)
            vpn->setName("network:vpn");

        NavigationItem* proxy = card->addChild<NavigationItem>(tr("Proxy"));
        proxy->setName("network:proxy");
        proxy->setLeading<RowIcon>("proxy.svg", Color::rgb(0x0067FF));
        proxy->onClicked = [this] { settingsWindow().pushPage("proxy"); };

        m_hotspot = card->addChild<NavigationItem>(tr("Hotspot"));
        m_hotspot->setName("network:hotspot");
        m_hotspot->setLeading<RowIcon>("network-wireless-hotspot", Color::rgb(0x2EBC6B));
        m_hotspot->onClicked = [this] { settingsWindow().pushPage("hotspot"); };

        m_subscriptions.push_back(m_enabled.changed.connect([this] { update(); }));
        m_subscriptions.push_back(m_handler.hotspotSupportedChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_connections.changed.connect([this] { update(); }));
        update();
    }

private:
    void update()
    {
        // Shown until NetworkManager has been read, so the row doesn't pop in late.
        m_hotspot->setVisible(!m_enabled.isReady() || m_handler.hotspotCapable());
        updateDevice(m_wifi, nm::ConnectionType::Wireless);
        updateDevice(m_ethernet, nm::ConnectionType::Wired);
    }

    void updateDevice(NavigationItem* row, nm::ConnectionType type)
    {
        if (!row)
            return;
        const auto found = m_connections.rows(type);
        row->setVisible(type == nm::ConnectionType::Wireless || !found.empty());
        const bool connected = std::any_of(found.begin(), found.end(), [](const auto& item) {
            return item.connectionState == nm::ConnectionStatus::Activated;
        });
        row->setValue(connected ? tr("Connected") : tr("Not connected"));
    }

    NetworkList m_connections;
    nm::EnabledConnections m_enabled;
    nm::Handler m_handler;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    NavigationItem* m_hotspot;
    NavigationItem* m_wifi;
    NavigationItem* m_ethernet;
};

} // namespace

static const PageRegistration registration("network", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<NetworkPage>(window);
});

} // namespace settings::network
