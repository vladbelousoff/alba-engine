#include "component.h"

TEST(Component, Basic)
{
  struct ComponentInt : alba::ComponentWrapper<ComponentInt>
  {
    int value{};
  };

  struct ComponentStringID : alba::ComponentWrapper<ComponentStringID>
  {
    alba::StringID value{};
  };

  auto a = ComponentInt();
  auto b = ComponentStringID();

  EXPECT_EQ(a.value, 0);
  EXPECT_EQ(b.value, alba::StringManager::invalid_id);

  EXPECT_EQ(ComponentInt::get_id(), alba::StringID{ typeid(ComponentInt).name() });
  EXPECT_EQ(ComponentStringID::get_id(), alba::StringID{ typeid(ComponentStringID).name() });
}
