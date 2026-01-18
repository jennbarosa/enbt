#include "acutest.h"
#include "NBTReader.h"
#include "NBTWriter.h"
#include <filesystem>
#include <cstdio>

// Cross-platform temp directory helper
static std::string get_temp_path(const char* filename) {
    std::filesystem::path temp = std::filesystem::temp_directory_path();
    return (temp / filename).string();
}

// Test reading primitive types
void test_read_primitives(void) {
    std::string testfile = get_temp_path("test_primitives.dat");

    // Write test data
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeByte("testByte", 42);
        writer.writeShort("testShort", 1234);
        writer.writeInt("testInt", 123456);
        writer.writeLong("testLong", 9876543210LL);
        writer.writeFloat("testFloat", 3.14159f);
        writer.writeDouble("testDouble", 2.71828);
        writer.writeString("testString", "Hello NBT");
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());

        char byteVal = reader.readByte("testByte");
        TEST_CHECK(byteVal == 42);

        short shortVal = reader.readShort("testShort");
        TEST_CHECK(shortVal == 1234);

        int intVal = reader.readInt("testInt");
        TEST_CHECK(intVal == 123456);

        long long longVal = reader.readLong("testLong");
        TEST_CHECK(longVal == 9876543210LL);

        float floatVal = reader.readFloat("testFloat");
        TEST_CHECK(floatVal > 3.14f && floatVal < 3.15f);

        double doubleVal = reader.readDouble("testDouble");
        TEST_CHECK(doubleVal > 2.71f && doubleVal < 2.72f);

        std::string strVal = reader.readString("testString");
        TEST_CHECK(strVal == "Hello NBT");

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test reading compound tags
void test_read_compound(void) {
    std::string testfile = get_temp_path("test_compound.dat");

    // Write nested compound
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeCompound("outer");
        writer.writeString("outerString", "outer value");
        writer.writeCompound("inner");
        writer.writeString("innerString", "inner value");
        writer.writeInt("innerInt", 999);
        writer.endCompound();
        writer.writeInt("outerInt", 888);
        writer.endCompound();
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        reader.enterCompound("outer");

        std::string outerStr = reader.readString("outerString");
        TEST_CHECK(outerStr == "outer value");

        reader.enterCompound("inner");
        std::string innerStr = reader.readString("innerString");
        TEST_CHECK(innerStr == "inner value");

        int innerInt = reader.readInt("innerInt");
        TEST_CHECK(innerInt == 999);
        reader.exitCompound();

        int outerInt = reader.readInt("outerInt");
        TEST_CHECK(outerInt == 888);

        reader.exitCompound();
        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test reading list tags
void test_read_list(void) {
    std::string testfile = get_temp_path("test_list.dat");

    // Write list of integers
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeListHead("intList", NBT::idInt, 5);
        writer.writeInt("", 10);
        writer.writeInt("", 20);
        writer.writeInt("", 30);
        writer.writeInt("", 40);
        writer.writeInt("", 50);
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        char elementType;
        int count;
        reader.readListHead("intList", &elementType, &count);

        TEST_CHECK(elementType == NBT::idInt);
        TEST_CHECK(count == 5);

        for (int i = 0; i < count; i++) {
            int val = reader.readInt(nullptr);
            TEST_CHECK(val == (i + 1) * 10);
        }

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test reading list of strings
void test_read_string_list(void) {
    std::string testfile = get_temp_path("test_string_list.dat");

    // Write list of strings
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeListHead("stringList", NBT::idString, 3);
        writer.writeString("", "first");
        writer.writeString("", "second");
        writer.writeString("", "third");
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        char elementType;
        int count;
        reader.readListHead("stringList", &elementType, &count);

        TEST_CHECK(elementType == NBT::idString);
        TEST_CHECK(count == 3);

        std::string s1 = reader.readString(nullptr);
        TEST_CHECK(s1 == "first");

        std::string s2 = reader.readString(nullptr);
        TEST_CHECK(s2 == "second");

        std::string s3 = reader.readString(nullptr);
        TEST_CHECK(s3 == "third");

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test reading list of compounds (like servers.dat)
void test_read_compound_list(void) {
    std::string testfile = get_temp_path("test_compound_list.dat");

    // Write list of compounds
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeListHead("items", NBT::idCompound, 2);

        writer.writeCompound("");
        writer.writeString("name", "Item1");
        writer.writeInt("value", 100);
        writer.endCompound();

        writer.writeCompound("");
        writer.writeString("name", "Item2");
        writer.writeInt("value", 200);
        writer.endCompound();

        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        char elementType;
        int count;
        reader.readListHead("items", &elementType, &count);

        TEST_CHECK(elementType == NBT::idCompound);
        TEST_CHECK(count == 2);

        // First compound
        reader.enterCompound();
        std::string name1 = reader.readString("name");
        TEST_CHECK(name1 == "Item1");
        int val1 = reader.readInt("value");
        TEST_CHECK(val1 == 100);
        reader.exitCompound();

        // Second compound
        reader.enterCompound();
        std::string name2 = reader.readString("name");
        TEST_CHECK(name2 == "Item2");
        int val2 = reader.readInt("value");
        TEST_CHECK(val2 == 200);
        reader.exitCompound();

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test reading servers.dat structure
void test_read_servers_dat(void) {
    std::string testfile = get_temp_path("test_servers.dat");

    // Write servers.dat structure
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeListHead("servers", NBT::idCompound, 3);

        writer.writeCompound("");
        writer.writeString("name", "Server1");
        writer.writeString("icon", "icon1data");
        writer.writeString("ip", "192.168.1.1");
        writer.writeByte("acceptTextures", 1);
        writer.endCompound();

        writer.writeCompound("");
        writer.writeString("name", "Server2");
        writer.writeString("icon", "icon2data");
        writer.writeString("ip", "192.168.1.2");
        writer.writeByte("acceptTextures", 0);
        writer.endCompound();

        writer.writeCompound("");
        writer.writeString("name", "Server3");
        writer.writeString("icon", "icon3data");
        writer.writeString("ip", "192.168.1.3");
        writer.writeByte("acceptTextures", 1);
        writer.endCompound();

        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        char elementType;
        int serverCount;
        reader.readListHead("servers", &elementType, &serverCount);

        TEST_CHECK(elementType == NBT::idCompound);
        TEST_CHECK(serverCount == 3);

        // Server 1
        reader.enterCompound();
        TEST_CHECK(reader.readString("name") == "Server1");
        TEST_CHECK(reader.readString("icon") == "icon1data");
        TEST_CHECK(reader.readString("ip") == "192.168.1.1");
        TEST_CHECK(reader.readByte("acceptTextures") == 1);
        reader.exitCompound();

        // Server 2
        reader.enterCompound();
        TEST_CHECK(reader.readString("name") == "Server2");
        TEST_CHECK(reader.readString("icon") == "icon2data");
        TEST_CHECK(reader.readString("ip") == "192.168.1.2");
        TEST_CHECK(reader.readByte("acceptTextures") == 0);
        reader.exitCompound();

        // Server 3
        reader.enterCompound();
        TEST_CHECK(reader.readString("name") == "Server3");
        TEST_CHECK(reader.readString("icon") == "icon3data");
        TEST_CHECK(reader.readString("ip") == "192.168.1.3");
        TEST_CHECK(reader.readByte("acceptTextures") == 1);
        reader.exitCompound();

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test empty list
void test_read_empty_list(void) {
    std::string testfile = get_temp_path("test_empty_list.dat");

    // Write empty list
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeListHead("emptyList", NBT::idInt, 0);
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        char elementType;
        int count;
        reader.readListHead("emptyList", &elementType, &count);

        TEST_CHECK(elementType == NBT::idInt);
        TEST_CHECK(count == 0);

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test peek functionality
void test_peek_tag_type(void) {
    std::string testfile = get_temp_path("test_peek.dat");

    // Write test data
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeInt("testInt", 42);
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());

        // Peek should not consume the tag
        char type1 = reader.peekTagType();
        TEST_CHECK(type1 == NBT::idInt);

        // Peek again should return same type
        char type2 = reader.peekTagType();
        TEST_CHECK(type2 == NBT::idInt);

        // Now actually read it
        int val = reader.readInt("testInt");
        TEST_CHECK(val == 42);

        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test error handling - wrong tag name
void test_error_wrong_name(void) {
    std::string testfile = get_temp_path("test_wrong_name.dat");

    // Write test data
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeString("correctName", "value");
        writer.endCompound();
        writer.close();
    }

    // Try to read with wrong name
    {
        NBT::NBTReader reader(testfile.c_str());

        bool caught = false;
        try {
            reader.readString("wrongName");
        } catch (const std::runtime_error& e) {
            caught = true;
            TEST_CHECK(std::string(e.what()).find("Expected tag name") != std::string::npos);
        }

        TEST_CHECK(caught);
        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test error handling - wrong tag type
void test_error_wrong_type(void) {
    std::string testfile = get_temp_path("test_wrong_type.dat");

    // Write test data
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeString("testTag", "value");
        writer.endCompound();
        writer.close();
    }

    // Try to read as wrong type
    {
        NBT::NBTReader reader(testfile.c_str());

        bool caught = false;
        try {
            reader.readInt("testTag");
        } catch (const std::runtime_error& e) {
            caught = true;
        }

        TEST_CHECK(caught);
        reader.close();
    }

    std::remove(testfile.c_str());
}

// Test array reading
void test_read_byte_array(void) {
    std::string testfile = get_temp_path("test_byte_array.dat");

    // Write byte array
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeByteArrayHead("bytes", 5);
        writer.writeByte("", 1);
        writer.writeByte("", 2);
        writer.writeByte("", 3);
        writer.writeByte("", 4);
        writer.writeByte("", 5);
        writer.endCompound();
        writer.close();
    }

    // Read it back
    {
        NBT::NBTReader reader(testfile.c_str());
        int count = reader.readByteArrayHead("bytes");
        TEST_CHECK(count == 5);

        for (int i = 0; i < count; i++) {
            char val = reader.readByte(nullptr);
            TEST_CHECK(val == i + 1);
        }

        reader.close();
    }

    std::remove(testfile.c_str());
}

TEST_LIST = {
    { "Read primitives", test_read_primitives },
    { "Read compound", test_read_compound },
    { "Read list", test_read_list },
    { "Read string list", test_read_string_list },
    { "Read compound list", test_read_compound_list },
    { "Read servers.dat", test_read_servers_dat },
    { "Read empty list", test_read_empty_list },
    { "Peek tag type", test_peek_tag_type },
    { "Error wrong name", test_error_wrong_name },
    { "Error wrong type", test_error_wrong_type },
    { "Read byte array", test_read_byte_array },
    { NULL, NULL }
};
