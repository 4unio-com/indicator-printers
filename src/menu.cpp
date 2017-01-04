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
#include <map>

namespace unity {
namespace indicator {
namespace printers {


class Menu::Impl
{
public:

    static constexpr char const * ATTRIBUTE_JOB_ID {"x-canonical-job-id"};
    static constexpr char const * ATTRIBUTE_TYPE {"x-canonical-type"};
    static constexpr char const * TYPE_SECTION {"section"};

    enum Section {
        CURRENT,
        QUEUED,
        PAUSED,
        NUM_SECTIONS
    };

    Impl(const std::shared_ptr<Actions>& actions,
         const std::shared_ptr<Client>& client):
        m_actions(actions),
        m_client(client)
    {
        // initialize the menu
        create_gmenu();
        update_header();

        m_client->job_state_changed().connect([this](const Job& job) {
                const auto printer = job.printer;
                g_debug("State changed for job %u '%s` on printer '%s'",
                        job.id,
                        job.name.c_str(),
                        printer.description.empty() ? printer.name.c_str() : printer.description.c_str());

                // Remove any existing menu item for the job.
                remove(job);

                Section section = NUM_SECTIONS;
                switch (job.state) {
                case Job::State::PENDING:
                    section = QUEUED;
                    break;
                case Job::State::HELD:
                    section = PAUSED;
                    break;
                case Job::State::PROCESSING:
                    section = CURRENT;
                    break;
                case Job::State::STOPPED:
                case Job::State::CANCELED:
                case Job::State::ABORTED:
                case Job::State::COMPLETED:
                    update_header();
                    return;
                }

                auto model = menu_model_for_section(section);
                if (model != nullptr) {
                    auto item = create_job_menu_item(job);
                    g_menu_append_item(G_MENU(model), item);
                    g_object_unref(item);
                    m_current_jobs[job.id] = section;
                }

                update_header();
            });
    }

    virtual ~Impl()
    {
        g_clear_object(&m_menu);
    }

    void update_header()
    {
        auto action_group = m_actions->action_group();
        std::string action_name{ "printers"};
        auto state = create_header_state();
        g_action_group_change_action_state(action_group,
                                           action_name.c_str(),
                                           state);
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
        auto title_v = g_variant_new_string(_("Printers"));
        const bool visible = !m_current_jobs.empty();
        GVariantBuilder b;
        g_variant_builder_init(&b, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(&b, "{sv}", "title", title_v);
        g_variant_builder_add(&b, "{sv}", "accessible-desc", title_v);
        g_variant_builder_add(&b, "{sv}", "icon", get_serialized_icon("printer-symbolic"));
        g_variant_builder_add(&b, "{sv}", "visible", g_variant_new_boolean(visible));
        return g_variant_builder_end(&b);
    }

    GMenuModel* menu_model()
    {
        return G_MENU_MODEL(m_menu);
    }

    GMenuModel* menu_model_for_section(Section section)
    {
        return g_menu_model_get_item_link(G_MENU_MODEL(m_submenu),
                                          section,
                                          G_MENU_LINK_SECTION);
    }

    void remove(const Job& job)
    {
        Section section = NUM_SECTIONS;
        if(get_current_section_for_job(job, section)) {
            auto model = menu_model_for_section(section);
            auto value = g_variant_new_uint32(job.id);
            auto pos = find_matching_menu_item(model, ATTRIBUTE_JOB_ID, value);

            if (pos >= 0) {
                g_menu_remove(G_MENU(model), pos);
                m_current_jobs.erase(job.id);
            }
        }
    }

private:
    std::shared_ptr<Actions> m_actions;
    std::shared_ptr<Client> m_client;
    std::map<uint32_t, Section> m_current_jobs;

    GMenu* m_submenu = nullptr;
    GMenu* m_menu = nullptr;

    static GMenuItem* create_section_menu_item(const Section& section)
    {
        const char* label;
        switch (section) {
        case CURRENT:
            label = _("Current jobs");
            break;
        case QUEUED:
            label = _("Queued jobs");
            break;
        case PAUSED:
            label = _("Paused jobs");
            break;
        default:
            return nullptr;
        }

        // create a new item
        auto item = g_menu_item_new(label, "indicator.section");
        g_menu_item_set_attribute(item, ATTRIBUTE_TYPE, "s", TYPE_SECTION);

        return item;
    }

    // Create the GMenuItem for the job.
    static GMenuItem* create_job_menu_item(const Job& job)
    {
        GMenuItem* menu_item = nullptr;

        menu_item = g_menu_item_new(job.name.c_str(), nullptr);
        g_menu_item_set_attribute(menu_item, ATTRIBUTE_JOB_ID, "u", job.id);

        return menu_item;
    }

    // find the position of the menumodel's item with the specified attribute
    static int find_matching_menu_item(GMenuModel* mm, const char* attr,
                                       GVariant* value)
    {
        g_return_val_if_fail(value != nullptr, -1);

        for (int i = 0, n = g_menu_model_get_n_items(mm); i < n; ++i) {
            auto test = g_menu_model_get_item_attribute_value(mm,
                                                              i,
                                                              attr,
                                                              nullptr);
            const bool equal = g_variant_equal(value, test);
            g_clear_pointer(&test, g_variant_unref);
            if (equal) {
                return i;
            }
        }

        return -1;
    }

    bool get_current_section_for_job(const Job& job, Section& section) const
    {
        const auto it = m_current_jobs.find(job.id);
        if (it == m_current_jobs.end()) {
            return false;
        }

        section = it->second;
        return true;
    }

    void create_gmenu()
    {
        g_assert(m_submenu == nullptr);

        // build the submenu
        m_submenu = g_menu_new();
        for(int i = 0; i < NUM_SECTIONS; i++) {
            // Create the section menu
            auto section = g_menu_new();
            g_menu_append_section(m_submenu, nullptr, G_MENU_MODEL(section));

            // Add the label menuitem for the section
            auto item = create_section_menu_item(static_cast<Section>(i));
            if (item != nullptr) {
                g_menu_insert_item(section, 0, item);
                g_clear_object(&item);
            }

            g_object_unref(section);
        }

        // add submenu to the header
        const std::string detailed_action{"indicator.printers"};
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


Menu::Menu(const std::shared_ptr<Actions>& actions,
           const std::shared_ptr<Client>& client):
    p(new Impl(actions, client))
{
}

Menu::~Menu()
{
}

GMenuModel* Menu::menu_model()
{
    return p->menu_model();
}

} // namespace printers
} // namespace indicator
} // namespace unity
