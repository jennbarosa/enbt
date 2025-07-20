#include "parse.hpp"
#include "toml.hpp"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <iostream>

std::vector<nbtserver> parse_servers_json(const std::string& content) {
	using json = nlohmann::json;
	if (content.empty()) {
		std::cout << "json file content is empty. no servers.dat created\n";
		return {};
	}
	
	if (!json::accept(content)) {
		// TODO show where its malformed/show error from nlohmann json?
		std::cout << "json is malformed. validate the syntax and try again\n";
		return {};
	}

	json config = json::parse(content);
	if (!config.contains("servers") || !config["servers"].is_array()) {
		std::cout << "json is malformed. requires a 'servers' array\n";
		return {};
	}

	std::vector<nbtserver> servers{};
    	servers.reserve(config["servers"].size());

	for (const auto& server : config["servers"]) {
		if (!server.contains("icon") || !server.contains("ip") || 
			!server.contains("name") || !server.contains("accept_textures")) {
			std::cout << "warning: a server entry is missing required fields. it will not be added to the servers list\n";
			continue;
		}

		servers.emplace_back(nbtserver{
			.icon = server.at("icon").get<std::string>(),
			.ip = server.at("ip").get<std::string>(),
			.name = server.at("name").get<std::string>(),
			.accept_textures = server.at("accept_textures").get<bool>()
		});
	}

	return servers;
}

std::vector<nbtserver> parse_servers_toml(const std::string& content) {
	if (content.empty()) {
		std::cout << "toml file content is empty. no servers.dat created\n";
		return {};
	}

  	const auto maybe_parsed = toml::try_parse_str(content);
	if (!maybe_parsed.is_ok()) {
		// TODO show error source from try_parse?
		std::cout << "toml file is malformed. validate the syntax and try again\n"; 
		return {};
	}

	auto config = maybe_parsed.unwrap();
	if (!config.at("servers").is_array_of_tables()) {
		std::cout << "toml is malformed. requires a 'servers' table as an array of tables\n";
		return {};
	}

	auto& server_tables = config["servers"].as_array();
	std::vector<nbtserver> servers{};
	servers.reserve(server_tables.size());

	for (const auto& server : server_tables) {
		auto icon = toml::find_or<std::string>(server, "icon", "");
		auto ip = toml::find_or<std::string>(server, "ip", "");
		auto name = toml::find_or<std::string>(server, "name", "");
		auto accept_textures = toml::find_or<bool>(server, "accept_textures", false);

		if (icon.empty() || ip.empty() || name.empty()) {
			std::cout << "warning: a server entry is missing required fields. it will not be added to the servers list\n";
			continue;
		}

		servers.emplace_back(nbtserver{
			.icon = icon,
			.ip = ip,
			.name = name,
			.accept_textures = accept_textures
		});
	}
	
	return servers;
}

std::vector<nbtserver> parse_servers_csv(const std::string& content) {
	if (content.empty()) {
		std::cout << "csv file content is empty. no servers.dat created\n";
		return {};
	}
	
	const size_t server_count = std::count(content.begin(), content.end(), '\n');

	std::vector<nbtserver> servers{};
	servers.reserve(server_count);

	std::string line;
	std::istringstream stream{content};
	while (std::getline(stream, line)) {
		// get nbt properties for this server by splitting delimiter
		// Example: Server Name,base6409ujisdfskdf,127.0.0.1,0
		std::vector<std::string> items{};
		for (std::size_t i = 0, pos = 0; pos < line.size(); ++i) {
			auto const next_pos = std::find_if(line.begin() + pos, line.end(), [](auto& c){return c == ','||c=='|'||c==';';}) - line.begin();
			items.push_back(line.substr(pos, next_pos - pos));
			pos = next_pos == line.size() ? line.size() : next_pos + 1;
		}	

		if (items.size() < 4) {
			std::cout << "warning: a server entry is missing required fields. it will not be added to the servers list\n";
			continue;
		}

		servers.emplace_back(nbtserver{ 
			.icon = items[1], 
			.ip = items[2], 
			.name = items[0], 
			.accept_textures = items[3][0] == '1' 
		});
	}

	return servers;
}
