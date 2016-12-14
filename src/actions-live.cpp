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

#include "actions-live.h"

//#include <url-dispatcher.h>

#include <glib.h>

#include <sstream>

namespace unity {
namespace indicator {
namespace printers {

/***
****
***/

LiveActions::LiveActions():
    Actions()
{
}

void LiveActions::execute_command(const std::string& cmdstr)
{
    const auto cmd = cmdstr.c_str();
    g_debug("Issuing command '%s'", cmd);

    GError* error = nullptr;
    if (!g_spawn_command_line_async(cmd, &error))
    {
        g_warning("Unable to start \"%s\": %s", cmd, error->message);
        g_error_free(error);
    }
}

void LiveActions::dispatch_url(const std::string& url)
{
    g_debug("Dispatching url '%s'", url.c_str());
    //url_dispatch_send(url.c_str(), nullptr, nullptr);
}

/***
****
***/

LiveActions::Desktop LiveActions::get_desktop()
{
    static bool cached = false;
    static LiveActions::Desktop result = LiveActions::OTHER;

    if (cached) {
        return result;
    }

    // check for unity8
    if (g_getenv ("MIR_SOCKET") != nullptr) {
        result = LiveActions::UNITY8;
     } else {
        const gchar *xdg_current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
        if (xdg_current_desktop != NULL) {
            gchar **desktop_names = g_strsplit (xdg_current_desktop, ":", 0);
            for (size_t i = 0; desktop_names[i]; ++i) {
                if (!g_strcmp0 (desktop_names[i], "Unity")) {
                    result = LiveActions::UNITY7;
                    break;
                }
            }
            g_strfreev (desktop_names);
        }
    }
    cached = true;
    return result;
}

void LiveActions::open_settings_app()
{
    switch(get_desktop()) {
    case LiveActions::UNITY8:
        dispatch_url("settings:///system/printers");
        break;
    default:
        execute_command("system-config-printer");
    }
}


} // namespace printers
} // namespace indicator
} // namespace unity
