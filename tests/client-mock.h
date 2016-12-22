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

#include "cups-client.h"

#include <gmock/gmock.h>

namespace unity {
namespace indicator {
namespace printers {

    class MockClient: public CupsClient
    {
    public:
        explicit MockClient(): CupsClient() {}
        ~MockClient() = default;

        MOCK_METHOD0(create_subscription, void());
        MOCK_METHOD0(renew_subscription, void());
        MOCK_METHOD0(cancel_subscription, void());
        MOCK_METHOD0(refresh, void());
    };

} // printers
} // indicator
} // unity
