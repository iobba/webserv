#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "webserv.hpp"

class   Location
{
    private:
        std::string                         _path;
        std::string                         _root;
        bool                                _upload;
        std::string                         _index;
        bool                                _autoindex;
        unsigned long                       _client_max_body_size;
        std::string                         _return;
        std::vector<std::string>            _allowed_mthds;
        std::map<std::string,std::string>   _cgi_path;

    public:
        ////// setters
        Location();
        void    set_path(std::string path);
        void    set_root(std::string root);
        void    set_upload(std::string up_load);
        void    set_index(std::string index);
        void    set_autoindex(std::string autoindex);
        void    set_client_max_body_size(std::string client_max_body_size);
        void    set_return(std::string ret__urn);
        void    set_allowed_methods(std::vector<std::string> methods);
        void    add_cgi_paths(std::string _ext_, std::string cgi_path_);
        ////// getters

        bool                                        _has_autoindex_setten;
        bool                                        _has_max_body_setten;
        bool                                        _has_upload_setten;
        const std::string&                          get_path() const;
        const std::string&                          get_root() const;
        bool                                        is_upload() const;
        const std::string&                          get_index() const;
        bool                                        is_autoindex() const;
        const unsigned long&                        get_client_max_body_size() const;
        const std::string&                          get_return() const;
        bool                                        is_allowed_method(std::string mthd) const;
        const std::map<std::string,std::string>&    get_cgi_paths() const;

        
};

#endif