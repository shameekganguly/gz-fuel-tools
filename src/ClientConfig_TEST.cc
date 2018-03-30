/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <ignition/common/Filesystem.hh>
#include <ignition/common/Util.hh>
#include "ignition/fuel_tools/ClientConfig.hh"
#include "test/test_config.h"

using namespace ignition;
using namespace fuel_tools;

/////////////////////////////////////////////////
/// \brief Get home directory.
/// \return Home directory or empty string if home wasn't found.
/// \ToDo: Move this function to ignition::common::Filesystem
std::string homePath()
{
  std::string homePath;
#ifndef _WIN32
  ignition::common::env("HOME", homePath);
#else
  ignition::common::env("HOMEPATH", homePath);
#endif

  return homePath;
}

/////////////////////////////////////////////////
/// \brief Initially no servers in config
TEST(ClientConfig, InitiallyNoServers)
{
  ClientConfig config;
  EXPECT_EQ(0u, config.Servers().size());
}

/////////////////////////////////////////////////
/// \brief Servers can be added
TEST(ClientConfig, ServersCanBeAdded)
{
  ClientConfig config;
  ServerConfig srv;
  srv.SetUrl("asdf");
  config.AddServer(srv);

  ASSERT_EQ(1u, config.Servers().size());
  EXPECT_EQ(std::string("asdf"), config.Servers().front().Url());
}

/////////////////////////////////////////////////
/// \brief We can load the default configuration file.
TEST(ClientConfig, CustomDefaultConfiguration)
{
  ClientConfig config;
  config.SetConfigPath(TEST_IGNITION_FUEL_INITIAL_CONFIG_PATH);
  config.LoadConfig();

  ASSERT_EQ(1u, config.Servers().size());
  EXPECT_EQ("https://api.ignitionfuel.org",
    config.Servers().front().Url());

  std::string defaultCacheLocation = ignition::common::joinPaths(
    homePath(), ".ignition", "fuel");
  EXPECT_EQ(defaultCacheLocation, config.CacheLocation());
}

/////////////////////////////////////////////////
/// \brief We can load custom settings in a configuration file.
TEST(ClientConfig, CustomConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    name: osrf"                         << std::endl
      << "    url: https://api.ignitionfuel.org"  << std::endl
      << ""                                       << std::endl
      << "  -"                                    << std::endl
      << "    name: another_server"               << std::endl
      << "    url: https://myserver"              << std::endl
      << ""                                       << std::endl
      << "# Where are the assets stored in disk." << std::endl
      << "cache:"                                 << std::endl
      << "  path: /tmp/ignition/fuel"             << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_TRUE(config.LoadConfig());

  ASSERT_EQ(2u, config.Servers().size());
  EXPECT_EQ("https://api.ignitionfuel.org",
    config.Servers().front().Url());
  EXPECT_EQ("https://myserver",
    config.Servers().back().Url());

  EXPECT_EQ("/tmp/ignition/fuel", config.CacheLocation());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief A server contains an already used name.
TEST(ClientConfig, RepeatedServerUrlConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    name: osrf"                         << std::endl
      << "    url: https://api.ignitionfuel.org"  << std::endl
      << ""                                       << std::endl
      << "  -"                                    << std::endl
      << "    name: osrf"                         << std::endl
      << "    url: https://myserver"              << std::endl
      << ""                                       << std::endl
      << "# Where are the assets stored in disk." << std::endl
      << "cache:"                                 << std::endl
      << "  path: /tmp/ignition/fuel"             << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief A server contains an already used URL.
TEST(ClientConfig, RepeatedServerConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    name: osrf"                         << std::endl
      << "    url: https://api.ignitionfuel.org"  << std::endl
      << ""                                       << std::endl
      << "  -"                                    << std::endl
      << "    name: another_server"               << std::endl
      << "    url: https://api.ignitionfuel.org"  << std::endl
      << ""                                       << std::endl
      << "# Where are the assets stored in disk." << std::endl
      << "cache:"                                 << std::endl
      << "  path: /tmp/ignition/fuel"             << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief A server without name is not valid.
TEST(ClientConfig, NoServerNameConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    url: https://api.ignitionfuel.org"  << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief A server with an empty name is not valid.
TEST(ClientConfig, EmptyServerNameConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    name: "                             << std::endl
      << "    url: https://api.ignitionfuel.org"  << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief A server without URL is not valid.
TEST(ClientConfig, NoServerUrlConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    name: osrf"  << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief A server with an empty URL is not valid.
TEST(ClientConfig, EmptyServerUrlConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"                                    << std::endl
      << "# The list of servers."                 << std::endl
      << "servers:"                               << std::endl
      << "  -"                                    << std::endl
      << "    name: osrf"                         << std::endl
      << "    url: "                              << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief The "cache" option requires to set "path".
TEST(ClientConfig, NoCachePathConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"    << std::endl
      << "cache:" << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
/// \brief The path parameter cannot be empty.
TEST(ClientConfig, EmptyCachePathConfiguration)
{
  ClientConfig config;

  // Create a temporary file with the configuration.
  std::ofstream ofs;
  std::string testPath = "test_conf.yaml";
  ofs.open(testPath, std::ofstream::out | std::ofstream::app);

  ofs << "---"     << std::endl
      << "cache:"  << std::endl
      << "  path:" << std::endl
      << std::endl;

  config.SetConfigPath(testPath);
  EXPECT_FALSE(config.LoadConfig());

  // Remove the configuration file.
  EXPECT_TRUE(ignition::common::removeFile(testPath));
}

/////////////////////////////////////////////////
TEST(ClientConfig, UserAgent)
{
  ClientConfig config;
  EXPECT_EQ("IgnitionFuelTools-" IGNITION_FUEL_TOOLS_VERSION_FULL,
            config.UserAgent());

  config.SetUserAgent("my_user_agent");
  EXPECT_EQ("my_user_agent", config.UserAgent());
}

/////////////////////////////////////////////////
TEST(ServerConfig, APIKey)
{
  ServerConfig config;
  EXPECT_TRUE(config.APIKey().empty());

  config.APIKey("my_api_key");
  EXPECT_EQ("my_api_key", config.APIKey());

  config.APIKey("my_other_api_key");
  EXPECT_EQ("my_other_api_key", config.APIKey());
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
