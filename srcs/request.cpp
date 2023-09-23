#include "../Includes/request.hpp"

Request::Request()
{
    _method_str = "";
    _method = NONE;
    _path = "";
    _version = "";
    _body = "";
    _body_length = 0;
    _first_read = true;
    _headers_finished = false;
    _parsing_done = false;
    _reading_done = false;
    _error_code = 0;
    _is_chunked = false;
    _server_name = "";
    _body_ignored = false;
    _body_name = "";
    _cleaned_body_name = "";
    _tmp_response = "hello world!";
    _all_headers_str = "";
    _status_code = 200;
    _returned_location = "";
    _response_body = "";
    _which_body = NONE;
    _headers_map.clear();
}

void     Request::request_analysis()
{
    try
    {
        request_parser();
        std::cout << "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhere\n";
        if (this->_reading_done)
            build_response();
    }
    catch(std::exception & e)
    {
        this->_reading_done = true;
        _status_code = std::strtoul(e.what(), NULL, 16);
        if (_status_code >= 400)
        {
            this->_response_body_file = this->_request_handler.get_error_page(_status_code);
            this->_which_body = FILE_BODY;
        }
        else if (_status_code >= 300)
        {
            std::string redirect_location = this->_response_body_file;
            this->_which_body = NONE;
        }
        set_response_headers(e.what());
    }
    print_request_parts();
    // which_method();
    // ...
}

void    Request::set_reading_done()
{
    this->_reading_done = true;
}

void Request::which_method()
{
    // ...
}

void    Request::request_parser()
{
    std::string line;
    std::vector<char>::iterator it_find;
    std::vector<char>::iterator char_it;
    if (this->_headers_finished == false)
    {
        int headers_end = find_in_vector(this->_request_vec, "\r\n\r\n");
        if (headers_end != -1)
        {
            // grab all headers's lines with their "\r\n"
            this->_all_headers_str = std::string(this->_request_vec.begin(), this->_request_vec.begin() + headers_end + 2);
            parse_headers();
            if (this->_body_ignored)
            {
                this->_reading_done = true;
                return ;
            }
            // create a file to store the body
            this->_body_name = create_body(".txt");
            if (this->_is_chunked)
                this->_cleaned_body_name = create_body(".jpeg");
            std::ofstream o_body_file(this->_body_name.c_str(), std::ios::app);
            if (!o_body_file.is_open())
                throw HTTPException(500);
            // pass all headers + the empty line
            char_it = this->_request_vec.begin() + headers_end + 4;
            for (; char_it != this->_request_vec.end(); ++char_it)
            {
                o_body_file.put(*char_it);
            }
            o_body_file.close();
            // watching the body length
            watch_body_len();
            this->_request_vec.clear();
        }
    }
    else
    {
        std::ofstream o_body_file(this->_body_name.c_str(), std::ios::app);
        if (!o_body_file.is_open())
            throw HTTPException(500);
        char_it = this->_request_vec.begin();
        for (; char_it != this->_request_vec.end(); ++char_it)
        {
            o_body_file.put(*char_it);
        }
        o_body_file.close();
        // watching the body length
        watch_body_len();
        this->_request_vec.clear();
    }
}

void    Request::parse_headers()
{
    std::istringstream request_stream(this->_all_headers_str);
    std::string line;

    std::getline(request_stream, line);
    request_line_analysis(line.substr(0, line.find("\r")));
    while (std::getline(request_stream, line))
    {
        size_t pos = line.find(':');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2); // Skip the ': ' part
            value = value.substr(0, value.find("\r"));
            this->_headers_map[key] = value; // Store in the map
        }
    }
    this->_headers_finished = true;
    this->analyze_headers(); // get some infos from headers
}

void    Request::watch_body_len()
{
    if (get_file_len(_body_name) > this->_request_handler.get_client_max_body_size())
        throw HTTPException(413);
    if (this->_is_chunked)
    {
        long long  body_end = look_for_word(_body_name, std::string("\r\n0\r\n"));
        if (body_end != -1)
        {
            //std::cout << "\n\nHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << "\n\n";
            // clean the body
            this->_body_length = get_file_len(_cleaned_body_name);
            if (clean_chunked_body())
                this->_reading_done = true;
            else
            {
                // clear the cleaned body file
                std::ofstream file(this->_cleaned_body_name.c_str(), std::ios::out | std::ios::trunc);
                if (!file.is_open())
                    throw HTTPException(500);
                file.close();
            }
        }
    }
    else
    {
        if (this->_body_length == get_file_len(_body_name))
            this->_reading_done = true;
        if (this->_body_length < get_file_len(_body_name))
            throw HTTPException(400);
    }
}

std::string    Request::create_body(std::string _ext_)
{
    // Get the current time in milliseconds
    std::time_t currentTime = std::time(NULL);
    unsigned int milliseconds = static_cast<unsigned int>(currentTime);
    static long unsigned int num_files = 0;
    num_files++;
    long unsigned int randooooooomNum = milliseconds + num_files;
    std::stringstream ss;
    ss << randooooooomNum;
    std::string file_name = "/nfs/sgoinfre/goinfre/Perso/iobba/" + ss.str() + _ext_;
    // std::cout << "heeeeeeeeeere = " << file_name << std::endl;
    std::ofstream o_body_file(file_name.c_str());
    if (!o_body_file.is_open())
        throw HTTPException(500);
    o_body_file.close();
    return (file_name);
}

long long     Request::look_for_word(std::string file__name, std::string to_find)
{
    std::ifstream file(file__name.c_str());
    if (!file.is_open())
        throw HTTPException(500);
    char c;
    size_t _index = 0;
    long long _pos = 0;
    while (file.get(c))
    {
        if (c == to_find[_index])
        {
            _index++;
            if (_index == to_find.length())
            {
                file.close();
                return (_pos);
            }
        }
        else
            _index = 0;
        _pos++;
    }
    file.close();
    return (-1);
}

void    Request::analyze_headers()
{
    // parse the headers , i need to take 4 infos from it
    std::map<std::string, std::string>::iterator it1 = _headers_map.find("Host");
    if (it1 != _headers_map.end())
        this->_server_name = it1->second;
    else
        throw HTTPException(400);
    std::map<std::string, std::string>::iterator it2 = _headers_map.find("Transfer-Encoding");
    std::map<std::string, std::string>::iterator it3 = _headers_map.find("Content-Length");
    if (it2 != _headers_map.end())
    {
        if (it2->second != "chunked")
            throw HTTPException(501);
        this->_is_chunked = true;
    }
    else if (it3 != _headers_map.end())
    {
        this->_body_length = std::strtoll(it3->second.c_str(), NULL, 10);
    }
    else if (this->_method == POST)
        throw HTTPException(400);
}

int    Request::clean_chunked_body()
{
    std::ifstream inputFile(this->_body_name.c_str()); // Open the input file for reading
    std::ofstream outputFile(this->_cleaned_body_name.c_str()); // Open the input file for reading

    if (!inputFile.is_open() || !inputFile.is_open())
    {
        std::cerr << "Failed to open the file." << std::endl;
        throw HTTPException(500);
    }

    char ch;
    bool is_in_size_line = true;
    std::string     size_line;
    long unsigned int chunk_len;
    while (inputFile.get(ch))
    {
        if (is_in_size_line)
        {
            if (ch != '\r')
                size_line.push_back(ch);
            else
            {
                chunk_len = std::strtoul(size_line.c_str(), NULL, 16);
                size_line.clear();
                is_in_size_line = false;
                if (!inputFile.get(ch))
                    break ;
                if (chunk_len == 0)
                {
                    // end of body
                    inputFile.close();
                    outputFile.close();
                    return (1);
                }

            }
        }
        else
        {
            if (chunk_len == 0)
            {
                if (!inputFile.get(ch))
                    break ;
                is_in_size_line = true;
            }
            else
            {
                outputFile.put(ch);
                chunk_len--;
            }
        }
    }
    inputFile.close();
    outputFile.close();
    return (0);
}

void    Request::print_request_parts()
{
    // printing the request
    std::cout << "method == " << _method_str << std::endl; 
    std::cout << "path == " << _path << std::endl; 
    std::cout << "version == " << _version << std::endl;
    if (this->_headers_finished)
    {
        std::cout << "headers :" << std::endl;
        std::map<std::string, std::string>::iterator it_map = _headers_map.begin();
        while (it_map != _headers_map.end())
        {
            std::cout << it_map->first << " ==> " << it_map->second << std::endl;
            it_map++;
        }
        // if (_body_name != "")
        // {
        //     std::ifstream i_body_file(this->_body_name.c_str());
        //     std::cout << "body : [";
        //     std::string line;
        //     while (std::getline(i_body_file, line))
        //     {
        //         std::cout << line;
        //     }
        //     std::cout << "]\n";
        //     i_body_file.close();
        // }
    }
}

void     Request::request_line_analysis(std::string line)
{
    std::istringstream request_line_stream(line);
    request_line_stream >> this->_method_str >> this->_path >> this->_version;

    this->parse_version();
    this->parse_path();
    this->parse_method();
}

void    Request::parse_method()
{
    if (this->_method_str.compare("GET") == 0)
        this->handle_GET();
    else if (this->_method_str.compare("DELETE") == 0)
        this->handle_DELETE();
    else if (this->_method_str.compare("POST") == 0)
        this->handle_POST();
    else
    {
        this->_method = NONE;
        throw HTTPException(501);
    }
}

void    Request::handle_GET()
{
    this->_method = GET;
    this->_method_str = "GET";
    this->_body_ignored = true;
    // ...
}

void    Request::handle_DELETE()
{
    this->_method = DELETE;
    this->_method_str = "DELETE";
    this->_body_ignored = true;
    // ...
}

void    Request::handle_POST()
{
    this->_method = POST;
    this->_method_str = "POST";
    this->_body_ignored = false;
    // ...
}

void    Request::parse_path()
{
    if (this->_path.length() > MAX_URI_length)
        throw HTTPException(414);
    size_t  two_points = this->_path.find("..");
    if (two_points != std::string::npos)
        throw HTTPException(403);
    std::string allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for (long unsigned int i = 0; i < this->_path.length(); i++)
    {
        size_t found = allowed_chars.find(_path[i]);
        if (found == std::string::npos)
            throw HTTPException(400);
    }
}

void    Request::parse_version()
{
    if (this->_version.compare("HTTP/1.1"))
        throw HTTPException(505);
}

bool    Request::is_reading_done()
{
    return (this->_reading_done);
}

void    Request::clearing_request()
{
    this->_method_str = "";
    this->_method = NONE;
    this->_path = "";
    this->_version = "";
    this->_headers_map.clear();
    this->_body = "";
    this->_body_length = 0;
    this->_first_read = true;
    this->_headers_finished = false;
    this->_parsing_done = false;
    this->_reading_done = false;
    this->_error_code = 0;
    this->_is_chunked = false;
    this->_server_name = "";
    this->_body_ignored = false;
    this->_body_name = "";
    this->_cleaned_body_name = "";
    this->_tmp_response = "hello world!";
    this->_all_headers_str = "";
    this->_status_code = 200;
    this->_returned_location = "";
    this->_response_body = "";
    this->_which_body = NONE;
}

bool    Request::_keep_connection()
{
    std::map<std::string, std::string>::iterator it = _headers_map.find("Connection");
    if (_headers_finished && it != _headers_map.end() && it->second == "keep-alive")
    {
        return (true);
    }
    return (false);
}

void    Request::fill_request_vec(char buffer[], int bytes_read)
{
    int i = 0;
    while (i < bytes_read)
    {
        this->_request_vec.push_back(buffer[i]);
        i++;
    }
}

int     Request::get_error_code()
{
    return (this->_error_code);
}

long unsigned int     get_file_len(std::string file__name)
{
    std::ifstream file(file__name.c_str());
    if (!file.is_open())
        throw HTTPException(500);
    file.seekg(0, std::ios::end); // Move to the end of the file
    long unsigned int   __len = file.tellg(); // Get the current position (file size)
    file.close();
    return (__len);
}

int     find_in_vector(std::vector<char> __vec, std::string to_find)
{
    long unsigned int i = 0;
    long unsigned int j = 0;
    while (i < __vec.size())
    {
        if (__vec[i] == to_find[j])
        {
            j++;
            if (j == to_find.length())
                return (i - (j - 1));
        }
        else
            j = 0;
        i++;
    }
    return (-1);
}

///////////// response //////////////////
void    Request::build_response()
{
    if (get_matched_location() == 0)
        throw HTTPException(404);
    // check return
    if (check_return())
        throw HTTPException(301);
    // is allowed method
    if (this->_serving_location.is_allowed_method(this->_method_str) == false)
        throw HTTPException(405);
    // max body size
    if (get_file_len(this->_body_name) > this->_serving_location.get_client_max_body_size())
        throw HTTPException(413);
    // specify which method
    if (this->_method == GET)
        GET_handler();
    // else if (this->_method == DELETE)
    //     DELETE_handler();
    // else
    //     POST_handler();
    std::cout << "\nWTTTTTTTTTTTTTTTTTTTTTTTTTTTTTF\n\n";
}

int Request::get_matched_location()
{
    unsigned long int more_specified = 0;
    std::vector<Location>  locations = _request_handler.get_locations();        
    unsigned long int i = 0;
    while (i < locations.size())
    {
        size_t found_location_path = this->_path.find(locations[i].get_path());
        if (found_location_path == 0)
        {
            if (this->_path.length() == locations[i].get_path().length())
            {
                if (locations[i].get_path().length() > more_specified)
                {
                    this->_serving_location = locations[i];
                    more_specified = locations[i].get_path().length();
                }
            }
            else if (this->_path.length() > locations[i].get_path().length())
            {
                if (this->_path[locations[i].get_path().length()] == '/' 
                    || locations[i].get_path() == "/")
                {
                    if (locations[i].get_path().length() > more_specified)
                    {
                        this->_serving_location = locations[i];
                        more_specified = locations[i].get_path().length();
                    }
                }
            }
        }
        i++;
    }
    return (more_specified);
}

int     Request::check_return()
{
    if (_serving_location.get_return() != "")
    {
        this->_returned_location = _serving_location.get_return();
        this->_status_code = 301;
        return (1);
    }
    return (0);
}

void    Request::GET_handler()
{
    int path_type = find_requested_file();
    if (path_type == 1)
    {
        if (GET_directory())
            GET_file();
    }
    else if (path_type == 0)
        GET_file();
    else 
        throw HTTPException(404);
}

int    Request::find_requested_file()
{
    std::string rooooooot = this->_serving_location.get_root();
    if (rooooooot[rooooooot.length() - 1] != '/')
        rooooooot.push_back('/');
    std::string only_path = this->_path.substr(0, this->_serving_location.get_path().length());
    if (only_path == "")
        this->_response_body_file = rooooooot;
    else 
    {
        if (only_path[0] == '/')
            only_path.erase(0, 1);
        this->_response_body_file = rooooooot + only_path;
    }
    // check if regular file or directory or something else(error)
    return (which_type(this->_response_body_file));
}

int    Request::GET_directory()
{
    if (this->_response_body_file[this->_response_body_file.length() - 1] != '/')
    {
        this->_path.push_back('/');
        this->_response_body_file = this->_path;
        throw HTTPException(301);
    }
    if (this->_serving_location.get_index() != "")
    {
        // find if it is necessery to concate the index's path with the root
        this->_response_body_file = this->_serving_location.get_index();
        return (1);
    }
    else
    {
        if (this->_serving_location.is_autoindex())
        {
            // generate an HTML page listing the contents of a directory
            DIR* dir = opendir(this->_response_body_file.c_str());
            if (dir == NULL)
            {
                std::cerr << "Failed to open directory, in opendir(autoindex)" << std::endl;
                throw HTTPException(500);
            }
            std::string html_content  = "<html><body><h1>Directory Listing</h1><ul>";
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL)
            {
                // skip . and ..
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                html_content += "<li><a href=\"" + std::string(entry->d_name) + "\">" + std::string(entry->d_name) + "</a></li>";
            }
            html_content += "</ul></body></html>";
            closedir(dir);
            this->_response_body = html_content;
            this->_which_body = STR_BODY;
            throw HTTPException(200);
        }
        else
            throw HTTPException(403);
    }
    return (0);
}

void    Request::GET_file()
{
    std::string file_ext = get_file_extention(this->_response_body_file);
    std::map<std::string,std::string> ext_map = this->_serving_location.get_cgi_paths();
    std::map<std::string,std::string>::iterator ext_found = ext_map.find(file_ext);
    if (ext_found != ext_map.end())
    {
        // run cgi
        // and the _status_code depend on the cgi
        this->_which_body = NONE; // just for the flow
        throw HTTPException(200);
    }
    else
    {
        this->_which_body = FILE_BODY;
        throw HTTPException(200);
    }
}

void    Request::set_response_headers(std::string _code_str)
{
    this->_response_headers = "";
    // status line 
    _response_headers = std::string("HTTP/1.1") + " ";
    _response_headers += _code_str + " ";
    _response_headers += _request_handler.get_error_messages(_status_code) + "\r\n";
    // other headers ...
    // Content Type or Location and Content length
    if (this->_status_code >= 300 && this->_status_code < 400)
    {
        _response_headers = std::string("Location:") + " ";
        _response_headers = this->_returned_location + "\r\n";
        _response_headers += std::string("Content-Length:") + " ";
        _response_headers += std::string("0") + "\r\n";
    }
    else if (this->_status_code == 200 && this->_which_body == STR_BODY)
    {
        _response_headers = std::string("Content-Type:") + " ";
        _response_headers = std::string("text/html") + "\r\n";
        std::stringstream ss;
        ss << this->_response_body.length();
        _response_headers += std::string("Content-Length:") + " ";
        _response_headers += ss.str() + "\r\n";
    }
    else if (this->_which_body == FILE_BODY)
    {
        std::string ext;
        size_t point_pos = this->_response_body_file.rfind(".");

        if (point_pos != std::string::npos)
            ext = this->_response_body_file.substr(point_pos);
        else
            ext = ".txt"; 
        _response_headers = std::string("Content-Type:") + " ";
        _response_headers = get_conetnt_type(ext) + "\r\n";
        std::stringstream ss;
        ss << get_file_len(this->_response_body_file);
        _response_headers += std::string("Content-Length:") + " ";
        _response_headers += ss.str() + "\r\n";
    }
    else
    {
        _response_headers += std::string("Content-Length:") + " ";
        _response_headers += std::string("0") + "\r\n";
    }
    // Date if needed
    if (true)
    {
        std::time_t currentTime;
        std::time(&currentTime);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&currentTime));
        // to string
        std::string date_str(buffer);
        _response_headers = std::string("Date:") + " ";
        _response_headers = date_str + "\r\n";
    }
}
