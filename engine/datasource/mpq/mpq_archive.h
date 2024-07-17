#pragma once

#include "StormLib.h"

#include <filesystem>

namespace loki {

  class MPQArchive
  {
  public:
    explicit MPQArchive() = default;
    explicit MPQArchive(const std::filesystem::path& path);

  public:
    auto is_valid() const -> bool
    {
      return handle != HANDLE{};
    }

    auto patch(const std::filesystem::path& path, const std::string& prefix = "") -> bool;

  private:
    HANDLE handle{};
  };

} // namespace loki

