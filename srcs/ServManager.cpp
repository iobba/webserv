#include "../Includes/ServManager.hpp"

ServManager::ServManager() : max_Fd(0) {}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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
    print_esrvers_map();
    std::cout << "Server is listening for connections on port 8008..." << std::endl;
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
    while (it != this->_servers_map.end())
    {
        std::cout << "server " << i << " :\n";
        std::cout << "         + socket -------> " << it->first << std::endl;
        std::cout << "         + server_name --> " << it->second.get_server_name() << std::endl;
        it++;
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

void    ServManager::configure_servers(std::vector<Server> servers_vec)
{
    std::vector<Server>::iterator it = servers_vec.begin();
    while (it != servers_vec.end())
    {
        Server              server = *it;
        int                 server_fd;
        struct sockaddr_in  address;
        int                 yes = 1;

        // Creating socket file descriptor
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
            throw SERVER_Exception("failed to bind!!");
        }
        server.set_socket(server_fd);
        server.set_server_info(address);
        if (listen(server_fd, BACKLOG) == -1)
            throw SERVER_Exception("failed to listen !!");
        this->max_Fd = std::max(this->max_Fd, server_fd + 1);
        this->_servers_map.insert(std::make_pair(server.get_socket(), server));
        it++;
    }
}

void    ServManager::setup_sets()
{
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);

    // add all server_sockets in the read_set
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

            // we need to store more data about the client
            std::cout << "New connection accepted." << std::endl;
            _new_client.set_socket(cl_socket);
            // set the server which is gonna handle this request
            _new_client._request._request_handler = it->second;
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
            char buffer[1000000]; // 1024
            memset(buffer, 0, sizeof(buffer));
            // std::cout << "recv fd = "  << client_socket << std::endl;
            ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            // std::string body(buffer, bytes_read);
            // std::string b  += body;
            if (bytes_read == -1)
            {
                perror("recv");
                it++;
                close_connection(client_socket);
                continue ;
            }
            else if (bytes_read == 0)
            {
                it++;
                close_connection(client_socket);
                continue ;
            }
            else
            {
                if (!is_favicon_ico(buffer))
                {
                    // std::cout << "Received request from client:\n" << bytes_read << std::endl;
                    // std::cout << "-----------------------------------------------\n";
                    // std::cout << "buffer == " << buffer << std::endl;
                    std::cout << "9999999999999999999999999999999999999999999\n";
                    it->second._request.request_analysis(buffer, bytes_read);
                    if (it->second._request._reading_done)
                    {
                        // build the response ...
                        std::cout << "\n\n##########################################\n\n";
                        FD_CLR(client_socket, &read_set);
                        FD_SET(client_socket, &write_set);
                    }
                }
                ++it;
            }
        }
        else 
            ++it;
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
            // std::cout << "response fd = " << client_socket << std::endl; 
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
                //std::cout <<  "aaaaaaaaa\n";
                if (it->second._request._status_code == 677173)// CGI
                    continue ;
                //std::cout <<  "bbbbbbbbb\n";
                if (it->second._request._status_code >= 400)
                {
                    it->second._request._response_body_file = it->second._request._request_handler.get_error_page(it->second._request._status_code);
                    it->second._request._which_body = FILE_BODY;
                }
                else if (it->second._request._status_code >= 300)
                {
                    std::string redirect_location = it->second._request._response_body_file;
                    it->second._request._which_body = NONE;
                }
                // std::cout << "gggggggggggggg\n";
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
    if (_client_._request._is_cgi)
    {
        if (_client_._request._waiting_done == false)
            _client_._request.waiting_child();
        if (_client_._request._waiting_done == false)
            return(0);
    }
    bool sending_done = false;
    // std::cout << "response fd = " << client_socket << std::endl;
    if (_client_._first_send)
    {
        // send headers first
        std::string headers = _client_._request._response_headers;
        if (send(client_socket, headers.c_str(), headers.length(), 0) == -1)
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
                throw HTTPException(500);
            }
            _client_._request._response_fd = fd;
        }
        // send the body
    }
    else if (_client_._request._which_body == STR_BODY)
    {
        std::string to_send = _client_._request._response_body;
        if (send(client_socket, to_send.c_str(), to_send.length(), 0) == -1)
        {
            perror("send");
            return(1);
        }
        sending_done = true;
    }
    else if (_client_._request._which_body == FILE_BODY)
    {
        //std::cout << "response headers :\n[ " << _client_._request._response_headers << "]\n";
        //std::cout << "11111111111111111\n"; 
        int bufferSize = 1024;
        char buffer[bufferSize];
        int bytesRead = read(_client_._request._response_fd, buffer, bufferSize);
        if (bytesRead < 0)
        {
            perror("read response body file in the response");
            throw HTTPException(500);
        }
        else if (bytesRead > 0)
        {
            int bytes_sent = send(client_socket, buffer, bytesRead, 0);
            if (bytes_sent < 0)
            {
                perror("send");
                close(_client_._request._response_fd);
                return(1);
            }
            else
                _client_._sending_offset += bytes_sent;
            if (_client_._sending_offset == get_file_len(_client_._request._response_body_file))
            {
                sending_done = true;
                close(_client_._request._response_fd);
            }
        }
        else
        {
            sending_done = true;
            close(_client_._request._response_fd);
        }
        //std::cout << "number of sent data is : " << _client_._sending_offset << std::endl;
        //std::cout << "2222222222222222222\n"; 
    }
    else
        sending_done = true;
    if (sending_done)
    {
        // clear the request and the response ...
        // setup_after_sending(client_socket, _client_);
        return (1);
    }
    return (0);
}

void    ServManager::setup_after_sending(int client_socket, Client &_client_)
{
    _client_._sending_offset = 0;
    _client_._first_send = true;
    Request  new_request;
    new_request._request_handler = _client_._request._request_handler;
    _client_._request = new_request;
    std::cout << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n";
    FD_SET(client_socket, &read_set);
    FD_CLR(client_socket, &write_set);
}

int     ServManager::close_connection(int to_close)
{
    if (FD_ISSET(to_close, &read_set))
        FD_CLR(to_close, &read_set);
    if (FD_ISSET(to_close, &write_set))
        FD_CLR(to_close, &write_set);
    std::cout << "\nclose = [ " << to_close << " ]\n" ;
    if (to_close + 1 == this->max_Fd)
        this->max_Fd--;
    close(to_close);
    this->_clients_map.erase(to_close);
    return (EXIT_SUCCESS);
}