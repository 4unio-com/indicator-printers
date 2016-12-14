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
#include "menu.h"

#include <core/signal.h>

#include <memory> // std::shared_ptr
#include <vector>

namespace unity {
namespace indicator {
namespace printers {

#define INDICATOR_BUS_NAME "com.canonical.indicator.printers"
#define INDICATOR_BUS_PATH "/com/canonical/indicator/printers"

/**
 * \brief Exports actions and menus to DBus. 
 */
class Exporter
{
public:
    explicit Exporter();
    ~Exporter();

    core::Signal<>& name_lost();

    void publish(const std::shared_ptr<Actions>& actions,
                 const std::vector<std::shared_ptr<Menu>>& menus);

private:
    class Impl;
    std::unique_ptr<Impl> p;

    // disable copying 
    Exporter(const Exporter&) = delete; 
    Exporter& operator=(const Exporter&) = delete; 
};

} // namespace printers
} // namespace indicator
} // namespace unity
