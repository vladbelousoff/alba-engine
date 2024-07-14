#include "string_manager.h"

std::shared_mutex loki::StringManager::mutex;
std::string loki::StringManager::invalid_string;
std::unordered_map<loki::StringID, std::string> loki::StringManager::id_to_string;
std::unordered_map<std::string, loki::StringID> loki::StringManager::string_to_id;
std::size_t loki::StringManager::string_counter = 0;
loki::StringID loki::StringManager::invalid_id = loki::StringManager::get_id_by_string(loki::StringManager::invalid_string);

loki::StringID::StringID()
    : id{ 0 }
{
}

loki::StringID::StringID(const std::string& string)
    : id{ StringManager::get_id_by_string(string).id }
{
}

bool loki::StringID::operator==(const loki::StringID& other) const
{
  return id == other.id;
}

auto loki::StringID::to_string() const -> const std::string&
{
  return StringManager::get_string_by_id(*this);
}

auto loki::StringManager::get_string_by_id(loki::StringID id) -> const std::string&
{
  std::shared_lock<std::shared_mutex> lock(mutex);
  auto it = id_to_string.find(id);
  if (it != id_to_string.end()) {
    return it->second;
  }

  return invalid_string;
}

auto loki::StringManager::get_id_by_string(const std::string& string) -> loki::StringID
{
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto it = string_to_id.find(string);
    if (it != string_to_id.end()) {
      return it->second;
    }
  }

  StringID id = StringID{};

  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    assert(string_counter < std::numeric_limits<std::size_t>::max());
    id.id = string_counter++;
    string_to_id.insert({ string, id });
    id_to_string.insert({ id, string });
  }

  return id;
}

TEST(StringManager, InvalidStringID)
{
  loki::StringID invalid;
  EXPECT_EQ(invalid, loki::StringManager::invalid_id);
}

TEST(StringManager, Simple)
{
  loki::StringID a = loki::StringID("Hello!");
  loki::StringID b = loki::StringID("Hello!");
  EXPECT_EQ(a, b);
  EXPECT_EQ(a.to_string(), b.to_string());
}

TEST(StringManager, Mulithreading)
{
  constexpr static int count = 10'000;

  auto create_and_compare = []() {
    for (int i = 0; i < count; ++i) {
      loki::StringID(std::to_string(i));
    }
  };

  std::thread thread1(create_and_compare);
  std::thread thread2(create_and_compare);

  thread1.join();
  thread2.join();

  for (int i = 0; i < count; ++i) {
    std::string string = std::to_string(i);
    loki::StringID id = loki::StringID(string);
    EXPECT_EQ(id.to_string(), string);
  }
}

