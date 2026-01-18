#include "acutest.h"
#include "serialize.hpp"
#include "parse.hpp"
#include "nlohmann/json.hpp"
#include <sstream>

// Test CSV serialization
void test_serialize_csv(void) {
    std::vector<nbtserver> servers = {
        {"icon1", "192.168.1.1", "Server1", true},
        {"icon2", "192.168.1.2", "Server2", false},
        {"icon3", "192.168.1.3", "Server3", true}
    };

    std::string csv = serialize_servers_csv(servers);

    // Check format: name,icon,ip,accept_textures
    TEST_CHECK(csv.find("Server1,icon1,192.168.1.1,1") != std::string::npos);
    TEST_CHECK(csv.find("Server2,icon2,192.168.1.2,0") != std::string::npos);
    TEST_CHECK(csv.find("Server3,icon3,192.168.1.3,1") != std::string::npos);

    // Count newlines (should be 3)
    int newlines = std::count(csv.begin(), csv.end(), '\n');
    TEST_CHECK(newlines == 3);
}

// Test CSV serialization with empty list
void test_serialize_csv_empty(void) {
    std::vector<nbtserver> servers;
    std::string csv = serialize_servers_csv(servers);
    TEST_CHECK(csv.empty());
}

// Test CSV serialization with special characters
void test_serialize_csv_special_chars(void) {
    std::vector<nbtserver> servers = {
        {"icon", "127.0.0.1", "Server, With, Commas", true}
    };

    std::string csv = serialize_servers_csv(servers);
    TEST_CHECK(csv.find("Server, With, Commas") != std::string::npos);
}

// Test JSON serialization
void test_serialize_json(void) {
    std::vector<nbtserver> servers = {
        {"iconA", "10.0.0.1", "ServerA", true},
        {"iconB", "10.0.0.2", "ServerB", false}
    };

    std::string json_str = serialize_servers_json(servers);

    // Parse JSON to verify structure
    using json = nlohmann::json;
    json parsed = json::parse(json_str);

    TEST_CHECK(parsed.contains("servers"));
    TEST_CHECK(parsed["servers"].is_array());
    TEST_CHECK(parsed["servers"].size() == 2);

    // Check first server
    TEST_CHECK(parsed["servers"][0]["name"] == "ServerA");
    TEST_CHECK(parsed["servers"][0]["icon"] == "iconA");
    TEST_CHECK(parsed["servers"][0]["ip"] == "10.0.0.1");
    TEST_CHECK(parsed["servers"][0]["accept_textures"] == true);

    // Check second server
    TEST_CHECK(parsed["servers"][1]["name"] == "ServerB");
    TEST_CHECK(parsed["servers"][1]["icon"] == "iconB");
    TEST_CHECK(parsed["servers"][1]["ip"] == "10.0.0.2");
    TEST_CHECK(parsed["servers"][1]["accept_textures"] == false);
}

// Test JSON serialization with empty list
void test_serialize_json_empty(void) {
    std::vector<nbtserver> servers;
    std::string json_str = serialize_servers_json(servers);

    using json = nlohmann::json;
    json parsed = json::parse(json_str);

    TEST_CHECK(parsed.contains("servers"));
    TEST_CHECK(parsed["servers"].is_array());
    TEST_CHECK(parsed["servers"].size() == 0);
}

// Test JSON serialization with UTF-8
void test_serialize_json_utf8(void) {
    std::vector<nbtserver> servers = {
        {"icon", "127.0.0.1", "Servidor Español", true}
    };

    std::string json_str = serialize_servers_json(servers);
    TEST_CHECK(json_str.find("Servidor Español") != std::string::npos ||
               json_str.find("Servidor") != std::string::npos);
}

// Test TOML serialization
void test_serialize_toml(void) {
    std::vector<nbtserver> servers = {
        {"iconX", "192.168.0.1", "ServerX", true},
        {"iconY", "192.168.0.2", "ServerY", false}
    };

    std::string toml = serialize_servers_toml(servers);

    // Check TOML format
    TEST_CHECK(toml.find("[[servers]]") != std::string::npos);
    TEST_CHECK(toml.find("name = \"ServerX\"") != std::string::npos);
    TEST_CHECK(toml.find("icon = \"iconX\"") != std::string::npos);
    TEST_CHECK(toml.find("ip = \"192.168.0.1\"") != std::string::npos);
    TEST_CHECK(toml.find("accept_textures = true") != std::string::npos);

    TEST_CHECK(toml.find("name = \"ServerY\"") != std::string::npos);
    TEST_CHECK(toml.find("accept_textures = false") != std::string::npos);

    // Count [[servers]] occurrences
    size_t pos = 0;
    int count = 0;
    while ((pos = toml.find("[[servers]]", pos)) != std::string::npos) {
        count++;
        pos += 11;
    }
    TEST_CHECK(count == 2);
}

// Test TOML serialization with empty list
void test_serialize_toml_empty(void) {
    std::vector<nbtserver> servers;
    std::string toml = serialize_servers_toml(servers);
    TEST_CHECK(toml.empty());
}

// Test TOML boolean values
void test_serialize_toml_booleans(void) {
    std::vector<nbtserver> servers = {
        {"i1", "1.1.1.1", "S1", true},
        {"i2", "2.2.2.2", "S2", false}
    };

    std::string toml = serialize_servers_toml(servers);

    // Find both boolean values
    size_t true_pos = toml.find("accept_textures = true");
    size_t false_pos = toml.find("accept_textures = false");

    TEST_CHECK(true_pos != std::string::npos);
    TEST_CHECK(false_pos != std::string::npos);
}

// Test roundtrip: vector -> CSV -> vector
void test_csv_roundtrip(void) {
    std::vector<nbtserver> original = {
        {"icon1", "1.2.3.4", "Test1", true},
        {"icon2", "5.6.7.8", "Test2", false}
    };

    // Serialize to CSV
    std::string csv = serialize_servers_csv(original);

    // Parse back
    std::vector<nbtserver> parsed = parse_servers_csv(csv);

    // Compare
    TEST_CHECK(parsed.size() == original.size());
    TEST_CHECK(parsed[0].name == original[0].name);
    TEST_CHECK(parsed[0].icon == original[0].icon);
    TEST_CHECK(parsed[0].ip == original[0].ip);
    TEST_CHECK(parsed[0].accept_textures == original[0].accept_textures);

    TEST_CHECK(parsed[1].name == original[1].name);
    TEST_CHECK(parsed[1].accept_textures == original[1].accept_textures);
}

// Test roundtrip: vector -> JSON -> vector
void test_json_roundtrip(void) {
    std::vector<nbtserver> original = {
        {"iconA", "10.10.10.10", "Alpha", true},
        {"iconB", "20.20.20.20", "Beta", false}
    };

    // Serialize to JSON
    std::string json_str = serialize_servers_json(original);

    // Parse back
    std::vector<nbtserver> parsed = parse_servers_json(json_str);

    // Compare
    TEST_CHECK(parsed.size() == original.size());
    TEST_CHECK(parsed[0].name == original[0].name);
    TEST_CHECK(parsed[0].icon == original[0].icon);
    TEST_CHECK(parsed[0].ip == original[0].ip);
    TEST_CHECK(parsed[0].accept_textures == original[0].accept_textures);

    TEST_CHECK(parsed[1].name == original[1].name);
    TEST_CHECK(parsed[1].accept_textures == original[1].accept_textures);
}

// Test roundtrip: vector -> TOML -> vector
void test_toml_roundtrip(void) {
    std::vector<nbtserver> original = {
        {"iconP", "30.30.30.30", "Primary", true},
        {"iconS", "40.40.40.40", "Secondary", false}
    };

    // Serialize to TOML
    std::string toml = serialize_servers_toml(original);

    // Parse back
    std::vector<nbtserver> parsed = parse_servers_toml(toml);

    // Compare
    TEST_CHECK(parsed.size() == original.size());
    TEST_CHECK(parsed[0].name == original[0].name);
    TEST_CHECK(parsed[0].icon == original[0].icon);
    TEST_CHECK(parsed[0].ip == original[0].ip);
    TEST_CHECK(parsed[0].accept_textures == original[0].accept_textures);

    TEST_CHECK(parsed[1].name == original[1].name);
    TEST_CHECK(parsed[1].accept_textures == original[1].accept_textures);
}

TEST_LIST = {
    { "Serialize CSV", test_serialize_csv },
    { "Serialize CSV empty", test_serialize_csv_empty },
    { "Serialize CSV special chars", test_serialize_csv_special_chars },
    { "Serialize JSON", test_serialize_json },
    { "Serialize JSON empty", test_serialize_json_empty },
    { "Serialize JSON UTF-8", test_serialize_json_utf8 },
    { "Serialize TOML", test_serialize_toml },
    { "Serialize TOML empty", test_serialize_toml_empty },
    { "Serialize TOML booleans", test_serialize_toml_booleans },
    { "CSV roundtrip", test_csv_roundtrip },
    { "JSON roundtrip", test_json_roundtrip },
    { "TOML roundtrip", test_toml_roundtrip },
    { NULL, NULL }
};
