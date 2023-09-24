#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "server.hpp"


class   Request
{
    private:
        ////// request //////
        std::string                         _method_str;
        int                                 _method;
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
        bool                                _reading_done;
        int                                 _error_code;
        bool                                _is_chunked;
        std::string                         _server_name;
        bool                                _body_ignored;



    public:
        ////// response //////////
        Location                            _serving_location;    
        int                                 _status_code;
        std::string                         _returned_location;

        int                                 _which_body;
        std::string                         _response_body;
        std::string                         _response_headers;
        std::string                         _response_body_file;


        ////////// request ////////
        // the server which is gonna handle this request
        // ******************************************************
        Server                              _request_handler;//**
        // ******************************************************
        void                fill_request_vec(char buffer[], int bytes_read);
        std::string                         _body_name;
        std::string                         _cleaned_body_name;
        std::string                         _tmp_response; // tmp
        Request();
        void                request_parser();
        void                request_analysis();
        void                request_line_analysis(std::string line);
        long long           look_for_word(std::string file__name, std::string to_find);
        std::string         create_body(std::string _ext_);
        void                watch_body_len();
        void                parse_method();
        void                parse_path();
        void                parse_version();
        void                analyze_headers();
        int                 clean_chunked_body();
        void                handle_GET();
        void                handle_DELETE();
        void                handle_POST();
        int                 get_error_code();
        bool                _keep_connection();
        void                set_reading_done();
        bool                is_reading_done();
        void                which_method();
        void                print_request_parts();
        void                parse_headers();

        ///////// response ////////
        void    build_response();
        int     get_matched_location();
        int     check_return();
        void    GET_handler();
        int     GET_directory();
        void    GET_file();
        void    DELETE_handler();
        void    POST_handler();
        int     find_requested_file();
        void    set_response_headers(std::string _code_str);


};

#endif