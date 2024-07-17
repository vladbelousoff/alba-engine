#include "mpq_archive.h"

#include "spdlog/spdlog.h"

loki::MPQArchive::MPQArchive(const std::filesystem::path& path)
{
  if (!SFileOpenArchive(path.string().c_str(), 0, 0x00000100, &handle)) {
    spdlog::error("Error opening MPQ archive: {}", path.string());
  }
}

bool loki::MPQArchive::patch(const std::filesystem::path& path, const std::string& prefix)
{
  return SFileOpenPatchArchive(handle, path.string().c_str(), prefix.c_str(), 0);
}
