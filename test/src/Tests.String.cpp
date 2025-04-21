#include "Core.hh"

#include "kstd/String.hh"

using namespace std::string_literals;

TEST(ExtractingTests, extractNameFromPath) {
    ASSERT_EQ(kstd::nameFromPath(""), "");
    ASSERT_EQ(kstd::nameFromPath("a"), "a");
    ASSERT_EQ(kstd::nameFromPath("abc/a"), "a");
    ASSERT_EQ(kstd::nameFromPath("/home/user/file.json"), "file"s);
    ASSERT_EQ(
      kstd::nameFromPath(
        "/home/user/file.json", kstd::NameExtractionMode::withExtension
      ),
      "file.json"s
    );
    ASSERT_EQ(kstd::nameFromPath("/home/user/file"), "file");
    ASSERT_EQ(
      kstd::nameFromPath("/home/user/file", kstd::NameExtractionMode::withExtension),
      "file"s
    );
    ASSERT_EQ(kstd::nameFromPath("file.json"), "file");
    ASSERT_EQ(
      kstd::nameFromPath("file.json", kstd::NameExtractionMode::withExtension),
      "file.json"s
    );
}

TEST(ExtractingTests, extractExtensionFromPath) {
    ASSERT_EQ(
      kstd::extensionFromPath("/home/user/file.json.test").value_or("??"),
      ".json.test"
    );
    ASSERT_EQ(
      kstd::extensionFromPath(
        "/home/user/file.json.test", kstd::ExtensionExtractionMode::lastChunk
      )
        .value_or("??"),
      ".test"s
    );
    ASSERT_FALSE(kstd::extensionFromPath("/home/user/file").has_value());
    ASSERT_FALSE(kstd::extensionFromPath("file").has_value());
}
