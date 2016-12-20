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

#include "menu.h"

#include <glib/gi18n.h>
#include <gio/gio.h>

#include <algorithm>
#include <iterator>
#include <vector>

namespace unity {
namespace indicator {
namespace printers {


class Menu::Impl
{
public:
    Impl(const std::shared_ptr<Actions>& actions,
         const std::shared_ptr<CupsClient>& client):
        m_actions(actions),
        m_client(client)
    {
        // initialize the menu
        create_gmenu();
    }

    virtual ~Impl()
    {
        g_clear_object(&m_menu);
    }

    void update_header()
    {
        auto action_group = m_actions->action_group();
        std::string action_name{ "indicator.printers.root"};
        auto state = create_header_state();
        g_action_group_change_action_state(action_group, action_name.c_str(), state);
    }

    GVariant* get_serialized_icon(const std::string& icon_name)
    {
        auto icon = g_themed_icon_new_with_default_fallbacks(icon_name.c_str());
        auto ret = g_icon_serialize(icon);
        g_object_unref(icon);

        return ret;
    }

    GVariant* create_header_state()
    {
        const auto title = _("Printers");

        GVariantBuilder b;
        g_variant_builder_init(&b, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(&b, "{sv}", "title", g_variant_new_string(title));
        g_variant_builder_add(&b, "{sv}", "icon", get_serialized_icon("printer-symbolic"));
        g_variant_builder_add(&b, "{sv}", "visible", g_variant_new_boolean(true));
        return g_variant_builder_end(&b);
    }

    GMenuModel* menu_model()
    {
        return G_MENU_MODEL(m_menu);
    }

private:
    std::shared_ptr<Actions> m_actions;
    std::shared_ptr<CupsClient> m_client;

    GMenu* m_submenu = nullptr;
    GMenu* m_menu = nullptr;

    void create_gmenu()
    {
        g_assert(m_submenu == nullptr);

        m_submenu = g_menu_new();

        // add submenu to the header
        const std::string detailed_action{"indicator.printers.root"};
        auto header = g_menu_item_new(nullptr, detailed_action.c_str());
        g_menu_item_set_attribute(header, "action-namespace", "s",
                                  "indicator.printers");
        g_menu_item_set_attribute(header, "x-canonical-type", "s",
                                  "com.canonical.indicator.root");
        g_menu_item_set_submenu(header, G_MENU_MODEL(m_submenu));
        g_object_unref(m_submenu);

        // add header to the menu
        m_menu = g_menu_new();
        g_menu_append_item(m_menu, header);
        g_object_unref(header);
    }


}; // class MenuImpl


Menu::Menu(const std::shared_ptr<Actions>& actions,
           const std::shared_ptr<CupsClient>& client):
    p(new Impl(actions, client))
{
}

GMenuModel* Menu::menu_model()
{
    return p->menu_model();
}

std::string Menu::profile_name(Profile profile)
{
    switch (profile) {
    case Menu::Desktop:
        return "desktop";
        break;

    case Menu::DesktopGreeter:
        return "desktop_greeter";
        break;

    case Menu::Phone:
        return "phone";
        break;

    case Menu::PhoneGreeter:
        return "phone_greeter";
        break;

    default:
        break;
    }
    return "unknown";
}

} // namespace printers
} // namespace indicator
} // namespace unity
