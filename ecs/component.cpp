#include "component.h"

TEST(Component, Basic)
{
  struct ComponentInt : loki::ComponentWrapper<ComponentInt>
  {
    int value{};
  };

  struct ComponentStringID : loki::ComponentWrapper<ComponentStringID>
  {
    loki::StringID value{};
  };

  auto a = ComponentInt();
  auto b = ComponentStringID();

  EXPECT_EQ(a.value, 0);
  EXPECT_EQ(b.value, loki::StringManager::invalid_id);

  EXPECT_EQ(ComponentInt::get_id(), loki::StringID{ typeid(ComponentInt).name() });
  EXPECT_EQ(ComponentStringID::get_id(), loki::StringID{ typeid(ComponentStringID).name() });
}
