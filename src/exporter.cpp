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

#include "exporter.h"

#include <core/signal.h>

namespace unity {
namespace indicator {
namespace printers {

class Exporter::Impl
{
public:

    Impl()
    {
    }

    ~Impl()
    {
        if (m_bus != nullptr) {
            for(auto& id : m_exported_menu_ids) {
                g_dbus_connection_unexport_menu_model(m_bus, id);
            }

            if (m_exported_actions_id) {
                g_dbus_connection_unexport_action_group(m_bus,
                                                        m_exported_actions_id);
            }
        }

        if (m_own_id) {
            g_bus_unown_name(m_own_id);
        }

        g_clear_object(&m_bus);
    }

    core::Signal<> name_lost;

    void publish(const std::shared_ptr<Actions>& actions,
                 const std::shared_ptr<Client>& client)
    {
        m_actions = actions;
        m_client = client;
        m_menu = new Menu(m_actions, m_client);
        m_own_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                  INDICATOR_BUS_NAME,
                                  G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT,
                                  on_bus_acquired,
                                  nullptr,
                                  on_name_lost,
                                  this,
                                  nullptr);
    }

private:
    static void on_bus_acquired(GDBusConnection* connection,
                                const gchar* name,
                                gpointer gthis)
    {
        g_debug("bus acquired: %s", name);
        static_cast<Impl*>(gthis)->on_bus_acquired(connection, name);
    }

    void on_bus_acquired(GDBusConnection* bus, const gchar* /*name*/)
    {
        m_bus = static_cast<GDBusConnection*>(g_object_ref(G_OBJECT(bus)));

        // export the actions
        GError * error = nullptr;
        const auto id = g_dbus_connection_export_action_group(m_bus,
                                                              INDICATOR_BUS_PATH,
                                                              m_actions->action_group(),
                                                              &error);
        if (id)
        {
            m_exported_actions_id = id;
        }
        else
        {
            g_warning("cannot export action group: %s", error->message);
            g_clear_error(&error);
        }

        // export the menus
        for(int i = 0, n = Menu::NUM_PROFILES; i < n; i++) {
            const auto profile_name = Menu::profile_name(static_cast<Menu::Profile>(i));
            const auto path = std::string(INDICATOR_BUS_PATH) + "/" + profile_name;
            const auto id = g_dbus_connection_export_menu_model(m_bus,
                                                                path.c_str(),
                                                                m_menu->menu_model(),
                                                                &error);
            if (id) {
                m_exported_menu_ids.insert(id);
            } else {
                if (error != nullptr) {
                    g_warning("cannot export %s menu: %s",
                              profile_name.c_str(), error->message);
                }
                g_clear_error(&error);
            }
        }
        m_client->create_subscription();
        m_client->refresh();
    }

    static void on_name_lost(GDBusConnection*, const gchar* name,
                             gpointer gthis)
    {
        g_debug("name lost: %s", name);
        static_cast<Impl*>(gthis)->name_lost();
    }


    std::set<guint> m_exported_menu_ids;
    guint m_own_id = 0;
    guint m_exported_actions_id = 0;
    GDBusConnection* m_bus = nullptr;
    std::shared_ptr<Actions> m_actions;
    std::shared_ptr<Client> m_client;
    Menu* m_menu;
};



Exporter::Exporter():
    p(new Impl())
{
}


Exporter::~Exporter()
{
}

core::Signal<>& Exporter::name_lost()
{
    return p->name_lost;
}

void Exporter::publish(const std::shared_ptr<Actions>& actions,
                       const std::shared_ptr<Client>& client)
{
    p->publish(actions, client);
}


} // namespace printers
} // namespace indicator
} // namespace unity
