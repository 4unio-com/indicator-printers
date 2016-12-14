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

namespace unity {
namespace indicator {
namespace printers {

/**
 * \brief Production implementation of the Actions interface.
 *
 * Delegates URLs, sets the timezone via org.freedesktop.timedate1, etc.
 *
 * @see MockActions
 */
class LiveActions: public Actions
{
public:
    enum Desktop {
        OTHER = 0,
        UNITY7,
        UNITY8,
    };

    explicit LiveActions();
    virtual ~LiveActions() =default;

    void open_settings_app() override;

protected:
    virtual Desktop get_desktop();
    virtual void execute_command(const std::string& command);
    virtual void dispatch_url(const std::string& url);
};

} // namespace printers
} // namespace indicator
} // namespace unity
