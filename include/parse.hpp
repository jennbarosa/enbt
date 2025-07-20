#ifndef ENBT_PARSE_H
#define ENBT_PARSE_H

#include <string>
#include <vector>

struct nbtserver {
	std::string icon; // base64
	std::string ip;
	std::string name;
	bool accept_textures;
};

std::vector<nbtserver> parse_servers_json(const std::string& content);
std::vector<nbtserver> parse_servers_toml(const std::string& content);
std::vector<nbtserver> parse_servers_csv(const std::string& content);

#endif
