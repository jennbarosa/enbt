#include "acutest.h"
#include "parse.hpp"
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <functional>

static std::string capture_output(const std::function<void()>& fn) {
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    fn();
    std::cout.rdbuf(old);
    return oss.str();
}

void test_parse_csv_empty(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_csv("");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "csv file content is empty. no servers.dat created\n");
}

void test_parse_csv_missing_property(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_csv("Server%c%c1.0.0.1%c1");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "warning: a server entry is missing required fields. it will not be added to the servers list\n");
}

void test_parse_csv_delims(void) {
	std::vector<char> delims{',', '|', ';'};
	std::string output = capture_output([&](){
		for (char c : delims) {
		    	char csv_out[30]; 
			const std::string csv_template = "Server%csoidfjsiodf%c1.0.0.1%c1";
			std::snprintf(csv_out, sizeof(csv_out), csv_template.c_str(), c, c, c);
			std::string csv_string(csv_out);
			std::vector<nbtserver> servers = parse_servers_csv(csv_string);
			TEST_CHECK(servers.size() == 1);
			TEST_CHECK(servers[0].icon == "soidfjsiodf");
			TEST_CHECK(servers[0].name == "Server");
			TEST_CHECK(servers[0].ip == "1.0.0.1");
			TEST_CHECK(servers[0].accept_textures);
		}
	});
	TEST_CHECK(output != "csv file content is empty. no servers.dat created\n");
}

void test_parse_csv_load(void) {
	std::string output = capture_output([&](){
		constexpr size_t load = 50000;
		std::stringstream buffer;
		for (size_t i = 0; i < load; ++i) {
			char csv_out[256]; 
			const std::string csv_template = "Server%d%csoidfjsiodf%c1.0.0.1%c1";
			std::snprintf(csv_out, sizeof(csv_out), csv_template.c_str(), i+1, ',', ',', ',');
			std::string csv_string(csv_out);
			buffer << csv_out << '\n'; 
		}
		std::vector<nbtserver> servers = parse_servers_csv(buffer.str());
		TEST_CHECK(servers.size() == load);

		for (size_t i = 0; i < servers.size(); ++i) {
			std::string name = "Server" + std::to_string(i+1);
			TEST_CHECK_(servers[i].icon == "soidfjsiodf", "icon was %s", servers[i].icon.c_str());
			TEST_CHECK_(servers[i].name == name, "name was %s", servers[i].name.c_str());
			TEST_CHECK_(servers[i].ip == "1.0.0.1", "ip was %s", servers[i].ip.c_str());
			TEST_CHECK_(servers[i].accept_textures, "accept_textures was %d", servers[i].accept_textures);
		}
	});
	TEST_CHECK(output != "csv file content is empty. no servers.dat created\n");
}

void test_parse_toml_empty(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_toml("");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "toml file content is empty. no servers.dat created\n");
}

void test_parse_toml_servers_not_a_table(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_toml(R"(
			servers = "this is not a table"
		)");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "toml is malformed. requires a 'servers' table as an array of tables\n");
}

void test_parse_toml_servers_entry_missing_property(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_toml(R"(
			[[servers]]
			icon = "/9j/4AAQSkZJRgABAQIAJQAl"
			name = "Server One Toml"
			accept_textures = true
		)");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK_(output == "warning: a server entry is missing required fields. it will not be added to the servers list\n", "output was %s", output.c_str());
}

void test_parse_toml_servers_parse(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_toml(R"(
			[[servers]]
			icon = "/9j/4AAQSkZJRgABAQIAJQAl"
			ip = "192.168.1.1"
			name = "Server One Toml"
			accept_textures = true

			[[servers]]
			icon = "/9j/4AAQSkZJRgABAQIAJQAl"
			ip = "192.168.1.2"
			name = "Server Two Toml"
			accept_textures = true
		)");
		TEST_CHECK(servers.size() == 2);
	});
	TEST_CHECK(output.empty());
}

void test_parse_toml_servers_malformed_table(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_toml("[[servers]]}D{FG]]d]f[g]{DF}g[fg");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "toml file is malformed. validate the syntax and try again\n");
}

void test_parse_json_empty(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_json("");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "json file content is empty. no servers.dat created\n");
}

void test_parse_json_malformed_object(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_json("{[S{DF}{}[]sd]f[}SDF{S}DfSDf[p}");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "json is malformed. validate the syntax and try again\n");
}

void test_parse_json_servers_not_an_array(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_json(R"({"servers": "not an array"})");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "json is malformed. requires a 'servers' array\n");
}

void test_parse_json_servers_missing_servers(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_json(R"({"somethingelse": "not servers"})");
		TEST_CHECK(servers.empty());
	});
	TEST_CHECK(output == "json is malformed. requires a 'servers' array\n");
}

void test_parse_json_servers_parse_skipping_malformed(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_json(R"(
			{
			  "servers": [
			    {
			      "icon": "/9j/4AAQSkZJRgABAQIAJQAl",
			      "ip": "192.168.1.1",
			      "name": "Server One Json",
			      "accept_textures": true
			    },
			    {
			      "icon": "/9j/4AAQSkZJRgABAQIAJQAl",
			      "name": "Server Two bad Json",
			      "accept_textures": false
			    },
			    {
			      "icon": "/9j/4AAQSkZJRgABAQIAJQAl",
			      "ip": "192.168.1.4",
			      "name": "Server One Json",
			      "accept_textures": true
			    },
			    {
			      "icon": "/9j/4AAQSkZJRgABAQIAJQAl",
			      "ip": "192.168.1.1",
			      "accept_textures": false
			    }
			  ]
			}
		)");
		TEST_CHECK(servers.size() == 2);
	});
	TEST_CHECK(output == "warning: a server entry is missing required fields. it will not be added to the servers list\nwarning: a server entry is missing required fields. it will not be added to the servers list\n");
}

void test_parse_json_servers_parse(void) {
	std::string output = capture_output([&](){
		std::vector<nbtserver> servers = parse_servers_json(R"(
			{
			  "servers": [
			    {
			      "icon": "icon1",
			      "ip": "ip1",
			      "name": "name1",
			      "accept_textures": true
			    },
			    {
			      "icon": "icon2",
			      "ip": "ip2",
			      "name": "name2",
			      "accept_textures": true
			    }
			  ]
			}
		)");
		TEST_CHECK(servers.size() == 2);
		for (size_t i = 0; i < servers.size(); ++i) {
			const auto idx = std::to_string(i+1);
			TEST_CHECK(servers[i].icon == "icon" + idx);
			TEST_CHECK(servers[i].ip == "ip" + idx);
			TEST_CHECK(servers[i].name == "name" + idx);
			TEST_CHECK(servers[i].accept_textures);
		}
	});
	TEST_CHECK(output.empty());
}


TEST_LIST = {
   { "Parse CSV - empty", test_parse_csv_empty },
   { "Parse CSV - delims", test_parse_csv_delims },
   { "Parse CSV - load", test_parse_csv_load },
   { "Parse CSV - missing property", test_parse_csv_missing_property },
   { "Parse TOML - empty", test_parse_toml_empty },
   { "Parse TOML - when servers is not a table", test_parse_toml_servers_not_a_table },
   { "Parse TOML - missing property", test_parse_toml_servers_entry_missing_property },
   { "Parse TOML - parse", test_parse_toml_servers_parse },
   { "Parse TOML - when servers is malformed table", test_parse_toml_servers_malformed_table },
   { "Parse JSON - empty", test_parse_json_empty },
   { "Parse JSON - malformed object", test_parse_json_malformed_object },
   { "Parse JSON - missing servers key", test_parse_json_servers_missing_servers },
   { "Parse JSON - servers key but not an array", test_parse_json_servers_not_an_array },
   { "Parse JSON - parse skip malformed", test_parse_json_servers_parse_skipping_malformed },
   { "Parse JSON - parse", test_parse_json_servers_parse },
   { NULL, NULL }
};


