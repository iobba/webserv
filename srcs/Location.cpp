#include "../Includes/Location.hpp"

Location::Location()
{
    _path = "";
    _root = "";
    _upload = false;
    _has_upload_setten = false;
    _index = "";
    _autoindex = false;
    _has_autoindex_setten = false;
    _client_max_body_size = 0;
    _has_max_body_setten = false;
    _return = "";
}

//////////// setters //////////////

void    Location::set_path(std::string path)
{
    this->_path = path;
}

void    Location::set_root(std::string root)
{
    if (this->_root != "")
        throw CONFIGException("Location root is duplicated");
    if (which_type(root) != 1)
        throw CONFIGException("invalid Location root");
    this->_root = root;
}

void    Location::set_upload(std::string up_load)
{
    if (this->_has_upload_setten)
        throw CONFIGException("Location upload is duplicated");
    else
        this->_has_upload_setten = true;
    if (up_load == "on")
        this->_upload = true;
    else if ((up_load == "off"))
        this->_upload = false;
    else
        throw CONFIGException("Location invalid autoindex [on--off]");
}

void    Location::set_index(std::string index)
{
    if (this->_index != "")
        throw CONFIGException("Location index is duplicated");
    if (which_type(index) != 0)
        throw CONFIGException("Location invalid index file!");
    if (access(index.c_str(), R_OK) != 0)
        throw CONFIGException("Location invalid index file!");
    this->_index = index;
}

void    Location::set_autoindex(std::string autoindex)
{
    if (this->_has_autoindex_setten)
        throw CONFIGException("Location autoindex is duplicated");
    else
        this->_has_autoindex_setten = true;
    if (autoindex == "on")
        this->_autoindex = true;
    else if ((autoindex == "off"))
        this->_autoindex = false;
    else
        throw CONFIGException("Location invalid autoindex [on--off]");
}

void    Location::set_client_max_body_size(std::string client_max_body_size)
{
    if (this->_has_max_body_setten)
        throw CONFIGException("Location client_max_body_size is duplicated");
    else
        this->_has_max_body_setten = true;
    for(unsigned long i = 0; i < client_max_body_size.length(); i++)
    {
        if (!std::isdigit(client_max_body_size[i]))
            throw CONFIGException("invalid Location client_max_body_size");
    }
    this->_client_max_body_size = std::strtoul(client_max_body_size.c_str(), NULL, 10);
}

void    Location::set_return(std::string ret__urn)
{
    if (this->_return != "")
        throw CONFIGException("Location return duplicated");
    this->_return = ret__urn;
}

void    Location::set_allowed_methods(std::vector<std::string> methods)
{
    if (this->_allowed_mthds.size() != 0)
        throw CONFIGException("allow_methods is duplicated");
    int get_nb = 0;
    int delete_nb = 0;
    int post_nb = 0;
    for (unsigned long i = 0; i < methods.size(); i++)
    {
        if (methods[i] == "GET")
            get_nb++;
        else if (methods[i] == "DELETE")
            delete_nb++;
        else if (methods[i] == "POST")
            post_nb++;
        else
            throw CONFIGException("unsupported Location method");
    }
    if (get_nb > 1 || delete_nb > 1 || post_nb > 1)
        throw CONFIGException("Location method is duplicated");
    this->_allowed_mthds = methods;
}

void    Location::add_cgi_paths(std::string _ext_, std::string cgi_path_)
{
    this->_cgi_path.insert(std::make_pair(_ext_, cgi_path_));
}

/////////// getters //////////

const std::string&  Location::get_path() const
{
    return (_path);
}

const std::string&  Location::get_root() const
{
    return (_root);
}

bool Location::is_upload() const
{
    return (_upload);
}

const std::string&  Location::get_index() const
{
    return (_index);
}

bool Location::is_autoindex() const
{
    return (_autoindex);
}

const unsigned long&    Location::get_client_max_body_size() const
{
    return (_client_max_body_size);
}

const std::string&  Location::get_return() const
{
    return (_return);
}

bool Location::is_allowed_method(std::string mthd) const
{
    std::vector<std::string>::const_iterator it = std::find(_allowed_mthds.begin(), _allowed_mthds.end(), mthd);

    if (it != _allowed_mthds.end())
        return (true);
    return (false);
}

const std::map<std::string,std::string>& Location::get_cgi_paths() const
{
    return (_cgi_path);
}
