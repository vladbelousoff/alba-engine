#pragma once

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace loki {

  class StringManager;

  class StringID
  {
    friend struct std::hash<StringID>;
    friend class StringManager;

  public:
    explicit StringID();
    explicit StringID(const std::string& string);

    auto to_string() const -> const std::string&;
    bool operator==(const StringID& other) const;

  private:
    std::size_t id;
  };

  class StringManager
  {
    friend class StringID;

  public:
    static std::string invalid_string;
    static StringID invalid_id;

  private:
    static auto get_string_by_id(StringID id) -> const std::string&;
    static auto get_id_by_string(const std::string& string) -> StringID;

    static std::shared_mutex mutex;
    static std::size_t string_counter;
    static std::unordered_map<StringID, std::string> id_to_string;
    static std::unordered_map<std::string, StringID> string_to_id;
  };

} // namespace loki

template <> struct std::hash<loki::StringID>
{
  std::size_t operator()(const loki::StringID& string) const
  {
    return hash<std::size_t>{}(string.id);
  }
};

template <> struct fmt::formatter<loki::StringID> : formatter<std::string>
{
  auto format(loki::StringID type, format_context& ctx)
  {
    return formatter<std::string>::format(type.to_string(), ctx);
  }
};
