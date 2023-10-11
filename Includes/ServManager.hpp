#ifndef SERVMANAGER_HPP
#define SERVMANAGER_HPP

#include "webserv.hpp"
#include "client.hpp"
#include "server.hpp"

class   ServManager
{
    private:
        std::map<int, Server>   _servers_map;
        std::map<int, Client>   _clients_map;
        fd_set                  read_set;
        fd_set                  write_set;
        int                     max_Fd;

    public:
        int nb_req;
        ServManager();
        void    configure_servers(std::vector<Server> servers_vec); /// this should setup all the serv_sockets 
        void    setup_sets();
        int     handle_connections(fd_set *tmp_readset);
        int     close_connection(int to_close);
        int     handle_request(fd_set *tmp_readset);
        int     read_request(int client_socket, Client &_client_);
        int     handle_response(fd_set *tmp_writeset);
        int     send_response(int client_socket, Client &_client_);
        int     send_headers(int client_socket, Client &_client_);
        int     send_string(int client_socket, Client &_client_);
        int     send_file(int client_socket, Client &_client_);
        int     launch_servers(std::vector<Server> _servers_);
        void    print_esrvers_map();

        // setters
        void    set_maxFd(int biggest);

};






#endif