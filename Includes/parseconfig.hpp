#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include "webserv.hpp"
#include "server.hpp"

class   ParseConfig
{
	private:
		std::vector<Server>     __servers;
		std::string             __content;
	
	public:
		ParseConfig();
		void                        parsing_config(std::string  config_file);
		void                        read_config(std::string __path);
		void                        clean_content();
		std::vector<std::string>    get_server_scopes();
		void                    	add_server(std::string server_scope);
		void						print_servers();
		const std::vector<Server>&	get_servers();
};

#endif