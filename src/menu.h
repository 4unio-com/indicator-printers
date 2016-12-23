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

#pragma once

#include "actions.h"
#include "client.h"

#include <memory> // std::shared_ptr
#include <vector>

#include <gio/gio.h> // GMenuModel

namespace unity {
namespace indicator {
namespace printers {

/**
 * \brief A menu for a specific profile; eg, Desktop or Phone.
 *
 * @see MenuFactory
 * @see Exporter
 */
class Menu
{
public:
    enum Profile {
        Desktop,
        DesktopGreeter,
        Phone,
        PhoneGreeter,
        NUM_PROFILES
    };
    Menu(const std::shared_ptr<Actions>& actions,
         const std::shared_ptr<Client>& client);
    virtual ~Menu() = default;

    GMenuModel* menu_model();

    static std::string profile_name(Profile profile);

private:
    class Impl;
    std::unique_ptr<Impl> p;
};

} // namespace printers
} // namespace indicator
} // namespace unity
