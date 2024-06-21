#include <CLI/CLI.hpp>
#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "project/project_application.h"

int main(int argc, char* argv[])
{
  CLI::App app{ "Alba Engine" };
  argv = app.ensure_utf8(argv);

  std::shared_ptr<alba::EngineSettings> settings = std::make_shared<alba::EngineSettings>();

  CLI::Option* testing_flag = app.add_flag("--testing");
  app.add_option("--root", settings->root_path);
  CLI11_PARSE(app, argc, argv)

  spdlog::info("Root: {}", absolute(settings->root_path).string());

  if (testing_flag->count()) {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
  }

  return ProjectApplication{}.launch(settings);
}
