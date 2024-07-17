#pragma once

#include "mpq_archive.h"

namespace loki {

  class MPQFile
  {
  public:
    explicit MPQFile(MPQArchive& archive)
        : archive{ archive } {};

  private:
    MPQArchive& archive;
  };

} // namespace loki

