/*
 * Copyright 2016-2017 Canonical Ltd.
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

#include "actions.h"

#include <glib.h>
#include <gio/gio.h>

namespace unity {
namespace indicator {
namespace printers {

/***
****
***/

namespace
{

GVariant* create_default_header_state()
{
    GVariantBuilder b;
    g_variant_builder_init(&b, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&b, "{sv}", "accessible-desc", g_variant_new_string("accessible-desc"));
    g_variant_builder_add(&b, "{sv}", "label", g_variant_new_string("label"));
    g_variant_builder_add(&b, "{sv}", "title", g_variant_new_string("title"));
    g_variant_builder_add(&b, "{sv}", "visible", g_variant_new_boolean(true));
    return g_variant_builder_end(&b);
}

} // unnamed namespace

/***
****
***/

Actions::Actions():
    m_actions(g_simple_action_group_new())
{
    // add the header actions
    auto gam = G_ACTION_MAP(m_actions);
    auto v = create_default_header_state();
    auto a = g_simple_action_new_stateful("printers", nullptr, v);
    g_action_map_add_action(gam, G_ACTION(a));
    g_object_unref(a);
}

Actions::~Actions()
{
    g_clear_object(&m_actions);
}

GActionGroup* Actions::action_group()
{
    return G_ACTION_GROUP(m_actions);
}


} // namespace printers
} // namespace indicator
} // namespace unity
