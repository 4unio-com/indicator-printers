/*
 * Copyright 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cups-client.h"
#include "cups-cupsd-notifier.h"

#include <cups/cups.h>

#include <stdexcept>

namespace unity {
namespace indicator {
namespace printers {

#define NOTIFY_LEASE_DURATION (24 * 60 * 60)


class CupsClient::Impl {
public:
    Impl()
    {
        GError *error = nullptr;

        m_notifier_proxy = notifier_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                           G_DBUS_PROXY_FLAGS_NONE,
                                                           nullptr,
                                                           CUPS_DBUS_PATH,
                                                           nullptr,
                                                           &error);

        if (error != nullptr) {
            std::string msg{"Error creating cups notifier proxy: "};
            throw std::runtime_error(msg + error->message);
            g_clear_error(&error);
        }

        g_object_connect(m_notifier_proxy,
                         "signal::job-created", on_job_changed, this,
                         "signal::job-state", on_job_changed, this,
                         "signal::job-completed", on_job_changed, this,
                         "signal::printer-state_changed", on_printer_state_changed, this,
                         nullptr);
    }

    ~Impl()
    {
        g_clear_object(&m_notifier_proxy);
    }

    // Signals to propagate
    core::Signal<const Printer&>& printer_state_changed()
    {
        return m_printer_state_changed;
    }

    core::Signal<const Job&>& job_state_changed()
    {
        return m_job_state_changed;
    }

    void create_subscription()
    {
        ipp_t *req;
        ipp_t *resp;
        ipp_attribute_t *attr;

        req = ippNewRequest (IPP_CREATE_PRINTER_SUBSCRIPTION);
        ippAddString (req, IPP_TAG_OPERATION, IPP_TAG_URI,
                      "printer-uri", NULL, "/");
        ippAddString (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD,
                      "notify-events", NULL, "all");
        ippAddString (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                      "notify-recipient-uri", NULL, "dbus://");
        ippAddInteger (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                       "notify-lease-duration", NOTIFY_LEASE_DURATION);

        resp = cupsDoRequest (CUPS_HTTP_DEFAULT, req, "/");
        if (!resp || cupsLastError() != IPP_OK) {
            g_warning ("Error subscribing to CUPS notifications: %s\n",
                       cupsLastErrorString ());
            return;
        }

        attr = ippFindAttribute (resp, "notify-subscription-id", IPP_TAG_INTEGER);
        if (attr) {
            m_subscription_id = ippGetInteger (attr, 0);
        } else {
            g_warning ("ipp-create-printer-subscription response doesn't contain "
                       "subscription id.\n");
        }
        ippDelete (resp);

        // Set up to renew the subscription a minute before it expires
        g_timeout_add_seconds(NOTIFY_LEASE_DURATION - 60,
                              on_subscription_timeout,
                              this);
    }

    void renew_subscription()
    {
        ipp_t *req;
        ipp_t *resp;

        req = ippNewRequest (IPP_RENEW_SUBSCRIPTION);
        ippAddInteger (req, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                       "notify-subscription-id", m_subscription_id);
        ippAddString (req, IPP_TAG_OPERATION, IPP_TAG_URI,
                      "printer-uri", NULL, "/");
        ippAddString (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                      "notify-recipient-uri", NULL, "dbus://");
        ippAddInteger (req, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                       "notify-lease-duration", NOTIFY_LEASE_DURATION);

        resp = cupsDoRequest (CUPS_HTTP_DEFAULT, req, "/");
        if (!resp || cupsLastError() != IPP_OK) {
            g_warning ("Error renewing CUPS subscription %d: %s\n",
                       m_subscription_id, cupsLastErrorString ());
            create_subscription();
        }

        ippDelete (resp);
    }

    void cancel_subscription()
    {
        ipp_t *req;
        ipp_t *resp;

        if (m_subscription_id <= 0) {
            return;
        }

        req = ippNewRequest (IPP_CANCEL_SUBSCRIPTION);
        ippAddString (req, IPP_TAG_OPERATION, IPP_TAG_URI,
                      "printer-uri", NULL, "/");
        ippAddInteger (req, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                       "notify-subscription-id", m_subscription_id);

        resp = cupsDoRequest (CUPS_HTTP_DEFAULT, req, "/");
        if (!resp || cupsLastError() != IPP_OK) {
            g_warning ("Error subscribing to CUPS notifications: %s\n",
                       cupsLastErrorString ());
            return;
        }

        ippDelete (resp);
    }

private:
    static gboolean on_subscription_timeout(gpointer gthis)
    {
        static_cast<Impl*>(gthis)->renew_subscription();
        return true;
    }

    static void on_job_changed (Notifier*,
            const char* printer_text,
            const char* printer_uri,
            const char *printer_name,
            unsigned int printer_state,
            const char *printer_state_reasons,
            bool printer_is_accepting_jobs,
            unsigned int job_id,
            unsigned int job_state,
            const char *job_state_reasons,
            const char *job_name,
            unsigned int job_impressions_completed,
            gpointer gthis)
    {
        Printer printer;
        printer.state = static_cast<Printer::State>(printer_state);
        printer.name = printer_name;
        printer.text = printer_text;
        printer.uri = printer_uri;
        printer.state_reasons = printer_state_reasons;
        printer.accepting_jobs = printer_is_accepting_jobs;

        Job job;
        job.printer = printer;
        job.state = static_cast<Job::State>(job_state);
        job.id = job_id;
        job.name = job_name;
        job.state_reasons = job_state_reasons;
        job.impressions_completed = job_impressions_completed;

        static_cast<Impl*>(gthis)->m_job_state_changed(job);
    }

    static void on_printer_state_changed(Notifier*,
                                         const char* text,
                                         const char* uri,
                                         const char* name,
                                         unsigned int state,
                                         const char* state_reasons,
                                         bool is_accepting_jobs,
                                         gpointer gthis)
    {
        Printer printer;
        printer.state = static_cast<Printer::State>(state);
        printer.name = name;
        printer.text = text;
        printer.uri = uri;
        printer.state_reasons = state_reasons;
        printer.accepting_jobs = is_accepting_jobs;
        static_cast<Impl*>(gthis)->m_printer_state_changed(printer);
    }

    int m_subscription_id;
    Notifier* m_notifier_proxy;
    core::Signal<const Printer&> m_printer_state_changed;
    core::Signal<const Job&> m_job_state_changed;
};

CupsClient::CupsClient() :
    p(new Impl())
{
}

CupsClient::~CupsClient()
{
}

core::Signal<const Printer&>& CupsClient::printer_state_changed()
{
    return p->printer_state_changed();
}

core::Signal<const Job&>& CupsClient::job_state_changed()
{
    return p->job_state_changed();
}

void CupsClient::create_subscription()
{
    p->create_subscription();
}

void CupsClient::renew_subscription()
{
    p->renew_subscription();
}

void CupsClient::cancel_subscription()
{
    p->cancel_subscription();
}

} // printers
} // indicators
} // unity
