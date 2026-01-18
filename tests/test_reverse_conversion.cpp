#include "acutest.h"
#include "NBTReader.h"
#include "NBTWriter.h"
#include "parse.hpp"
#include "serialize.hpp"
#include <filesystem>
#include <cstdio>
#include <fstream>

// Cross-platform temp directory helper
static std::string get_temp_path(const char* filename) {
    std::filesystem::path temp = std::filesystem::temp_directory_path();
    return (temp / filename).string();
}

// Test complete CSV -> DAT -> CSV roundtrip
void test_full_csv_roundtrip(void) {
    std::string csv_input = get_temp_path("test_input.csv");
    std::string dat_file = get_temp_path("test.dat");
    std::string csv_output = get_temp_path("test_output.csv");

    // Create CSV input
    {
        std::ofstream out(csv_input);
        out << "Server1,icon1data,192.168.1.1,1\n";
        out << "Server2,icon2data,192.168.1.2,0\n";
        out << "Server3,icon3data,192.168.1.3,1\n";
        out.close();
    }

    // Parse CSV
    std::ifstream in_file(csv_input);
    std::stringstream buffer;
    buffer << in_file.rdbuf();
    in_file.close();
    std::vector<nbtserver> servers1 = parse_servers_csv(buffer.str());

    TEST_CHECK(servers1.size() == 3);

    // Write to DAT
    {
        NBT::NBTWriter writer(dat_file.c_str());
        writer.writeListHead("servers", NBT::idCompound, servers1.size());
        for (const auto& server : servers1) {
            writer.writeCompound("");
            writer.writeString("name", server.name.data());
            writer.writeString("icon", server.icon.data());
            writer.writeString("ip", server.ip.data());
            writer.writeByte("acceptTextures", server.accept_textures);
            writer.endCompound();
        }
        writer.endCompound();
        writer.close();
    }

    // Read back from DAT
    std::vector<nbtserver> servers2 = parse_servers_dat(dat_file);
    TEST_CHECK(servers2.size() == 3);

    // Serialize to CSV
    std::string csv_content = serialize_servers_csv(servers2);

    // Write CSV output
    {
        std::ofstream out(csv_output);
        out << csv_content;
        out.close();
    }

    // Compare input and output
    std::ifstream in1(csv_input);
    std::ifstream in2(csv_output);
    std::string line1, line2;

    int lines_compared = 0;
    while (std::getline(in1, line1) && std::getline(in2, line2)) {
        TEST_CHECK(line1 == line2);
        lines_compared++;
    }
    TEST_CHECK(lines_compared == 3);

    in1.close();
    in2.close();

    // Cleanup
    std::remove(csv_input.c_str());
    std::remove(dat_file.c_str());
    std::remove(csv_output.c_str());
}

// Test complete JSON -> DAT -> JSON roundtrip
void test_full_json_roundtrip(void) {
    std::string json_input = get_temp_path("test_input.json");
    std::string dat_file = get_temp_path("test.dat");

    // Create JSON input
    {
        std::ofstream out(json_input);
        out << "{\n";
        out << "  \"servers\": [\n";
        out << "    {\"icon\": \"iconA\", \"ip\": \"10.0.0.1\", \"name\": \"ServerA\", \"accept_textures\": true},\n";
        out << "    {\"icon\": \"iconB\", \"ip\": \"10.0.0.2\", \"name\": \"ServerB\", \"accept_textures\": false}\n";
        out << "  ]\n";
        out << "}\n";
        out.close();
    }

    // Parse JSON
    std::ifstream in_file(json_input);
    std::stringstream buffer;
    buffer << in_file.rdbuf();
    in_file.close();
    std::vector<nbtserver> servers1 = parse_servers_json(buffer.str());

    TEST_CHECK(servers1.size() == 2);

    // Write to DAT
    {
        NBT::NBTWriter writer(dat_file.c_str());
        writer.writeListHead("servers", NBT::idCompound, servers1.size());
        for (const auto& server : servers1) {
            writer.writeCompound("");
            writer.writeString("name", server.name.data());
            writer.writeString("icon", server.icon.data());
            writer.writeString("ip", server.ip.data());
            writer.writeByte("acceptTextures", server.accept_textures);
            writer.endCompound();
        }
        writer.endCompound();
        writer.close();
    }

    // Read back from DAT
    std::vector<nbtserver> servers2 = parse_servers_dat(dat_file);
    TEST_CHECK(servers2.size() == 2);

    // Verify data integrity
    TEST_CHECK(servers2[0].name == "ServerA");
    TEST_CHECK(servers2[0].icon == "iconA");
    TEST_CHECK(servers2[0].ip == "10.0.0.1");
    TEST_CHECK(servers2[0].accept_textures == true);

    TEST_CHECK(servers2[1].name == "ServerB");
    TEST_CHECK(servers2[1].icon == "iconB");
    TEST_CHECK(servers2[1].ip == "10.0.0.2");
    TEST_CHECK(servers2[1].accept_textures == false);

    // Cleanup
    std::remove(json_input.c_str());
    std::remove(dat_file.c_str());
}

// Test complete TOML -> DAT -> TOML roundtrip
void test_full_toml_roundtrip(void) {
    std::string toml_input = get_temp_path("test_input.toml");
    std::string dat_file = get_temp_path("test.dat");

    // Create TOML input
    {
        std::ofstream out(toml_input);
        out << "[[servers]]\n";
        out << "icon = \"icon1\"\n";
        out << "ip = \"1.1.1.1\"\n";
        out << "name = \"Server1\"\n";
        out << "accept_textures = true\n";
        out << "\n";
        out << "[[servers]]\n";
        out << "icon = \"icon2\"\n";
        out << "ip = \"2.2.2.2\"\n";
        out << "name = \"Server2\"\n";
        out << "accept_textures = false\n";
        out.close();
    }

    // Parse TOML
    std::ifstream in_file(toml_input);
    std::stringstream buffer;
    buffer << in_file.rdbuf();
    in_file.close();
    std::vector<nbtserver> servers1 = parse_servers_toml(buffer.str());

    TEST_CHECK(servers1.size() == 2);

    // Write to DAT
    {
        NBT::NBTWriter writer(dat_file.c_str());
        writer.writeListHead("servers", NBT::idCompound, servers1.size());
        for (const auto& server : servers1) {
            writer.writeCompound("");
            writer.writeString("name", server.name.data());
            writer.writeString("icon", server.icon.data());
            writer.writeString("ip", server.ip.data());
            writer.writeByte("acceptTextures", server.accept_textures);
            writer.endCompound();
        }
        writer.endCompound();
        writer.close();
    }

    // Read back from DAT
    std::vector<nbtserver> servers2 = parse_servers_dat(dat_file);
    TEST_CHECK(servers2.size() == 2);

    // Verify data integrity
    TEST_CHECK(servers2[0].name == "Server1");
    TEST_CHECK(servers2[0].icon == "icon1");
    TEST_CHECK(servers2[0].ip == "1.1.1.1");
    TEST_CHECK(servers2[0].accept_textures == true);

    TEST_CHECK(servers2[1].name == "Server2");
    TEST_CHECK(servers2[1].accept_textures == false);

    // Cleanup
    std::remove(toml_input.c_str());
    std::remove(dat_file.c_str());
}

// Test parse_servers_dat with invalid file
void test_parse_dat_invalid_file(void) {
    std::vector<nbtserver> servers = parse_servers_dat("/nonexistent/file.dat");
    TEST_CHECK(servers.empty());
}

// Test parse_servers_dat with empty path
void test_parse_dat_empty_path(void) {
    std::vector<nbtserver> servers = parse_servers_dat("");
    TEST_CHECK(servers.empty());
}

// Test parse_servers_dat with malformed structure
void test_parse_dat_malformed(void) {
    std::string testfile = get_temp_path("test_malformed.dat");

    // Write NBT without servers list
    {
        NBT::NBTWriter writer(testfile.c_str());
        writer.writeString("notServers", "value");
        writer.endCompound();
        writer.close();
    }

    std::vector<nbtserver> servers = parse_servers_dat(testfile);
    // Should fail gracefully and return empty vector
    TEST_CHECK(servers.empty());

    std::remove(testfile.c_str());
}

// Test with large number of servers
void test_large_server_list(void) {
    std::string dat_file = get_temp_path("test_large.dat");
    const int SERVER_COUNT = 100;

    // Create large server list
    std::vector<nbtserver> original_servers;
    for (int i = 0; i < SERVER_COUNT; i++) {
        nbtserver server;
        server.name = "Server" + std::to_string(i);
        server.icon = "icon" + std::to_string(i);
        server.ip = "192.168.1." + std::to_string(i % 256);
        server.accept_textures = (i % 2 == 0);
        original_servers.push_back(server);
    }

    // Write to DAT
    {
        NBT::NBTWriter writer(dat_file.c_str());
        writer.writeListHead("servers", NBT::idCompound, original_servers.size());
        for (const auto& server : original_servers) {
            writer.writeCompound("");
            writer.writeString("name", server.name.data());
            writer.writeString("icon", server.icon.data());
            writer.writeString("ip", server.ip.data());
            writer.writeByte("acceptTextures", server.accept_textures);
            writer.endCompound();
        }
        writer.endCompound();
        writer.close();
    }

    // Read back
    std::vector<nbtserver> read_servers = parse_servers_dat(dat_file);
    TEST_CHECK(read_servers.size() == SERVER_COUNT);

    // Spot check a few servers
    TEST_CHECK(read_servers[0].name == "Server0");
    TEST_CHECK(read_servers[50].name == "Server50");
    TEST_CHECK(read_servers[99].name == "Server99");
    TEST_CHECK(read_servers[0].accept_textures == true);
    TEST_CHECK(read_servers[1].accept_textures == false);

    std::remove(dat_file.c_str());
}

// Test with empty server list
void test_empty_server_list(void) {
    std::string dat_file = get_temp_path("test_empty.dat");

    // Write empty server list
    {
        NBT::NBTWriter writer(dat_file.c_str());
        writer.writeListHead("servers", NBT::idCompound, 0);
        writer.endCompound();
        writer.close();
    }

    // Read back
    std::vector<nbtserver> servers = parse_servers_dat(dat_file);
    TEST_CHECK(servers.empty());

    std::remove(dat_file.c_str());
}

// Test with special characters in server names
void test_special_characters(void) {
    std::string dat_file = get_temp_path("test_special.dat");

    std::vector<nbtserver> original = {
        {"icon", "127.0.0.1", "Server with spaces", true},
        {"icon", "127.0.0.2", "Server_with_underscores", false},
        {"icon", "127.0.0.3", "Server-with-dashes", true}
    };

    // Write to DAT
    {
        NBT::NBTWriter writer(dat_file.c_str());
        writer.writeListHead("servers", NBT::idCompound, original.size());
        for (const auto& server : original) {
            writer.writeCompound("");
            writer.writeString("name", server.name.data());
            writer.writeString("icon", server.icon.data());
            writer.writeString("ip", server.ip.data());
            writer.writeByte("acceptTextures", server.accept_textures);
            writer.endCompound();
        }
        writer.endCompound();
        writer.close();
    }

    // Read back
    std::vector<nbtserver> read_servers = parse_servers_dat(dat_file);
    TEST_CHECK(read_servers.size() == 3);
    TEST_CHECK(read_servers[0].name == "Server with spaces");
    TEST_CHECK(read_servers[1].name == "Server_with_underscores");
    TEST_CHECK(read_servers[2].name == "Server-with-dashes");

    std::remove(dat_file.c_str());
}

TEST_LIST = {
    { "Full CSV roundtrip", test_full_csv_roundtrip },
    { "Full JSON roundtrip", test_full_json_roundtrip },
    { "Full TOML roundtrip", test_full_toml_roundtrip },
    { "Parse DAT invalid file", test_parse_dat_invalid_file },
    { "Parse DAT empty path", test_parse_dat_empty_path },
    { "Parse DAT malformed", test_parse_dat_malformed },
    { "Large server list", test_large_server_list },
    { "Empty server list", test_empty_server_list },
    { "Special characters", test_special_characters },
    { NULL, NULL }
};
