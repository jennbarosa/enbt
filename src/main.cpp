#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "parse.hpp"
#include "serialize.hpp"
#include "NBTWriter.h"
#include <vector>

namespace fs = std::filesystem;

#ifdef _WIN32
#include <io.h> // _isatty
#define isatty _isatty
#else
#include <unistd.h> // isatty
#endif

void usage(const std::string_view program) {
	std::cout << "Usage: " << program << " -i <input_file> [options]\n";
	std::cout << "Options\n";
	std::cout << "\t-i <input_file>\t\t\tInput file\n";
	std::cout << "\t-t <csv|toml|json>\t\tSpecifies the type of input/output file\n";
	std::cout << "\t-o <output_path>\t\tSpecifies the output path\n";
	std::cout << "\t-r, --reverse\t\t\tReverse mode: convert servers.dat to CSV/JSON/TOML\n";
	std::cout << "\nExamples:\n";
	std::cout << "  Forward:  " << program << " -i servers.csv -o servers.dat\n";
	std::cout << "  Reverse:  " << program << " -r -i servers.dat -t csv -o servers.csv\n";
}

void parse_arg(const std::string_view cmd, 
	       std::string& stored_value,
	       const std::string_view default_value, 
	       int* argc, 
	       char*** argv,
	       bool arg_required=false) {
	if (arg_required && (*argc) < 2) {
		std::cout << '\'' << cmd << "' requires an argument\n";
		exit(1);
	}
	if (stored_value == default_value)
		stored_value = (*argv)[1];
	(*argv)++;
	(*argc)--;
}


void ips_to_dat(std::istream* ip_stream, const std::string_view output_path, const std::string_view format) {
	fs::path output_fs_path = output_path;
	if (output_fs_path.empty()) {
		std::cout << "Output path is empty\n";
		exit(1);
	}

	if (fs::is_directory(output_fs_path)) {
		output_fs_path = output_fs_path / "servers.dat";	
	}

	std::stringstream buffer;
	buffer << ip_stream->rdbuf();

	const std::string ips_content = buffer.str();
	const std::vector<nbtserver> servers = [&](){
		if (format == "csv")
			return parse_servers_csv(ips_content);
		else if (format == "toml")
			return parse_servers_toml(ips_content);
		else if (format == "json")
			return parse_servers_json(ips_content);
		return std::vector<nbtserver>{};
	}(); 

	if (servers.empty()) {
		std::cout << "There are no servers in your input file\n";
		exit(1);
	}

	NBT::NBTWriter writer(output_fs_path.string().data());
	writer.writeListHead("servers", NBT::idCompound, servers.size());
	for (const nbtserver& server : servers) {	
		#if 0
		std::cout << server.name << '\n';
		std::cout << server.icon << '\n';
		std::cout << server.ip << '\n';
		std::cout << server.accept_textures << '\n';
		std::cout << "------------------\n"; 
		#endif
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

void dat_to_format(const std::string_view input_path,
				   const std::string_view output_path,
				   const std::string_view format) {

	const std::vector<nbtserver> servers = parse_servers_dat(std::string(input_path));

	if (servers.empty()) {
		std::cout << "No servers found in " << input_path << "\n";
		exit(1);
	}

	const std::string output_content = [&]() {
		if (format == "csv")
			return serialize_servers_csv(servers);
		else if (format == "toml")
			return serialize_servers_toml(servers);
		else if (format == "json")
			return serialize_servers_json(servers);
		return std::string{};
	}();

	if (output_content.empty()) {
		std::cout << "Failed to serialize servers\n";
		exit(1);
	}

	// Write to file or stdout
	if (output_path == "-") {
		std::cout << output_content;
	} else {
		std::string output_path_str{output_path};
		std::ofstream out{output_path_str};
		if (!out.is_open()) {
			std::cout << "Unable to open output file for writing (" << output_path << ")\n";
			exit(1);
		}
		out << output_content;
		out.close();
	}
}

int main(int argc, char** argv) {
	const std::string_view program = argv[0];
	argv++;
	argc--;

	std::string input_path{};
	std::string output_path = "servers.dat";
	std::string input_type = "csv";
	bool explicit_extension = false;
	bool reverse_mode = false;

	while (argc > 0) {
		const std::string_view cmd = argv[0];
		if (cmd == "-i") {
			parse_arg(cmd, input_path, "", &argc, &argv, true);
		} else if (cmd == "-o") {
			parse_arg(cmd, output_path, "servers.dat", &argc, &argv, true);
		} else if (cmd == "-t") {
			parse_arg(cmd, input_type, "csv", &argc, &argv, true);
			explicit_extension = true;
		} else if (cmd == "-r" || cmd == "--reverse") {
			reverse_mode = true;
		} else {
			std::cout << "unknown option '" << cmd << "'\n";
			usage(program);
			exit(1);
		}
		argv++;
		argc--;
	}

	// Validate input
	if (input_path.empty()) {
		std::cout << "No input file provided (-i required)\n";
		usage(program);
		exit(1);
	}

	if (!fs::exists(input_path)) {
		std::cout << "Can't load '" << input_path << "': file doesn't exist\n";
		exit(1);
	}

	// Handle reverse mode vs forward mode
	if (reverse_mode) {
		// servers.dat -> CSV/JSON/TOML
		if (!explicit_extension) {
			std::cout << "Reverse mode requires explicit output format (-t csv|json|toml)\n";
			exit(1);
		}

		if (input_type != "csv" && input_type != "toml" && input_type != "json") {
			std::cout << "Invalid value for -t '" << input_type << "'\n";
			exit(1);
		}

		dat_to_format(input_path, output_path, input_type);
	} else {
		// CSV/JSON/TOML -> servers.dat (existing code)
		std::ifstream ip_file_stream;
		std::istream* ip_stream;
		bool cin_piped = !isatty(fileno(stdin));

		ip_file_stream.open(input_path.data());
		if (!ip_file_stream.is_open()) {
			std::cout << "Unable to open input file for reading (" << input_path << ")\n";
			exit(1);
		}
		ip_stream = &ip_file_stream;

		if (!explicit_extension) {
			auto ext = fs::path(input_path).extension().string();
			if (ext.empty()) {
				std::cout << "The input file provided does not have an extension. "
						  << "Provide an explicit input type with the -t option\n";
				exit(1);
			}
			input_type = ext.erase(0, 1);
		}

		if (input_type != "csv" && input_type != "toml" && input_type != "json") {
			std::cout << "Invalid value for -t '" << input_type << "'\n";
			exit(1);
		}

		ips_to_dat(ip_stream, output_path, input_type);
	}

	return 0;
}

