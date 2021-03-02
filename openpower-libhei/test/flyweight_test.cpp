#include <util/hei_flyweight.hpp>

#include "gtest/gtest.h"

using namespace libhei;

class Foo
{
  public:
    explicit Foo(int i) : iv_i(i) {}

    bool operator==(const Foo& i_r) const
    {
        return iv_i == i_r.iv_i;
    }

    bool operator<(const Foo& i_r) const
    {
        return iv_i < i_r.iv_i;
    }

  private:
    int iv_i = 0;
};

TEST(FlyweightTest, TestSet1)
{
    auto& foo_factory = Flyweight<Foo>::getSingleton();

    auto f1 = foo_factory.get(1);
    auto f2 = foo_factory.get(2);
    auto f3 = foo_factory.get(1); // same as f1

    ASSERT_NE(f1, f2); // Pointing to different objects
    ASSERT_EQ(f1, f3); // Pointing to the same object

    ASSERT_EQ(size_t(2),
              foo_factory.size()); // Only two entries in the flyweight
}
