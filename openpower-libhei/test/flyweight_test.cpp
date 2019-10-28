#include <util/hei_flyweight.hpp>

#include "gtest/gtest.h"

using namespace libhei;

class Foo
{
  public:
    Foo() = default;
    explicit Foo(int i) : iv_i(i) {}
    int get() const { return iv_i; }
    bool operator==(const Foo& i_r) const { return iv_i == i_r.iv_i; }
    bool operator<(const Foo& i_r) const { return iv_i <  i_r.iv_i; }
  private:
    int iv_i = 0;
};

Foo& addFoo(int i)
{
    return Flyweight<Foo>::getSingleton().get(Foo { i });
}

TEST(FlyweightTest, TestSet1)
{
    // Add some unique entries in a random order and keep track of where those
    // enties exist in memory.
    Foo* a[5];
    a[1] = &(addFoo(1));
    a[2] = &(addFoo(2));
    a[0] = &(addFoo(0));
    a[4] = &(addFoo(4));
    a[3] = &(addFoo(3));

    // Now add more entries and verify the 'new' entries match the same
    // addresses as the previously added entries.
    for (int i = 4; i >= 0; i--)
    {
        ASSERT_EQ(a[i], &(addFoo(i)));
    }

    // At this point, we have proven that duplicate entries will return
    // references to the original unique entries. There is probably more we can
    // do here, but this is enough to prove the Flyweight class follows the
    // flyweight design pattern.
}
