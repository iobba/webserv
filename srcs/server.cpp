#include "../Includes/server.hpp"

// initialisation of members
Server::Server()
{
    _socket = -1;
    _port = "";
    _server_name = "";
    _host = "";
    _root = "";
    _client_max_body_size = 0;
    _has_max_body_setten = false;
    _index = "";
    _autoindex = false;
    _has_autoindex_setten = false;
    this->errors_initialisation();
} 

void        Server::errors_initialisation()
{
    // set error pages 
    _error_pages[400] = "/Users/iobba/Desktop/webserv/error_pages/400.html";
    _error_pages[403] = "/Users/iobba/Desktop/webserv/error_pages/403.html";
    _error_pages[404] = "/Users/iobba/Desktop/webserv/error_pages/404.html";
    _error_pages[405] = "/Users/iobba/Desktop/webserv/error_pages/405.html";
    _error_pages[408] = "/Users/iobba/Desktop/webserv/error_pages/408.html";
    _error_pages[409] = "/Users/iobba/Desktop/webserv/error_pages/409.html";
    _error_pages[413] = "/Users/iobba/Desktop/webserv/error_pages/413.html";
    _error_pages[414] = "/Users/iobba/Desktop/webserv/error_pages/414.html";
    _error_pages[429] = "/Users/iobba/Desktop/webserv/error_pages/429.html";
    _error_pages[431] = "/Users/iobba/Desktop/webserv/error_pages/431.html";
    _error_pages[500] = "/Users/iobba/Desktop/webserv/error_pages/500.html";
    _error_pages[501] = "/Users/iobba/Desktop/webserv/error_pages/501.html";
    _error_pages[502] = "/Users/iobba/Desktop/webserv/error_pages/502.html";
    _error_pages[505] = "/Users/iobba/Desktop/webserv/error_pages/505.html";
    _error_pages[507] = "/Users/iobba/Desktop/webserv/error_pages/507.html";
    // set error messages
    _error_messages[200] = "OK";
    _error_messages[201] = "Created";
    _error_messages[202] = "Accepted";
    _error_messages[204] = "No Content";
    _error_messages[300] = "Multiple Choices";
    _error_messages[301] = "Moved Permanently";
    _error_messages[302] = "Found";
    _error_messages[307] = "Temporary Redirect";
    _error_messages[308] = "Permanent Redirect";
    _error_messages[400] = "Bad Request";
    _error_messages[403] = "Forbidden";
    _error_messages[404] = "Not Found";
    _error_messages[405] = "Method Not Allowed";
    _error_messages[408] = "Request Timeout";
    _error_messages[409] = "Conflict";
    _error_messages[413] = "Content Too Large";
    _error_messages[414] = "URI Too Long";
    _error_messages[429] = "Too Many Requests";
    _error_messages[431] = "Request Header Fields Too Large";
    _error_messages[500] = "Internal Server Error";
    _error_messages[501] = "Not Implemented";
    _error_messages[502] = "Bad Gateway";
    _error_messages[505] = "HTTP Version Not Supported";
    _error_messages[507] = "Insufficient Storage";
}

// setters
void    Server::set_socket(int sock)
{
    this->_socket = sock;
}

void    Server::set_server_info(struct sockaddr_in infos)
{
    _server_info = infos;
}

void    Server::set_port(std::string port)
{
    if (this->_port != "")
        throw CONFIGException("port is duplicated");
    if (port.length() > 5)
        throw CONFIGException("maximum port contain only 5 digits!");
    for(unsigned long i = 0; i < port.length(); i++)
    {
        if (!std::isdigit(port[i]))
            throw CONFIGException("invalid port");
    }
    unsigned long to_check = std::strtoul(port.c_str(), NULL, 10);
    if (to_check > 65535)
        throw CONFIGException("invalid port : [0--65535]");
    this->_port = port;
}

void    Server::set_server_name(std::string server_name)
{
    if (this->_server_name != "")
        throw CONFIGException("server_name is duplicated");
    this->_server_name = server_name;
}

void    Server::set_host(std::string host)
{
    if (this->_host != "")
        throw CONFIGException("host is duplicated");
    struct in_addr addr;
    if (inet_pton(AF_INET, host.c_str(), &addr) != 1)
        throw CONFIGException("invalid host");
    this->_host = host;
}

void    Server::set_root(std::string root)
{
    if (this->_root != "")
        throw CONFIGException("root is duplicated");
    if (which_type(root) != 1)
        throw CONFIGException("invalid root");
    this->_root = root;
}

void    Server::set_client_max_body_size(std::string client_max_body_size)
{
    if (this->_has_max_body_setten)
        throw CONFIGException("client_max_body_size is duplicated");
    else
        this->_has_max_body_setten = true;
    for(unsigned long i = 0; i < client_max_body_size.length(); i++)
    {
        if (!std::isdigit(client_max_body_size[i]))
            throw CONFIGException("invalid client_max_body_size");
    }
    this->_client_max_body_size = std::strtoul(client_max_body_size.c_str(), NULL, 10);
}

void    Server::set_index(std::string index)
{
    if (this->_index != "")
        throw CONFIGException("index is duplicated");
    if (which_type(index) != 0)
        throw CONFIGException("invalid index file!");
    if (access(index.c_str(), R_OK) != 0)
        throw CONFIGException("invalid index file!");
    this->_index = index;
}

void    Server::set_autoindex(std::string autoindex)
{
    if (this->_has_autoindex_setten)
        throw CONFIGException("autoindex is duplicated");
    else
        this->_has_autoindex_setten = true;
    if (autoindex == "on")
        this->_autoindex = true;
    else if ((autoindex == "off"))
        this->_autoindex = false;
    else
        throw CONFIGException("invalid autoindex [on--off]");
}

void    Server::add_error_page(std::string key, std::string value)
{
    ///// check key
    if (key.length() > 3)
        throw CONFIGException("invalid error_page [100--599]");
    for(unsigned long i = 0; i < key.length(); i++)
    {
        if (!std::isdigit(key[i]))
            throw CONFIGException("invalid error_page key[0--9]");
    }
    int key_nb = std::strtoul(key.c_str(), NULL, 10);
    if (key_nb < 100 || key_nb > 599)
        throw CONFIGException("invalid error_page [100--599]");
    ///// check value
    if (which_type(value) != 0)
        throw CONFIGException("invalid error_page file!");
    if (access(value.c_str(), R_OK) != 0)
        throw CONFIGException("invalid error_page file!");
    ///// add it to the map
    this->_error_pages[key_nb] = value;
}

void   Server::add_location(std::string _path_, std::vector<std::string> lines)
{
    Location  new_location;

    new_location.set_path(_path_);
    std::vector<std::string>::iterator  it = lines.begin();
    while (it != lines.end())
    {
        std::vector<std::string>  words = split_to_words(*it);
        if (words[0] == "root" && words.size() == 2)
            new_location.set_root(words[1]);
        else if (words[0] == "upload" && words.size() == 2)
            new_location.set_upload(words[1]);
        else if (words[0] == "index" && words.size() == 2)
            new_location.set_index(words[1]);
        else if (words[0] == "autoindex" && words.size() == 2)
            new_location.set_autoindex(words[1]);
        else if (words[0] == "client_max_body_size" && words.size() == 2)
            new_location.set_client_max_body_size(words[1]);
        else if (words[0] == "return" && words.size() > 1)
            new_location.set_return(words[1]);
        else if (words[0] == "allow_methods" && words.size() > 1)
        {
            // grab the methods
            std::vector<std::string> methooods(words.begin() + 1, words.end());
            new_location.set_allowed_methods(methooods);
        }
        else if (words[0] == "cgi_path" && words.size() == 3)
            new_location.add_cgi_paths(words[1], words[2]);
        else
            throw CONFIGException("invalid directive in location block");
        it++;
    }
    if (new_location.get_root() == "")
        new_location.set_root(this->_root);
    if (new_location._has_max_body_setten == false)
    {
        std::ostringstream oss;
        oss << this->_client_max_body_size;
        std::string __max = oss.str();
        new_location.set_client_max_body_size(__max);
    }
    this->_locations.push_back(new_location);
}

////////////////////// .........../////////////////////

// getters
const int&  Server::get_socket() const
{
    return (_socket);
}

const struct sockaddr_in&  Server::get_server_info() const
{
    return (_server_info);
}

const std::string&  Server::get_port() const
{
    return (_port);
}

const std::string&  Server::get_server_name() const
{
    return (_server_name);
}

const std::string&  Server::get_host() const
{
    return (_host);
}

const std::string&  Server::get_root() const
{
    return (_root);
}

const unsigned long&    Server::get_client_max_body_size() const
{
    return (_client_max_body_size);
}

const std::string&  Server::get_index() const
{
    return (_index);
}

bool    Server::is_autoindex() const
{
    return (_autoindex);
}

const std::string  Server::get_error_page(int key) const
{
    std::map<int, std::string>::const_iterator  it = _error_pages.find(key);
    if (it != _error_pages.end())
        return (it->second);
    return ("");
}

const std::string  Server::get_error_messages(int key) const
{
    std::map<int, std::string>::const_iterator  it = _error_messages.find(key);
    if (it != _error_messages.end())
        return (it->second);
    return ("ERROR MESSAGE NOT FOUND");
}

const std::vector<Location>&    Server::get_locations() const
{
    return (_locations);
}
