
#include "kstd/Core.hh"
#include "kstd/Log.hh"

#include <kstd/serialization/BinaryReader.hh>
#include <kstd/serialization/BinaryWriter.hh>
#include <kstd/FileSystem.hh>

int main() {
    kstd::log::init("serialization");

    kstd::u8 x  = 0xA;
    kstd::u16 y = 0xB;
    kstd::u32 z = 0xC;

    kstd::BinaryWriter writer;
    writer.write(x).write(y).write(z);
    writer.write("Hello world");

    kstd::writeBinaryFile("./dump", writer.getBuffer());
    kstd::BinaryReader reader{ kstd::readBinaryFile("./dump") };

    kstd::u8 nx;
    reader.read(nx);
    auto ny  = reader.read<kstd::u16>();
    auto nz  = reader.read<kstd::u32>();
    auto str = reader.read<std::string>();

    kstd::log::info("{} - {} - {} - {}", nx, ny, nz, str);

    return 0;
}
