#include <mimalloc-new-delete.h>

#include <CLI/CLI.hpp>
#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "project/project_application.h"

int main(int argc, char* argv[])
{
  CLI::App app{ "loki Engine" };
  argv = app.ensure_utf8(argv);

  std::shared_ptr<loki::EngineSettings> settings = std::make_shared<loki::EngineSettings>();

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
