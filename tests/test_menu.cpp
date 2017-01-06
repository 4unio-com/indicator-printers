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

#include "actions-mock.h"
#include "client-mock.h"
#include "glib-fixture.h"

#include "menu.h"

#include <gmock/gmock.h>

using namespace unity::indicator::printers;
using namespace ::testing;

class MenuFixture: public GlibFixture
{
private:

    typedef GlibFixture super;

protected:

    GTestDBus* bus = nullptr;

    void SetUp() override
    {
        super::SetUp();

        // bring up the test bus
        bus = g_test_dbus_new(G_TEST_DBUS_NONE);
        g_test_dbus_up(bus);
        const auto address = g_test_dbus_get_bus_address(bus);
        g_setenv("DBUS_SYSTEM_BUS_ADDRESS", address, true);
        g_setenv("DBUS_SESSION_BUS_ADDRESS", address, true);
    }

    void TearDown() override
    {
        GError * error = nullptr;
        GDBusConnection* connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
        if(!g_dbus_connection_is_closed(connection)) {
            g_dbus_connection_close_sync(connection, nullptr, &error);
        }
        g_assert_no_error(error);
        g_clear_object(&connection);
        g_test_dbus_down(bus);
        g_clear_object(&bus);

        super::TearDown();
    }
};


TEST_F(MenuFixture, Menu)
{
    auto actions = std::make_shared<MockActions>();
    auto client = std::make_shared<MockClient>();

    // Test for initialization
    auto menu = std::make_shared<Menu>(actions, client);
    ASSERT_FALSE(nullptr == menu);

    // Test refresh
    EXPECT_CALL(*client, refresh()).Times(1)
        .WillOnce(Invoke([&client](){
                    // Create a fake job, defaults to PENDING
                    Job fake_job;
                    fake_job.id = 42;
                    fake_job.name = "Life, The Universe, and Everything";
                    fake_job.printer.description = "Deep Thought";
                    fake_job.printer.name = "deep-thought";
                    client->m_job_state_changed(fake_job);

                    // Test switching to HELD
                    fake_job.state = Job::State::HELD;
                    client->m_job_state_changed(fake_job);

                    // Test switching to PROCESSING
                    fake_job.state = Job::State::PROCESSING;
                    client->m_job_state_changed(fake_job);

                    // Test switching to COMPLETED
                    fake_job.state = Job::State::COMPLETED;
                    client->m_job_state_changed(fake_job);
                }));
    client->refresh();
}
