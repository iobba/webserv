#include "../Includes/parseconfig.hpp"

ParseConfig::ParseConfig() : __content("") {}

void    ParseConfig::parsing_config(std::string  config_file)
{
    read_config(config_file);
    clean_content();

    std::vector<std::string>    servers_scopes = get_server_scopes();
    // create servers
    std::vector<std::string>::iterator it = servers_scopes.begin();
    for (; it != servers_scopes.end(); it++)
    {
        add_server(*it);
    }
    if (this->__servers.size() == 0)
        throw CONFIGException("No server provided");
    // check for duplicate
    std::vector<Server>::iterator it_1 = this->__servers.begin();
    while (it_1 != this->__servers.end())
    {
        std::vector<Server>::iterator it_2 = it_1 + 1;
        while (it_2 != this->__servers.end())
        {
            if (it_2->get_host() == it_1->get_host()
                && it_2->get_port() == it_1->get_port()
                && it_2->get_server_name() == it_1->get_server_name())
                throw CONFIGException("two servers duplicated [host--port--server_name]");
            it_2++;
        }
        it_1++;
    }
}

void    ParseConfig::read_config(std::string __path)
{
    if (which_type(__path))
        throw CONFIGException("invalid config file!");
    std::ifstream file(__path.c_str());
    if (file.is_open())
    {
         std::stringstream file_contents_stream;

        file_contents_stream << file.rdbuf();
        file.close();
        this->__content.clear();
        this->__content = file_contents_stream.str();
        if (this->__content.empty())
            throw CONFIGException("config file is empty!");
    }
    else
        throw CONFIGException("invalid config file!");
}

void    ParseConfig::clean_content()
{
    // comments
    while (1)
    {
        size_t found_hash = this->__content.find('#');
        if (found_hash != std::string::npos)
        {
            size_t found_nl = this->__content.find('\n', found_hash);
            if (found_nl != std::string::npos)
                this->__content = this->__content.erase(found_hash, found_nl - found_hash);
            else
                this->__content = this->__content.erase(found_hash, this->__content.length() - found_hash);
        }
        else
            break ;
    }
}



std::vector<std::string>    ParseConfig::get_server_scopes()
{
    std::vector<std::string>  _scopes;
    int _scopes_nb = 0;
    for (long unsigned int i = 0; i < __content.length(); i++)
    {
        // pass whitespaces
        while (i < __content.length() && std::isspace(__content[i]))
            i++;
        if (i == __content.length())
            break ;
        // pass server keyword
        if (__content.length() <= i + 6 || __content.substr(i, 6) != "server")
            throw CONFIGException("invalid config file");
        i += 6;
        // pass whitespaces
        while (i < __content.length() && std::isspace(__content[i]))
            i++;
        if (i == __content.length())
            throw CONFIGException("uncomplited server scope");
        // pass "{"
        if (__content.length() <= i || __content[i] != '{')
            throw CONFIGException("server should start with \'{\'");
        i += 1;
        // pass whitespaces
        while (i < __content.length() && std::isspace(__content[i]))
            i++;
        if (i == __content.length())
            throw CONFIGException("uncomplited server scope");
        int start = i;
        int left_curly_brace_nb = 1;
        // find end of server scope
        while (i < __content.length())
        {
            if (__content[i] == '{')
                left_curly_brace_nb++;
            else if (__content[i] == '}')
                left_curly_brace_nb--;
            if (left_curly_brace_nb == 0)
                break ;
            i++;
        }
        if (i == __content.length())
            throw CONFIGException("uncomplited server scope");
        int end = i;
        if (start == end)
            throw CONFIGException("empty server scope");
        // add a server scope
        _scopes.push_back(__content.substr(start, end - start));
        _scopes_nb++;
    }
    if (_scopes_nb == 0)
        throw CONFIGException("No server provided");
    return (_scopes);
}

void  ParseConfig::add_server(std::string server_scope)
{
    Server  new_server;

    std::vector<std::string>    lines = split_to_lines(server_scope);
    std::vector<std::string>::iterator  it = lines.begin();
    while (it != lines.end())
    {
        std::vector<std::string>  words = split_to_words(*it);
        if (words[0] == "listen" && words.size() == 2)
            new_server.set_port(words[1]);
        else if (words[0] == "server_name" && words.size() == 2)
            new_server.set_server_name(words[1]);
        else if (words[0] == "host" && words.size() == 2)
            new_server.set_host(words[1]);
        else if (words[0] == "root" && words.size() == 2)
            new_server.set_root(words[1]);
        else if (words[0] == "client_max_body_size" && words.size() == 2)
            new_server.set_client_max_body_size(words[1]);
        else if (words[0] == "index" && words.size() == 2)
            new_server.set_index(words[1]);
        else if (words[0] == "autoindex" && words.size() == 2)
            new_server.set_autoindex(words[1]);
        else if (words[0] == "error_page" && words.size() == 3)
            new_server.add_error_page(words[1], words[2]);
        else if (words[0] == "location" && words.size() == 2)
        {
            std::string paaaaath = words[1];
            it++;
            if (it == lines.end())
                throw CONFIGException("uncomplited Location block");
            std::vector<std::string>  words1 = split_to_words(*it);
            if (words1[0] != "{" || words1.size() > 1)
                throw CONFIGException("invalid Location syntax");
            std::vector<std::string>    location_lines;
            while (1)
            {
                it++;
                if (it == lines.end())
                    throw CONFIGException("uncomplited Location block");
                std::vector<std::string>  words2 = split_to_words(*it);
                if (words2[0] == "}")
                {
                    if (words2.size() == 1)
                        break ;
                    else
                        throw CONFIGException("nothing should be after location [ }... ]");
                }
                else
                    location_lines.push_back(*it);
            }
            new_server.add_location(paaaaath, location_lines);
        }
        else
            throw CONFIGException("invalid directive in server block");
        it++;
    }
    if (new_server.get_port() == "")
        throw CONFIGException("the server should have a port");
    // if (new_server.get_server_name() == "")
    //     throw CONFIGException("the server should have a server_name");
    if (new_server.get_host() == "")
        new_server.set_host("127.0.0.1");
    if (new_server.get_root() == "")
        throw CONFIGException("the root directive is needed in the server block");
    if (new_server._has_max_body_setten == false)
        new_server.set_client_max_body_size("3000000");
    if (new_server.get_index() == "")
        new_server.set_index("/Users/iobba/Desktop/our_root/index.html");
    // at the end
    this->__servers.push_back(new_server);
}

std::vector<std::string>    split_to_lines(std::string all_scope)
{
    std::vector<std::string>    lines;
    std::istringstream iss(all_scope);

    std::string line;
    while (std::getline(iss, line))
    {
        for (unsigned long i = 0; i < line.length(); i++)
        {
            if (!std::isspace(line[i]))
            {
                lines.push_back(line);
                break ;
            }
        }
    }
    return (lines);
}

std::vector<std::string>    split_to_words(std::string line)
{
    std::vector<std::string>  words;
    unsigned long i = 0;
    while (i < line.length())
    {
        // pass white spaces
        while (i < line.length() && std::isspace(line[i]))
            i++;
        if (i == line.length())
            break ;
        // word
        unsigned long start = i;
        while (i < line.length() && !std::isspace(line[i]))
            i++;
        unsigned long end = i;
        words.push_back(line.substr(start, end - start)); 
    }
    if (words.size() < 1)
        throw CONFIGException("invalid directive : at least two words");
    return (words);
}

const std::vector<Server>& ParseConfig::get_servers()
{
    return (this->__servers);
}

void    ParseConfig::print_servers()
{
    std::vector<Server>::iterator it_1 = this->__servers.begin();
    int i = 1;
    while (it_1 != this->__servers.end())
    {
        std::cout << "SERVER " << i << " :\n";
        std::cout << "          port ------------------> " << it_1->get_port() << std::endl;
        std::cout << "          server_name -----------> " << it_1->get_server_name() << std::endl;
        std::cout << "          host ------------------> " << it_1->get_host() << std::endl;
        std::cout << "          root ------------------> " << it_1->get_root() << std::endl;
        std::cout << "          client_max_body_size --> " << it_1->get_client_max_body_size() << std::endl;
        std::cout << "          index -----------------> " << it_1->get_index() << std::endl;
        std::cout << "          autoindex -------------> " << it_1->is_autoindex() << std::endl;
        it_1++;
        i++;
    }
}