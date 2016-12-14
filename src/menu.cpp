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


Menu::Menu (Profile profile_in, const std::string& name_in):
    m_profile(profile_in),
    m_name(name_in)
{
}

const std::string& Menu::name() const
{
    return m_name;
}

Menu::Profile Menu::profile() const
{
    return m_profile;
}

GMenuModel* Menu::menu_model()
{
    return G_MENU_MODEL(m_menu);
}


class MenuImpl: public Menu
{
protected:
    MenuImpl(const Menu::Profile profile_in,
             const std::string& name_in,
             std::shared_ptr<Actions>& actions):
        Menu(profile_in, name_in),
        m_actions(actions)
    {
        // initialize the menu
        create_gmenu();
    }

    virtual ~MenuImpl()
    {
        g_clear_object(&m_menu);
    }

    virtual GVariant* create_header_state() =0;

    void update_header()
    {
        auto action_group = m_actions->action_group();
        auto action_name = name() + "-header";
        auto state = create_header_state();
        g_action_group_change_action_state(action_group, action_name.c_str(), state);
    }

    std::shared_ptr<Actions> m_actions;
    GMenu* m_submenu = nullptr;

private:

    void create_gmenu()
    {
        g_assert(m_submenu == nullptr);

        m_submenu = g_menu_new();

        // add submenu to the header
        const auto detailed_action = std::string("indicator.") + name() + "-header";
        auto header = g_menu_item_new(nullptr, detailed_action.c_str());
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



/***
****
***/

class DesktopBaseMenu: public MenuImpl
{
protected:
    DesktopBaseMenu(Menu::Profile profile_,
                    const std::string& name_,
                    std::shared_ptr<Actions>& actions_):
        MenuImpl(profile_, name_, actions_)
    {
        update_header();
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
};

class DesktopMenu: public DesktopBaseMenu
{
public:
    DesktopMenu(std::shared_ptr<Actions>& actions_):
        DesktopBaseMenu(Desktop,"desktop", actions_) {}
};

class DesktopGreeterMenu: public DesktopBaseMenu
{
public:
    DesktopGreeterMenu(std::shared_ptr<Actions>& actions_):
        DesktopBaseMenu(DesktopGreeter,"desktop_greeter", actions_) {}
};

class PhoneBaseMenu: public MenuImpl
{
protected:
    PhoneBaseMenu(Menu::Profile profile_,
                  const std::string& name_,
                  std::shared_ptr<Actions>& actions_):
        MenuImpl(profile_, name_, actions_)
    {
        update_header();
    }

    GVariant* create_header_state()
    {
        GVariantBuilder b;
        g_variant_builder_init(&b, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(&b, "{sv}", "title", g_variant_new_string (_("Printers")));
        g_variant_builder_add(&b, "{sv}", "visible", g_variant_new_boolean (TRUE));

        return g_variant_builder_end (&b);
    }
};

class PhoneMenu: public PhoneBaseMenu
{
public:
    PhoneMenu(std::shared_ptr<Actions>& actions_):
        PhoneBaseMenu(Phone, "phone", actions_) {}
};

class PhoneGreeterMenu: public PhoneBaseMenu
{
public:
    PhoneGreeterMenu(std::shared_ptr<Actions>& actions_):
        PhoneBaseMenu(PhoneGreeter, "phone_greeter", actions_) {}
};

/****
*****
****/

MenuFactory::MenuFactory(const std::shared_ptr<Actions>& actions_):
    m_actions(actions_)
{
}

std::shared_ptr<Menu>
MenuFactory::buildMenu(Menu::Profile profile)
{
    std::shared_ptr<Menu> menu;

    switch (profile)
    {
    case Menu::Desktop:
        menu.reset(new DesktopMenu(m_actions));
        break;

    case Menu::DesktopGreeter:
        menu.reset(new DesktopGreeterMenu(m_actions));
        break;

    case Menu::Phone:
        menu.reset(new PhoneMenu(m_actions));
        break;

    case Menu::PhoneGreeter:
        menu.reset(new PhoneGreeterMenu(m_actions));
        break;

    default:
        g_warn_if_reached();
        break;
    }

    return menu;
}

/****
*****
****/

} // namespace printers
} // namespace indicator
} // namespace unity
