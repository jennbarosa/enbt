#ifndef ENBT_SERIALIZE_H
#define ENBT_SERIALIZE_H

#include <string>
#include <vector>
#include "parse.hpp"

// Convert vector<nbtserver> to formatted strings
std::string serialize_servers_csv(const std::vector<nbtserver>& servers);
std::string serialize_servers_json(const std::vector<nbtserver>& servers);
std::string serialize_servers_toml(const std::vector<nbtserver>& servers);

#endif
