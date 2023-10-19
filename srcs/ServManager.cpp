#include "../Includes/ServManager.hpp"

ServManager::ServManager() : max_Fd(0) {}

int ServManager::launch_servers(std::vector<Server> _servers_)
{
    try
    {
        this->configure_servers(_servers_);
        this->setup_sets();
    }
    catch(std::exception & e)
    {
        std::cout << "CONFIGURING SERVER ERROR : " << e.what() << std::endl;
        exit (1);
    }
    nb_req = 0;
    print_esrvers_map();
    while (true)
    {
        fd_set tmp_ReadSet = this->read_set;
        fd_set tmp_WriteSet = this->write_set;
        int ready = select(this->max_Fd, &tmp_ReadSet, &tmp_WriteSet, NULL, NULL);
        if (ready == -1)
        {                    
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Check for new connections
        if (this->handle_connections(&tmp_ReadSet))
            continue ;

        // Check for data from clients (recieve requests)
        if (this->handle_request(&tmp_ReadSet))
            continue ;

        // Check for sockets ready for writing (send messages)
        this->handle_response(&tmp_WriteSet);
        FD_ZERO(&tmp_ReadSet);
        FD_ZERO(&tmp_WriteSet);
    }
    return (0);
}

void    ServManager::print_esrvers_map()
{
    std::map<int, Server>::iterator it = this->_servers_map.begin();
    int i = 1;
    std::cout << "unique servers are :\n";
    while (it != this->_servers_map.end())
    {
        std::cout << "server " << i << " :\n";
        std::cout << "         + socket -------> " << it->first << std::endl;
        std::cout << "         + server_name --> " << it->second.get_server_name() << std::endl;
        it++;
        i++;
    }
    
    std::vector<Server>::iterator it1 = this->_all_servers.begin();
    i = 1;
    std::cout << "---------------------------------------\naaaaall servers are :\n";
    while (it1 != this->_all_servers.end())
    {
        std::cout << "server " << i << " :\n";
        std::cout << "         + socket -------> " << it1->get_socket() << std::endl;
        std::cout << "         + server_name --> " << it1->get_server_name() << std::endl;
        it1++;
        i++;
    }
}

int is_favicon_ico(std::string _req_)
{
    if (_req_.substr(0, 25).compare("GET /favicon.ico HTTP/1.1") == 0)
        return (1);
    return (0);
}

void    ServManager::set_maxFd(int biggest)
{
    this->max_Fd = biggest;
}

bool    ServManager::is_old_server(Server to_check)
{
    std::string s_host = to_check.get_host();
    std::string s_port = to_check.get_port();
    std::map<int, Server>::iterator it = this->_servers_map.begin();
    int check = false;
    while (it != this->_servers_map.end())
    {
        if (it->second.get_host() == s_host && it->second.get_port() == s_port)
        {
            check = true;
            break ;
        }
        it++;
    }
    if (check)
    {
        to_check.set_socket(it->second.get_socket());
        to_check.set_server_info(it->second.get_server_info());
        this->_all_servers.push_back(to_check);
        return (true);
    }
    return (false);
}

void    ServManager::configure_servers(std::vector<Server> servers_vec)
{
    std::vector<Server>::iterator it = servers_vec.begin();
    while (it != servers_vec.end())
    {
        Server              server = *it;
        int                 server_fd;
        struct sockaddr_in  address;
        int                 yes = 1;

        if (is_old_server(server))
        {
            it++;
            continue ;
        }
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
            throw SERVER_Exception("socket has failed : creating the socket for a server!!");
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(server.get_host().c_str());
        unsigned long port_ = std::strtoul(server.get_port().c_str(), NULL, 10);
        address.sin_port = htons(port_);
        memset(address.sin_zero, '\0', sizeof address.sin_zero);
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
            throw SERVER_Exception("setsockopt has failed !!");
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            close(server_fd);
            perror("bind");
            throw SERVER_Exception("failed to bind!!");
        }
        server.set_socket(server_fd);
        server.set_server_info(address);
        if (listen(server_fd, BACKLOG) == -1)
            throw SERVER_Exception("failed to listen !!");
        this->max_Fd = std::max(this->max_Fd, server_fd + 1);
        this->_servers_map.insert(std::make_pair(server.get_socket(), server));
        this->_all_servers.push_back(server);
        it++;
    }
}

void    ServManager::setup_sets()
{
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);

    std::map<int, Server>::iterator it = _servers_map.begin();
    while (it != _servers_map.end())
    {
        FD_SET(it->first, &read_set);
        it++;
    }
}

int    ServManager::handle_connections(fd_set *tmp_readset)
{
    std::map<int, Server>::iterator it = _servers_map.begin();
    while (it != _servers_map.end())
    {
        if (FD_ISSET(it->first, tmp_readset))
        {
            // new client object
            Client      _new_client;
            int cl_socket = accept(it->first, NULL, NULL);
            if (cl_socket == -1)
            {
                perror("accept");
                return(1) ;
            }
            _new_client.set_socket(cl_socket);
            // set the server which is gonna handle this request
            _new_client._request._default_server = it->second;
            _new_client._request._all_servers = this->_all_servers;
            this->_clients_map.insert(std::make_pair(cl_socket, _new_client));
            FD_SET(cl_socket, &read_set);
            this->max_Fd = std::max(this->max_Fd, cl_socket + 1);
        }
        it++;
    }
    return (0);
}

int     ServManager::handle_request(fd_set *tmp_readset)
{
    std::map<int, Client>::iterator it = _clients_map.begin();
    while (it != _clients_map.end())
    {
        int client_socket = it->first;
        if (FD_ISSET(client_socket, tmp_readset))
        {
            if (read_request(client_socket, it->second))
            {
                it++;
                close_connection(client_socket);
                continue ;
            }
            nb_req++;
            std::cout << "\e[1;32mnumber of requests: " << nb_req << "\e[0m\n";
        }
        ++it;
    }
    return (0);
}

int     ServManager::read_request(int client_socket, Client &_client_)
{
    char buffer[1000000]; // 1024
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read == -1 || bytes_read == 0)
    {
        perror("recv");
        return (1);
    }
    else if (!is_favicon_ico(buffer))
    {
        _client_._request.request_analysis(buffer, bytes_read);
        if (_client_._request._reading_done)
        {
            FD_CLR(client_socket, &read_set);
            FD_SET(client_socket, &write_set);
        }
    }
    else
    {
        _client_._is_favicon = true;
        FD_CLR(client_socket, &read_set);
        FD_SET(client_socket, &write_set);
    }
    return (0);
}

int     ServManager::handle_response(fd_set *tmp_writeset)
{
    std::map<int, Client>::iterator it = _clients_map.begin();
    while (it != _clients_map.end())
    {
        int client_socket = it->first;
        if (FD_ISSET(client_socket, tmp_writeset))
        {
            try 
            {
                if (send_response(it->first, it->second))
                {
                    it++;
                    close_connection(client_socket);
                    continue ;
                }
            }
            catch(std::exception & e)
            {
                it->second._request._status_code = std::strtoul(e.what(), NULL, 10);
                if (it->second._request._status_code == 677173)// CGI
                    continue ;
                if (it->second._request._status_code >= 400)
                {
                    it->second._request._response_body_file = it->second._request.server.get_error_page(it->second._request._status_code);
                    it->second._request._which_body = FILE_BODY;
                }
                else if (it->second._request._status_code >= 300)
                {
                    it->second._request._which_body = NONE;
                }
                it->second._request.set_response_headers(e.what());
                continue ;
            }
        }
        it++;
    }
    return (0);
}

int    ServManager::send_response(int client_socket, Client &_client_)
{
    bool sending_done = false;
    if (_client_._is_favicon)
    {
        _client_._is_favicon = false;
        throw HTTPException(404);
    }
    if (_client_._request._is_cgi)
    {
        if (_client_._request._waiting_done == false)
            _client_._request.waiting_child();
        if (_client_._request._waiting_done == false)
            return(0);
    }
    if (_client_._first_send) // send headers first
    {
        if (send_headers(client_socket, _client_))
            return (1);
    }
    else if (_client_._request._which_body == STR_BODY) // send the body as string
    {
        if (send_string(client_socket, _client_))
            return (1);
        sending_done = true;
    }
    else if (_client_._request._which_body == FILE_BODY) // send the body as file
    {
        int check = send_file(client_socket, _client_);
        if (check == 1)
            return (1);
        if (check == 2)
            sending_done = true;
    }
    else // there is no body
        sending_done = true;
    if (sending_done)
    {
        if (_client_._request._method == POST
            && (_client_._request._serving_location.is_upload() == false || _client_._request._status_code == 413))
        {
            if (unlink(_client_._request._body_name.c_str()) != 0)
            {
                std::cout << "ERROR while deleting the uploaded file\n";
            }
        } 
        return (1);
    }
    return (0);
}

int    ServManager::send_headers(int client_socket, Client &_client_)
{
    std::string headers = _client_._request._response_headers;
    int bytes_sent = send(client_socket, headers.c_str(), headers.length(), 0);
    if (bytes_sent <= 0)
    {
        perror("send");
        return(1);
    }
    _client_._first_send = false;
    if (_client_._request._which_body == FILE_BODY)
    {
        int fd = open(_client_._request._response_body_file.c_str(), O_RDONLY);
        if (fd == -1)
        {
            std::cout << "infile in the response open error" << std::endl;
            return (1);
        }
        _client_._request._response_fd = fd;
    }
    return (0);
}

int    ServManager::send_string(int client_socket, Client &_client_)
{
    std::string to_send = _client_._request._response_body;
    int bytes_sent = send(client_socket, to_send.c_str(), to_send.length(), 0);
    if (bytes_sent <= 0)
    {
        perror("send");
        return(1);
    }
    return (0);
}

int    ServManager::send_file(int client_socket, Client &_client_)
{
    int bufferSize = 1024;
    char buffer[bufferSize];
    int bytesRead = read(_client_._request._response_fd, buffer, bufferSize);
    if (bytesRead < 0)
    {
        perror("read response body file in the response");
        return (1);
    }
    else if (bytesRead > 0)
    {
        int bytes_sent = send(client_socket, buffer, bytesRead, 0);
        if (bytes_sent <= 0)
        {
            // perror("send");
            close(_client_._request._response_fd);
            return(1);
        }
    }
    else // bytesRead = 0 ==> all the file has been sent
    {
        close(_client_._request._response_fd);
        return (2);
    }
    return (0);
}

int     ServManager::close_connection(int to_close)
{
    if (FD_ISSET(to_close, &read_set))
        FD_CLR(to_close, &read_set);
    if (FD_ISSET(to_close, &write_set))
        FD_CLR(to_close, &write_set);
    if (to_close + 1 == this->max_Fd)
        this->max_Fd--;
    close(to_close);
    this->_clients_map.erase(to_close);
    return (EXIT_SUCCESS);
}