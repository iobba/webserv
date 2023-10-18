#include "../Includes/ServManager.hpp"
#include "../Includes/parseconfig.hpp"

void signal_handler(int __sig)
{
    if (__sig == SIGINT)
    {
        std::exit(0);
    }
}

int main(int ac, char **av)
{
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    if (ac <= 2)
    {
        std::string config_file;
        if (ac == 2)
            config_file = av[1];
        else
            config_file = "config/default.conf";
        // parsing the confige file
        ParseConfig     parse_config;
        try
        {
            parse_config.parsing_config(config_file);
        }
        catch(std::exception & e)
        {
            std::cout << "CONFIG_ERROR : " << e.what() << std::endl;
            exit (1);
        }
        // getting started with the servers
        ServManager  _manager;
        _manager.launch_servers(parse_config.get_servers());
    }
    else
    {
        std::cout << "unvalid number of arguments!\nUSAGE : ./webserv \"path/to/a/config/file\"" << std::endl;
        return (1);
    }

    return (0);
}
