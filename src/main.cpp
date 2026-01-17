#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "parse.hpp"
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
	std::cout << "Usage: " << program << " -i <ip_list_input> [options]\n";
	std::cout << "Options\n";
	std::cout << "\t-i <input_file>\t\t\tInput file with list of ips\n";
	std::cout << "\t-t <csv|toml|json>\t\tSpecifies the type of input file\n";
	std::cout << "\t-o <output_path>\t\tSpecifies the output. Default is 'servers.dat'\n";
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

int main(int argc, char** argv) {
	const std::string_view program = argv[0];
	argv++;
	argc--;

	std::string input_path{};
	std::string output_path = "servers.dat";	
	std::string input_type = "csv";
	bool explicit_extension = false;

	while (argc > 0) {
		const std::string_view cmd = argv[0];
		if (cmd == "-i") {
			parse_arg(cmd, input_path, "", &argc, &argv, true);	
		} else if (cmd == "-o") {
			parse_arg(cmd, output_path, "servers.dat", &argc, &argv, true);
		} else if (cmd == "-t") {
			parse_arg(cmd, input_type, "csv", &argc, &argv, true);
			explicit_extension = true;
		} else {		
			std::cout << "unknown option '" << cmd << "'\n";
			usage(program);
			exit(1);
		}
		argv++;
		argc--;
	}

	std::ifstream ip_file_stream;
	std::istream* ip_stream;
	bool cin_piped = !isatty(fileno(stdin));
	if (input_path.empty()) {
		if (!cin_piped) {
			std::cout << "No input data provided\n";
			exit(1);
		}
		// input is piped. -i is ignored
		ip_stream = &std::cin;
	} else {
		ip_file_stream.open(input_path.data());
		if (!ip_file_stream.is_open()) {
			std::cout << "Unable to open input file for reading (" << input_path << ")\n";
			exit(1);
		}
		ip_stream = &ip_file_stream;
	}

	if (!fs::exists(input_path) && !cin_piped) {
		std::cout << "Can't load '" << input_path << "': file doesn't exist\n";
		exit(1);
	}

	if (!explicit_extension) {
		if (cin_piped) {
			std::cout << "You're piping data to enbt, but I have no idea what format it is. You need to provide the '-t' option\n";
			exit(1);
		}

		// work how its expected to unless overriden with -t
		auto ext = fs::path(input_path).extension().string();
		if (ext.empty()) {
			std::cout << "The input file provided does not have an extension. Provide an explicit input type with the -t option\n";
			exit(1);
		}
		input_type = ext.erase(0, 1); //remove '.'
	}

	if (input_type != "csv" && input_type != "toml" && input_type != "json") {
		std::cout << "Invalid value for -t '" << input_type << "'\n";
		exit(1);
	}
	
	ips_to_dat(ip_stream, output_path, input_type);
	
	return 0;
}

