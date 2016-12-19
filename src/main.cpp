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
#include "cups-client.h"
#include "exporter.h"
#include "menu.h"
#include "utils.h"

#include <glib/gi18n.h>
#include <gio/gio.h>

using namespace unity::indicator::printers;

int main() {
    // Work around a deadlock in glib's type initialization.
    // It can be removed when https://bugzilla.gnome.org/show_bug.cgi?id=674885 is fixed.
    g_type_ensure(G_TYPE_DBUS_CONNECTION);

    // boilerplate i18n
    setlocale(LC_ALL, "");

    // Need to prepend $SNAP to properly load translations
    auto localedir = Utilities::prepend_snap_path(GETTEXT_LOCALEDIR);
    bindtextdomain(GETTEXT_PACKAGE, localedir.c_str());
    textdomain(GETTEXT_PACKAGE);

    // set up us the machine
    auto loop = g_main_loop_new(nullptr, false);

    // create the menus
    auto actions = std::make_shared<LiveActions>();
    auto client = std::make_shared<CupsClient>();
    MenuFactory factory(actions, client);
    std::vector<std::shared_ptr<Menu>> menus;
    for(int i = 0, n = Menu::NUM_PROFILES; i < n; i++) {
        menus.push_back(factory.buildMenu(Menu::Profile(i)));
    }

    // export them & run until we lose the busname
    Exporter exporter;
    exporter.name_lost().connect([loop](){
        g_message("%s exiting; failed/lost bus ownership", GETTEXT_PACKAGE);
        g_main_loop_quit(loop);
    });
    exporter.publish(actions, menus);

    g_main_loop_run(loop);

    // cleanup
    g_main_loop_unref(loop);
    return 0;
}
