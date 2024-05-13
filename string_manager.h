#pragma once

#include <gtest/gtest.h>

#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace kiwi {

  class StringManager;

  class StringID
  {
    friend struct std::hash<StringID>;
    friend StringManager;

  public:
    bool operator==(const StringID& other) const
    {
      return id == other.id;
    }

  private:
    std::size_t id = 0;
  };

  class StringManager
  {
  public:
    static std::string invalid_string;
    static StringID invalid_id;

    static auto get_string_by_id(StringID id) -> const std::string&;
    static auto get_id_by_string(const std::string& string) -> StringID;

  private:
    static std::shared_mutex mutex;
    static std::size_t string_counter;
    static std::unordered_map<StringID, std::string> id_to_string;
    static std::unordered_map<std::string, StringID> string_to_id;
  };

} // namespace kiwi

template <> struct std::hash<kiwi::StringID>
{
  std::size_t operator()(const kiwi::StringID& string) const
  {
    return hash<std::size_t>{}(string.id);
  }
};
