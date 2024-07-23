#include "string_manager.h"

std::shared_mutex loki::StringManager::mutex;
std::string loki::StringManager::invalid_string;
std::unordered_map<loki::StringID, std::string> loki::StringManager::id_to_string;
std::unordered_map<std::string, loki::StringID> loki::StringManager::string_to_id;
std::size_t loki::StringManager::string_counter = 0;

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
