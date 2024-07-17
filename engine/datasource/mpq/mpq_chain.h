#pragma once

#include <filesystem>
#include <string>

#include "EASTL/vector.h"
#include "mpq_archive.h"

namespace loki {

  class MPQChain
  {
  public:
    explicit MPQChain() = default;
    explicit MPQChain(const std::filesystem::path& data_dir);
    
  private:
    MPQArchive archive{};
  };

} // namespace loki

