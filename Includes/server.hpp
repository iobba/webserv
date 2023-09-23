#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"
#include "Location.hpp"

class   Server
{
    private:
        int                         _socket;
        std::string                 _port; // default 80
        struct sockaddr_in          _server_info;
        std::string                 _server_name;
        std::string                 _host;// or you can take the entire addr if needed // default 127.0.0.1
        std::string                 _root;
        unsigned long               _client_max_body_size; // default 3000000
        std::string                 _index;// default index.html
        bool                        _autoindex; // default off
        std::map<int, std::string>  _error_pages;// should be some default ones
        std::map<int, std::string>  _error_messages;
        std::vector<Location>       _locations;
        //  ... other info 
    
    public:

        // setters
        Server();
        void        errors_initialisation();
        void        set_socket(int  sock);
        void        set_server_info(struct sockaddr_in infos);
        void        set_port(std::string port);
        void        set_server_name(std::string server_name);
        void        set_host(std::string host);
        void        set_root(std::string root);
        void        set_client_max_body_size(std::string client_max_body_size);
        void        set_index(std::string index);
        void        set_autoindex(std::string autoindex);
        void        add_error_page(std::string key, std::string value);
        void        add_location(std::string _path_, std::vector<std::string> lines);
        // ...

        // getters
        bool                            _has_max_body_setten;
        bool                            _has_autoindex_setten;
        const int&                      get_socket() const;
        const struct sockaddr_in&       get_server_info() const;
        const std::string&              get_port() const;
        const std::string&              get_server_name() const;
        const std::string&              get_host() const;
        const std::string&              get_root() const;
        const unsigned long&            get_client_max_body_size() const;
        const std::string&              get_index() const;
        bool                            is_autoindex() const;
        const std::string               get_error_page(int key) const;
        const std::string               get_error_messages(int key) const;
        const std::vector<Location>&    get_locations() const;
        // i can get more getters for locations
        // ...

};









#endif