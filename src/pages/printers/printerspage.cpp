// SPDX-License-Identifier: BSD-3-Clause
#ifdef SETTINGS_WITH_CUPS
#include <cups/cups.h>

#include <algorithm>

#include "base/i18n.h"
#include "base/mime_apps.h"
#include "base/thread_pool.h"
#include "ui/label.h"
#include "widgets/card.h"
#include "widgets/formdialog.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/rowicon.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {
namespace {

struct Printer
{
    std::string name;
    std::string instance;
    std::string model;
    std::string state;
    bool isDefault;

    std::string label() const { return instance.empty() ? name : name + "/" + instance; }
};

struct Queues
{
    std::vector<Printer> printers;
    bool available = false;
    bool local = false;
    bool saved = true;
};

Queues queryPrinters(const std::optional<Printer>& chosen)
{
    Queues result;
    const std::string server = cupsServer();
    result.local = server.starts_with('/') || server == "localhost" || server == "127.0.0.1" || server == "::1";
    std::unique_ptr<http_t, decltype(&httpClose)> connection(
        httpConnect2(server.c_str(), ippPort(), nullptr, AF_UNSPEC, cupsEncryption(), 1, 1000, nullptr), httpClose);
    if (!connection)
        return result;
    result.available = true;
    httpSetTimeout(connection.get(), 2.0, nullptr, nullptr);
    cups_dest_t* destinations = nullptr;
    const int count = cupsGetDests2(connection.get(), &destinations);
    if (chosen) {
        const char* instance = chosen->instance.empty() ? nullptr : chosen->instance.c_str();
        result.saved = cupsGetDest(chosen->name.c_str(), instance, count, destinations) != nullptr;
        if (result.saved) {
            cupsSetDefaultDest(chosen->name.c_str(), instance, count, destinations);
            result.saved = cupsSetDests2(connection.get(), count, destinations) == 0;
        }
    }
    for (int i = 0; i < count; ++i) {
        const auto& dest = destinations[i];
        const auto option = [&dest](const char* key) {
            const char* value = cupsGetOption(key, dest.num_options, dest.options);
            return std::string(value ? value : "");
        };
        result.printers.push_back({dest.name, dest.instance ? dest.instance : "",
                                   option("printer-make-and-model"), option("printer-state"), dest.is_default != 0});
    }
    cupsFreeDests(count, destinations);
    std::sort(result.printers.begin(), result.printers.end(),
              [](const Printer& a, const Printer& b) { return a.label() < b.label(); });
    return result;
}

std::string stateText(const Printer& printer)
{
    if (printer.state == "3")
        return tr("Idle");
    if (printer.state == "4")
        return tr("Printing");
    if (printer.state == "5")
        return tr("Stopped");
    return tr("Unknown");
}

class PrintersPage : public Page
{
public:
    explicit PrintersPage(SettingsWindow& window) : Page(window, tr("Printers & Scanners"))
    {
        auto* preferences = content()->addChild<Card>(Card::Style::List);
        auto* row = preferences->addChild<ListRow>(tr("Default printer"));
        row->setInteractive(false);
        m_default = row->addContent<RowComboBox>();
        m_default->onActivated = [this](int index) {
            if (index >= 0 && size_t(index) < m_printers.size())
                load(m_printers[size_t(index)]);
        };

        auto* printers = content()->addChild<Section>(tr("Printers"));
        m_rows = printers->addChild<Card>(Card::Style::List);
        m_status = printers->addChild<Footnote>();
        auto* buttons = content()->addChild<RowLayout>();
        buttons->setSpacing(8);
        buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
        m_refresh = buttons->addChild<StandardButton>(tr("Refresh"));
        m_refresh->onClicked = [this] { load(); };
        m_manage = buttons->addChild<StandardButton>(tr("Add or Manage Printers…"));
        m_manage->onClicked = [this] {
            base::mimeapps::openUri("http://localhost:" + std::to_string(ippPort()) + "/admin/");
        };
        m_manage->setVisible(false);
        load();
    }

private:
    void load(std::optional<Printer> chosen = {})
    {
        m_default->setEnabled(false);
        m_refresh->setEnabled(false);
        m_status->setVisible(true);
        m_status->setText(tr("Looking for printers…"));
        m_job = base::runInThread([chosen = std::move(chosen)] { return queryPrinters(chosen); },
                                  [this](Queues result) { update(std::move(result)); });
    }

    void update(Queues result)
    {
        m_printers = std::move(result.printers);
        m_status->setVisible(m_printers.empty() || !result.saved);
        m_status->setText(!result.available ? tr("The CUPS printing service is unavailable.")
            : !result.saved ? tr("Could not change the default printer.") : tr("No printers configured."));
        m_refresh->setEnabled(true);
        m_manage->setVisible(result.available && result.local);
        while (!m_rows->children().empty())
            m_rows->takeChild(m_rows->children().front().get());
        m_rows->setVisible(!m_printers.empty());
        std::vector<std::string> names;
        int current = -1;
        for (const Printer& printer : m_printers) {
            if (printer.isDefault)
                current = int(names.size());
            names.push_back(printer.label());
            auto* row = m_rows->addChild<NavigationItem>(printer.label());
            row->setLeading<RowIcon>("printer.svg", Color::rgb(0x62656b));
            row->setMinimumRowHeight(48);
            row->setValue(stateText(printer));
            row->onClicked = [this, printer] { showPrinter(printer); };
        }
        const bool empty = names.empty();
        m_default->setItems(std::move(names));
        m_default->setDisplayText(current < 0 ? std::optional<std::string>(tr("None")) : std::nullopt);
        m_default->setCurrentIndex(current);
        m_default->setEnabled(!empty);
    }

    void showPrinter(const Printer& printer)
    {
        m_details = std::make_unique<FormDialog>(settingsWindow(), printer.label(), 365);
        auto* list = m_details->content()->addChild<Card>(Card::Style::List);
        list->addChild<InfoRow>(tr("Model"), printer.model);
        list->addChild<InfoRow>(tr("Status"), stateText(printer));
        m_details->show();
    }

    RowComboBox* m_default;
    Card* m_rows;
    Footnote* m_status;
    StandardButton* m_refresh;
    StandardButton* m_manage;
    std::vector<Printer> m_printers;
    std::unique_ptr<FormDialog> m_details;
    base::Job m_job;
};

const PageRegistration registration("printers", [](SettingsWindow& window) {
    return std::make_unique<PrintersPage>(window);
});

} // namespace
} // namespace settings
#endif
