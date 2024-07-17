#include "mpq_chain.h"

#include "spdlog/spdlog.h"

loki::MPQChain::MPQChain(const std::filesystem::path& data_dir)
{
  namespace fs = std::filesystem;

  for (const auto& entry : fs::recursive_directory_iterator(data_dir)) {
    if (!fs::is_regular_file(entry)) {
      continue;
    }

    if (entry.path().extension() != ".MPQ") {
      continue;
    }

    spdlog::info("Found a MPQ file: {}", entry.path().string());

    if (archive.is_valid()) {
      if (!archive.patch(entry.path())) {
        spdlog::error("Error patching: {}", entry.path().string());
      }
    } else {
      archive = MPQArchive{ entry.path() };
    }
  }
}
