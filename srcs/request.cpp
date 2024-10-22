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
    _all_headers_str = "";
    _status_code = 200;
    _returned_location = "";
    _response_body = "";
    _response_body_file = "";
    _which_body = NONE;
    _response_fd = -1;
    _is_cgi = false;
    _body_recieved_len = 0;
    _waiting_done = false;
    _child_id = 0;
    _child_start = 0;
    _query_string = "";
}

void     Request::request_analysis(char buffer[], int bytes_read)
{
    std::string request_content(buffer, bytes_read);
    this->_body += request_content;
    try
    {
        request_parser();
        if (this->_reading_done)
            build_response();
    }
    catch(std::exception & e)
    {
        this->_reading_done = true;
        _status_code = std::strtoul(e.what(), NULL, 10);
        if (_status_code == 677173)// CGI
            return ;
        if (_status_code >= 400)
        {
            this->_response_body_file = this->server.get_error_page(_status_code);
            this->_which_body = FILE_BODY;
        }
        else if (_status_code >= 300)
        {
            std::string redirect_location = this->_response_body_file;
            this->_which_body = NONE;
        }
        set_response_headers(e.what());
    }
}

void    Request::set_reading_done()
{
    this->_reading_done = true;
}

void    Request::request_parser()
{
    if (this->_headers_finished == false)
    {
        size_t headers_end = this->_body.find("\r\n\r\n");
        if (headers_end != std::string::npos)
        {
            // grab all headers's lines with their "\r\n"
            this->_all_headers_str = this->_body.substr(0, headers_end + 2);
            parse_headers();
            if (this->_body_ignored)
            {
                this->_reading_done = true;
                return ;
            }
            this->_body = this->_body.substr(headers_end + 4);
            // create a file to store the body
            std::string file_type = "text/plain";
            std::map<std::string, std::string>::iterator it3 = this->_headers_map.find("Content-Type");
            if (it3 != this->_headers_map.end())
                file_type = it3->second;
            std::string file_ext = get_content_type(file_type, 1);
            this->_body_name = create_body(file_ext);
            this->_body_recieved_len = 0;
        }
        else
            return ;
    }
    upload_body();
}

void    Request::upload_body()
{
    if (this->_is_chunked)
            uploading();
    else
    {
        write(this->_uploaded_fd, _body.c_str(), _body.length());
        this->_body_recieved_len += _body.length();
        // look for end of body
        if (this->_body_length == this->_body_recieved_len)
        {
            close (this->_uploaded_fd);
            this->_which_body = NONE;
            this->_reading_done = true;
        }
        if (this->_body_length < this->_body_recieved_len)
        {
            close (this->_uploaded_fd);
            throw HTTPException(400);
        }
        this->_body = "";
    }
    // check the uploading file len
    if (this->_body_recieved_len > this->_serving_location.get_client_max_body_size())
    {
        close (this->_uploaded_fd);
        throw HTTPException(413);
    }
}

void    Request::uploading()
{
    size_t  cr_lf = this->_body.find("\r\n");
    if (cr_lf != std::string::npos)
    {
        unsigned long int _chunk_len = std::strtoul(this->_body.substr(0, cr_lf).c_str(), NULL, 16);
        if (_chunk_len == 0) // end of body
        {
            this->_which_body = NONE;
            close (this->_uploaded_fd);
            this->_reading_done = true;
        }
        if (this->_body.length() - (cr_lf + 2) >= _chunk_len + 2)
        {
            std::string cleand_body = this->_body.substr(cr_lf + 2, _chunk_len);
            write(this->_uploaded_fd, cleand_body.c_str(), cleand_body.length());
            this->_body_recieved_len += cleand_body.length();
            // update the body string
            this->_body = this->_body.substr(cr_lf + 2 + _chunk_len + 2);
            uploading();
        }
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
    std::string file_name = this->_serving_location.get_root(); // the root of the location where the upload gonna happen
    file_name += ss.str() + _ext_;
    this->_uploaded_fd = open(file_name.c_str(), O_CREAT | O_WRONLY, 777);
    if (this->_uploaded_fd == -1)
    {
        std::cerr << "Failed to open the uploaded file." << std::endl;
        throw HTTPException(500);
    }
    return (file_name);
}

void    Request::set_perfect_server()
{
    std::vector<Server>::iterator it = this->_all_servers.begin();
    while (it != this->_all_servers.end())
    {
        if (it->get_host() == this->_default_server.get_host()
            && it->get_port() == this->_default_server.get_port()
            && it->get_server_name() == this->_server_name)
        {
            this->server = *it;
            return ;
        }
        it++;
    }
    this->server = this->_default_server;
}

void    Request::analyze_headers()
{
    std::map<std::string, std::string>::iterator it1 = _headers_map.find("Host");
    if (it1 != _headers_map.end())
    {
        this->_server_name = it1->second;
        set_perfect_server();
    }
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
    if (this->_method == POST)
    {
        make_location_ready(); // find Location
    }
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
    {
        this->_method = GET;
        this->_body_ignored = true;
    }
    else if (this->_method_str.compare("DELETE") == 0)
    {
        this->_method = DELETE;
        this->_body_ignored = true;
    }
    else if (this->_method_str.compare("POST") == 0)
    {
        this->_method = POST;
        this->_body_ignored = false;
    }
    else
    {
        this->_method = NONE;
        throw HTTPException(501);
    }
}

void    Request::parse_path()
{
    size_t question_mark = this->_path.find("?");
    if (question_mark != std::string::npos)
    {
        if (this->_path.length() >= question_mark + 2)
            this->_query_string = this->_path.substr(question_mark + 1);
        this->_path = this->_path.substr(0, question_mark);
    }
    else
        this->_query_string = "";
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

bool    Request::_keep_connection()
{
    std::map<std::string, std::string>::iterator it = _headers_map.find("Connection");
    if (_headers_finished && it != _headers_map.end() && it->second == "keep-alive")
    {
        return (true);
    }
    return (false);
}

int     Request::get_error_code()
{
    return (this->_error_code);
}

///////////// response //////////////////
void    Request::build_response()
{
    // find Location
    make_location_ready();
    if (this->_method == GET || this->_method == POST)
        GET_handler();
    else
        DELETE_handler();
}

void    Request::make_location_ready()
{
    if (get_matched_location() == 0)
        throw HTTPException(404);
    // check return
    if (check_return())
        throw HTTPException(301);
    // is allowed method
    if (this->_serving_location.is_allowed_method(this->_method_str) == false)
        throw HTTPException(405);
}

int Request::get_matched_location()
{
    unsigned long int more_specified = 0;
    std::vector<Location>  locations = server.get_locations();       
    unsigned long int i = 0;
    while (i < locations.size())
    {
		std::string	loc_path =  locations[i].get_path();
		bool is_fix = (loc_path[loc_path.size() - 1] == '/' && loc_path != "/");
		if (is_fix)
			loc_path = loc_path.substr(0, loc_path.size() - 1);

        size_t found_location_path = this->_path.find(loc_path);
		// std::cout << loc_path << "|" << this->_path << "|is_fix : " << is_fix << std::endl;
		// /root/test/sssssss/
		// /root/test/sssssssasdasdasd
		// loc_path.size() < this->_path -> this->path[loc_path.size()]
        if (found_location_path == 0)
        {
            if (this->_path.length() == loc_path.length())
            {
                if (loc_path.length() > more_specified)
                {
                    this->_serving_location = locations[i];
                    more_specified = loc_path.length();
                }
            }
            else if (this->_path.length() > loc_path.length())
            {
                if (this->_path[loc_path.length()] == '/' 
                    || loc_path == "/"
					|| (!is_fix && this->_path[loc_path.length()] != '/'))
                {
                    if (loc_path.length() > more_specified)
                    {
                        this->_serving_location = locations[i];
                        more_specified = loc_path.length();
                    }
					if (!is_fix && loc_path != "/") {
						size_t fnd = this->_path.find("/", loc_path.size());
						if (fnd != std::string::npos)
							this->_path = this->_path.substr(0, loc_path.size()) + this->_path.substr(fnd);
						else
							this->_path = this->_path.substr(0, loc_path.size());
					}
                }else
					return (0);
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

int    Request::GET_directory()
{
    if (this->_path[this->_path.length() - 1] != '/')
    {
        this->_path += "/";
        this->_returned_location = this->_path;
        throw HTTPException(301);
    }
    if (this->_serving_location.get_index() != "")
    {
        this->_response_body_file = this->_serving_location.get_index();
        return (1);
    }
    else
    {
        if (this->_method == POST && this->_serving_location.is_upload())
            throw HTTPException(201);
        else if (this->_method == GET && this->_serving_location.is_autoindex()) // only GET
        {
            // generate an HTML page listing the contents of a directory
            this->_response_body = generate_html_page_dir(this->_response_body_file);
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
    if (ext_found != ext_map.end()) // run cgi
    {
        this->_is_cgi = true;
        cgi_process(ext_found);
        waiting_child();
    }
    else
    {
        if (this->_method == POST) // post
        {
            if (this->_serving_location.is_upload())
                throw HTTPException(201);
            else
                throw HTTPException(403);
        } 
        this->_which_body = FILE_BODY; // get
        throw HTTPException(200);
    }
}

void    Request::DELETE_handler()
{
    int path_type = find_requested_file();
    if (path_type == 1)
        delete_directory();
    else if (path_type == 0)
        delete_file();
    else 
        throw HTTPException(404);
}

int    Request::delete_directory()
{
    if (this->_path[this->_path.length() - 1] != '/')
        throw HTTPException(409);
    if (access(this->_response_body_file.c_str(), W_OK) != 0)
            throw HTTPException(403);
    delete_directory_contents(this->_response_body_file);
    throw HTTPException(204);
    return (EXIT_SUCCESS);
}

void    Request::delete_file()
{
    if (access(this->_response_body_file.c_str(), W_OK) != 0)
        throw HTTPException(403);
    if (unlink(this->_response_body_file.c_str()) == 0)
        throw HTTPException(204);
    else
        throw HTTPException(500);
}

int    Request::find_requested_file()
{
	std::string only_path;
    std::string rooooooot = this->_serving_location.get_root();
    if (rooooooot[rooooooot.length() - 1] != '/')
        rooooooot.push_back('/');
	if (this->_serving_location.get_path().size() < this->_path.size()) {
    	only_path = this->_path.substr(this->_serving_location.get_path().length());
	}
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

void    Request::set_response_headers(std::string _code_str)
{
    this->_response_headers = "";
    // status line 
    _response_headers += std::string("HTTP/1.1") + " ";
    _response_headers += _code_str + " ";
    _response_headers += server.get_error_messages(_status_code) + "\r\n";
    // Content Type or Location and Content length
    if (this->_status_code >= 300 && this->_status_code < 400)
    {
        _response_headers += std::string("Location:") + " ";
        _response_headers += this->_returned_location + "\r\n";
        _response_headers += std::string("Content-Length:") + " ";
        _response_headers += std::string("0") + "\r\n";
    }
    else if (this->_status_code == 200 && this->_which_body == STR_BODY)
    {
        _response_headers += std::string("Content-Type:") + " ";
        _response_headers += std::string("text/html") + "\r\n";
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
        _response_headers += std::string("Content-Type:") + " ";
        _response_headers += get_content_type(ext, 0) + "\r\n";
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
    if (true)
    {
        std::time_t currentTime;
        std::time(&currentTime);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&currentTime));
        std::string date_str(buffer);
        _response_headers += std::string("Date:") + " ";
        _response_headers += date_str + "\r\n";
    }
    _response_headers += "\r\n";
}

void    Request::cgi_process(std::map<std::string,std::string>::iterator ext_found)
{
	std::string str = this->_response_body_file;
	int fd_out = create_file(".txt", this->_response_body_file);
    this->_child_id = fork();
    if (this->_child_id == -1)
    {
		close(fd_out);
        perror("fork");
        throw HTTPException(500);
    }
    else if (this->_child_id == 0) // child
    {
        dup2(fd_out, 1);
		close(fd_out);
		this->_response_body_file = str;
        execute_cgi(ext_found);
    }
    // parent
	close(fd_out);
    this->_child_start = std::time(NULL);
    _waiting_done = false;
}

void   Request::waiting_child()
{
    int status;
    pid_t result;

    std::time_t _noow_ = std::time(NULL);
    if (_noow_ - this->_child_start >= CHILD_TIMEOUT)
    {
        kill(this->_child_id, SIGTERM);
        waitpid(this->_child_id, NULL, 0);
        _waiting_done = true;
		this->_child_id = -1;
        throw HTTPException(408);
    }
    result = waitpid(this->_child_id, &status, WNOHANG);
    if (result == -1)
    {
        perror("waitpid");
        _waiting_done = true;
    }
    else if (result != 0) // Child process has finished
    {
        _waiting_done = true;
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0)
            {
                // printf("Child process exited with status: %d\n", WEXITSTATUS(status));
                throw HTTPException(500);
            }
        } else if (WIFSIGNALED(status))
        {
            // printf("Child process terminated by signal: %d\n", WTERMSIG(status));
            throw HTTPException(500);
        }
        recv_cgi_response();
    }
    // result == 0 ==> child still running
} 

void    Request::execute_cgi(std::map<std::string,std::string>::iterator ext_found)
{
    char* program_path = (char *)this->_response_body_file.c_str();
    char* const args[3] = {(char *)ext_found->second.c_str(), program_path, NULL};
    char* env[8];
    // QUERY_STRING
    std::string query_str("QUERY_STRING=");
    if (this->_method == GET)
    {
        query_str.append(this->_query_string);
    }
    else if (this->_method == POST)
    {
        int fd = open(this->_body_name.c_str(), O_RDONLY);
        if (fd == -1)
        {
            std::cout << "infile in cgi open error" << std::endl;
            exit (1);
        }
        dup2(fd, 0);
        close (fd);
    }
    // Cookies
    std::string cookie("HTTP_COOKIE=");
    std::map<std::string, std::string>::iterator it0 = this->_headers_map.find("Cookie");
    if (it0 != this->_headers_map.end())
        cookie.append(it0->second);
    // content type FOR POST
    std::string content_type("CONTENT_TYPE=");
    std::map<std::string, std::string>::iterator it1 = this->_headers_map.find("Content-Type");
    if (it1 != this->_headers_map.end())
        content_type.append(it1->second);
    // content length FOR POST
    std::string content_length("CONTENT_LENGTH=");
    std::map<std::string, std::string>::iterator it2 = this->_headers_map.find("Content-Length");
    if (it2 != this->_headers_map.end())
        content_length.append(it1->second);
    // method, filename, _status
    std::string request_method("REQUEST_METHOD=");
    request_method.append(this->_method_str);
    std::string script_filename("SCRIPT_FILENAME=");
    script_filename.append(this->_response_body_file);
    std::string redirect_status("REDIRECT_STATUS=");
    redirect_status.append("200");
    env[0] = (char*)content_type.c_str();
    env[1] = (char*)content_length.c_str();
    env[2] = (char*)request_method.c_str();
    env[3] = (char*)script_filename.c_str(); // path to script
    env[4] = (char*)redirect_status.c_str();
    env[5] = (char*)cookie.c_str();
    env[6] = (char*)query_str.c_str();
    env[7] = NULL;
    if (execve(args[0], args, env) == -1)
    perror("execve");
    exit (1);
}

void    Request::recv_cgi_response()
{
    char buffer[1024];
	int fd_out = -1;
    ssize_t bytes_read;
    std::string cgi_body;
    std::string cgi_returned_headers;

    int fd_in = open(this->_response_body_file.c_str(), O_RDONLY, 0666);
	if (fd_in < 0)
		throw HTTPException(500);
    int premier_read = true;
    while (true)
    {
        bytes_read = read(fd_in, buffer, sizeof(buffer));

        if (bytes_read < 0)
        {
            perror("read from pipe");
            exit (1);
        }
        else if (bytes_read == 0)
            break ;
        std::string cgi_return(buffer, bytes_read);
        if (premier_read)
        {
			fd_out = create_file(".txt", this->_response_body_file);
			premier_read = false;
            size_t body_start = cgi_return.find("\r\n\r\n");
            if (body_start == std::string::npos)
                cgi_returned_headers = "";
            else
            {
                cgi_returned_headers = cgi_return.substr(0, body_start + 2);
                cgi_return = cgi_return.substr(body_start + 4);
            }
        }
		if (fd_out != -1)
	        write(fd_out, cgi_return.c_str(), cgi_return.length());
    }
    close(fd_out);
    close(fd_in);
    set_cgi_headers(cgi_returned_headers);
    this->_which_body = FILE_BODY;
    throw HTTPException(677173);
}

void    Request::set_cgi_headers(std::string cgi_return)
{
    // status line
    size_t  found = cgi_return.find("HTTP/1.1");
    if (found != std::string::npos)
    {
        size_t cr_lf = cgi_return.find_first_of(";\r\n", found);
        this->_response_headers = cgi_return.substr(found, cr_lf - found);
        this->_response_headers.append("\r\n");
    }
    else
        this->_response_headers.append("HTTP/1.1 200 OK\r\n");
    // cookies
    found = cgi_return.find("Set-Cookie: ");
    if (found != std::string::npos)
    {
        size_t cr_lf = cgi_return.find_first_of("\r\n", found);
        this->_response_headers += cgi_return.substr(found, cr_lf - found);
        this->_response_headers.append("\r\n");
    }
    // content type
    this->_response_headers.append("Content-Type: ");
    std::string content_type;
    found = cgi_return.find("Content-type: ");
    if (found != std::string::npos)
    {
        size_t semicolon_nl = cgi_return.find_first_of(";\r\n", found);
        content_type = cgi_return.substr(found + 14, semicolon_nl - (found + 14));
        this->_response_headers += content_type;
        this->_response_headers.append("\r\n");
    }
    else
        this->_response_headers.append("text/html\r\n");
    // content length
    this->_response_headers.append("Content-Length: ");
    std::stringstream ss;
    found = cgi_return.find("Content-Length: ");
    if (found != std::string::npos)
    {
        size_t semicolon_nl = cgi_return.find_first_of(";\r\n", found);
        ss << cgi_return.substr(found + 16, semicolon_nl - (found + 16));
    }
    else
        ss << get_file_len(this->_response_body_file);
    this->_response_headers += ss.str() + "\r\n";
    if (true)
    {
        std::time_t currentTime;
        std::time(&currentTime);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&currentTime));
        std::string date_str(buffer);
        this->_response_headers += std::string("Date:") + " ";
        this->_response_headers += date_str + "\r\n";
    }
    // add the body
    this->_response_headers.append("\r\n");
}
