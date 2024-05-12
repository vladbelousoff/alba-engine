#include "string_manager.h"

std::shared_mutex kiwi::StringManager::mutex;
std::string kiwi::StringManager::invalid_string;
std::unordered_map<kiwi::StringID, std::string> kiwi::StringManager::id_to_string;
std::unordered_map<std::string, kiwi::StringID> kiwi::StringManager::string_to_id;
std::size_t kiwi::StringManager::string_counter = 0;
kiwi::StringID kiwi::StringManager::invalid_id = kiwi::StringManager::get_id_by_string(kiwi::StringManager::invalid_string);

auto kiwi::StringManager::get_string_by_id(kiwi::StringID id) -> const std::string&
{
  std::shared_lock<std::shared_mutex> lock(mutex);
  auto it = id_to_string.find(id);
  if (it != id_to_string.end()) {
    return it->second;
  }

  return invalid_string;
}

auto kiwi::StringManager::get_id_by_string(const std::string& string) -> kiwi::StringID
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
    id.id = string_counter++;
    string_to_id.insert({ string, id });
    id_to_string.insert({ id, string });
  }

  return id;
}
