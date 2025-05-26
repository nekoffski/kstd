#include "Core.hh"

#include "kstd/serialization/All.hh"

using namespace kstd;

namespace kstd {

void serialize(BinaryWriter& bw, const Foo& foo) {
    bw.write(foo.x).write(foo.y).write(foo.z).write(foo.w);
}

void deserialize(BinaryReader& br, Foo& foo) {
    br.read(foo.x).read(foo.y).read(foo.z).read(foo.w);
}

}  // namespace kstd

TEST(SerializationTests, customType) {
    Foo foo{ 1, 2, 3, "hello" };
    BinaryWriter bw;
    bw.write(foo);

    auto foo2 = BinaryReader{ bw.getBuffer() }.read<Foo>();
    ASSERT_EQ(foo, foo2);
}

TEST(SerializationTests, readOverBuffer) {
    BinaryReader br{ BinaryBuffer{} };
    ASSERT_ANY_THROW(br.read<u8>());
}

TEST(SerializationTests, basic) {
    BinaryWriter bw;

    bool a        = true;
    u8 b          = 0xA;
    u16 c         = 0xAABBu;
    u32 d         = 0xABCDABCDu;
    u64 e         = 0xAABBCCDDAABBCCDDu;
    i8 f          = b;
    i16 g         = c;
    i32 h         = d;
    i64 i         = e;
    f32 j         = 1337.0f;
    f64 k         = 242156.0;
    std::string l = "hello world";

    bw.write(a)
      .write(b)
      .write(c)
      .write(d)
      .write(e)
      .write(f)
      .write(g)
      .write(h)
      .write(i)
      .write(j)
      .write(k)
      .write(l);

    BinaryReader br{ bw.getBuffer() };

    ASSERT_EQ(a, br.read<bool>());
    ASSERT_EQ(b, br.read<u8>());
    ASSERT_EQ(c, br.read<u16>());
    ASSERT_EQ(d, br.read<u32>());
    ASSERT_EQ(e, br.read<u64>());
    ASSERT_EQ(f, br.read<i8>());
    ASSERT_EQ(g, br.read<i16>());
    ASSERT_EQ(h, br.read<i32>());
    ASSERT_EQ(i, br.read<i64>());
    ASSERT_EQ(j, br.read<f32>());
    ASSERT_EQ(k, br.read<f64>());
    ASSERT_EQ(l, br.read<std::string>());
}
