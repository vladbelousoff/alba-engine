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

TEST(StringManager, InvalidStringID)
{
  kiwi::StringID invalid;
  EXPECT_EQ(invalid, kiwi::StringManager::invalid_id);
}

TEST(StringManager, Simple)
{
  kiwi::StringID a = kiwi::StringManager::get_id_by_string("Hello!");
  kiwi::StringID b = kiwi::StringManager::get_id_by_string("Hello!");
  EXPECT_EQ(a, b);
  EXPECT_EQ(kiwi::StringManager::get_string_by_id(a), kiwi::StringManager::get_string_by_id(b));
}

TEST(StringManager, Mulithreading)
{
  constexpr static int count = 10'000;

  auto create_and_compare = []() {
    for (int i = 0; i < count; ++i) {
      kiwi::StringManager::get_id_by_string(std::to_string(i));
    }
  };

  std::thread thread1(create_and_compare);
  std::thread thread2(create_and_compare);

  thread1.join();
  thread2.join();

  for (int i = 0; i < count; ++i) {
    std::string string = std::to_string(i);
    kiwi::StringID id = kiwi::StringManager::get_id_by_string(string);
    EXPECT_EQ(kiwi::StringManager::get_string_by_id(id), string);
  }
}
