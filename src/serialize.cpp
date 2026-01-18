#include "serialize.hpp"
#include "nlohmann/json.hpp"
#include <sstream>

std::string serialize_servers_csv(const std::vector<nbtserver>& servers) {
    std::ostringstream oss;
    for (const auto& server : servers) {
        oss << server.name << ','
            << server.icon << ','
            << server.ip << ','
            << (server.accept_textures ? '1' : '0') << '\n';
    }
    return oss.str();
}

std::string serialize_servers_json(const std::vector<nbtserver>& servers) {
    using json = nlohmann::json;
    json output;
    output["servers"] = json::array();

    for (const auto& server : servers) {
        json entry;
        entry["icon"] = server.icon;
        entry["ip"] = server.ip;
        entry["name"] = server.name;
        entry["accept_textures"] = server.accept_textures;
        output["servers"].push_back(entry);
    }

    return output.dump(2);  // Pretty print with 2-space indent
}

std::string serialize_servers_toml(const std::vector<nbtserver>& servers) {
    std::ostringstream oss;

    for (const auto& server : servers) {
        oss << "[[servers]]\n";
        oss << "icon = \"" << server.icon << "\"\n";
        oss << "ip = \"" << server.ip << "\"\n";
        oss << "name = \"" << server.name << "\"\n";
        oss << "accept_textures = " << (server.accept_textures ? "true" : "false") << "\n";
        oss << "\n";
    }

    return oss.str();
}
