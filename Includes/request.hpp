#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "server.hpp"


class   Request
{
    private:
        ////// request //////
        std::string                         _method_str;
        std::string                         _path;
        std::string                         _version;
        std::map<std::string, std::string>  _headers_map;
        std::string                         _body;
        long unsigned int                   _body_length;
        bool                                _first_read;
        std::string                         _all_headers_str;
        std::vector<char>                   _request_vec;
        bool                                _headers_finished;
        bool                                _parsing_done;
        int                                 _error_code;
        bool                                _is_chunked;
        std::string                         _server_name;
        bool                                _body_ignored;
        int                                 _uploaded_fd;
        unsigned long int                   _body_recieved_len;



    public:
        int                                 _method;
        bool                                _reading_done;
        ////// response //////////
        Location                            _serving_location;    
        int                                 _status_code;
        std::string                         _returned_location;

        int                                 _which_body;
        std::string                         _response_body;
        std::string                         _response_headers;
        std::string                         _response_body_file;
        int                                 _response_fd;
        // cgi
        bool                                _is_cgi;
        std::string                         _query_string;
        bool                                _waiting_done;
        int                                 _cgi_pipe[2];
        pid_t                               _child_id;
        std::time_t                         _child_start;

        ////////// request ////////
        // the server which is gonna handle this request*
        Server                              server;
        Server                              _default_server;
        std::vector<Server>                 _all_servers;
        std::string                         _body_name;
        std::string                         _cleaned_body_name;
        // func
        Request();
        void                request_parser();
        void                request_analysis(char buffer[], int bytes_read);
        void                request_line_analysis(std::string line);
        std::string         create_body(std::string _ext_);
        void                parse_method();
        void                parse_path();
        void                parse_version();
        void                analyze_headers();
        void                set_perfect_server();
        int                 get_error_code();
        bool                _keep_connection();
        void                set_reading_done();
        bool                is_reading_done();
        void                print_request_parts();
        void                parse_headers();

        ///////// response ////////
        void    build_response();
        void    make_location_ready();
        int     get_matched_location();
        int     check_return();
        void    GET_handler();
        int     GET_directory();
        void    GET_file();
        void    DELETE_handler();
        void    delete_file();
        int     delete_directory();
        int     find_requested_file();
        void    set_response_headers(std::string _code_str);
        void    cgi_process(std::map<std::string,std::string>::iterator ext_found);
        void    execute_cgi(std::map<std::string,std::string>::iterator ext_found);
        void    recv_cgi_response();
        void    waiting_child();
        void    set_cgi_headers(std::string cgi_return);
        void    upload_body();
        void    uploading();



};

#endif