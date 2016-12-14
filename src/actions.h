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

#include <memory> // shared_ptr
#include <string>

#include <gio/gio.h> // GSimpleActionGroup

namespace unity {
namespace indicator {
namespace printers {

/**
 * \brief Interface for all the actions that can be activated by users.
 *
 * This is a simple C++ wrapper around our GActionGroup that gets exported
 * onto the bus. Subclasses implement the actual code that should be run
 * when a particular action is triggered.
 */
class Actions
{
public:

    virtual void open_settings_app() = 0;

    GActionGroup* action_group();

protected:
    explicit Actions();
    virtual ~Actions();

private:
    GSimpleActionGroup* m_actions = nullptr;

    // we've got raw pointers in here, so disable copying
    Actions(const Actions&) = delete;
    Actions& operator=(const Actions&) = delete;
};

} // namespace printers
} // namespace indicator
} // namespace unity
