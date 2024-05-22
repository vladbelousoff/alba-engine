#include "component.h"

TEST(Component, Basic)
{
  struct ComponentInt : kiwi::ComponentWrapper<ComponentInt>
  {
    int value{};
  };

  struct ComponentStringID : kiwi::ComponentWrapper<ComponentStringID>
  {
    kiwi::StringID value{};
  };

  auto a = ComponentInt();
  auto b = ComponentStringID();

  EXPECT_EQ(a.value, 0);
  EXPECT_EQ(b.value, kiwi::StringManager::invalid_id);

  EXPECT_EQ(ComponentInt::get_id(), kiwi::StringID{ typeid(ComponentInt).name() });
  EXPECT_EQ(ComponentStringID::get_id(), kiwi::StringID{ typeid(ComponentStringID).name() });
}
