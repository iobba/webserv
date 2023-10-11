#include "../Includes/webserv.hpp"
////////// HTTP exception
HTTPException::HTTPException(int error_code) : error_code_(error_code)
{
    std::stringstream ss;
    ss << error_code_;
    error_str = ss.str();
}

const char* HTTPException::what() const throw()
{
    return (error_str.c_str());
}


/////// confige file exception
CONFIGException::CONFIGException(std::string error) : __error(error) {}

const char* CONFIGException::what() const throw()
{
    return (__error.c_str());
}


//////// Launching servers exception
SERVER_Exception::SERVER_Exception(std::string error) : __error(error) {}

const char* SERVER_Exception::what() const throw()
{
    return (__error.c_str());
}

std::string     get_file_extention(std::string file_name)
{
    size_t dot_position = file_name.rfind('.');

    if (dot_position != std::string::npos) 
    {
        return (file_name.substr(dot_position));
    }
    return ("");
}

void clear_and_replace(std::string file_name, std::string new_content)
{
    // Open the file in output mode, which clears its contents
    std::ofstream output_file(file_name.c_str());
    if (!output_file.is_open())
        throw HTTPException(500);
    // Write the new content to the file
    output_file << new_content;
    output_file.close();
}

std::string get_content_type(std::string to_find, int i)
{
    std::map<std::string, std::string> mime_types;
    mime_types[".html"] = "text/html";
    mime_types[".htm"] = "text/html";
    mime_types[".txt"] = "text/plain";
    mime_types[".css"] = "text/css";
    mime_types[".js"] = "application/javascript";
    mime_types[".json"] = "application/json";
    mime_types[".xml"] = "application/xml";
    mime_types[".jpg"] = "image/jpeg";
    mime_types[".jpeg"] = "image/jpeg";
    mime_types[".png"] = "image/png";
    mime_types[".gif"] = "image/gif";
    mime_types[".bmp"] = "image/bmp";
    mime_types[".ico"] = "image/x-icon";
    mime_types[".pdf"] = "application/pdf";
    mime_types[".zip"] = "application/zip";
    mime_types[".tar"] = "application/x-tar";
    mime_types[".gz"] = "application/gzip";
    mime_types[".mp3"] = "audio/mpeg";
    mime_types[".wav"] = "audio/wav";
    mime_types[".mp4"] = "video/mp4";
    mime_types[".avi"] = "video/x-msvideo";
    mime_types[".csv"] = "text/csv";
    mime_types[".ppt"] = "application/vnd.ms-powerpoint";
    mime_types[".xls"] = "application/vnd.ms-excel";
    mime_types[".doc"] = "application/msword";
    mime_types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    mime_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    mime_types[".svg"] = "image/svg+xml";
    mime_types[".woff"] = "application/font-woff";
    mime_types[".woff2"] = "font/woff2";
    mime_types[".ttf"] = "font/truetype";
    mime_types[".otf"] = "font/opentype";
    mime_types[".eot"] = "application/vnd.ms-fontobject";
    mime_types[".csv"] = "text/csv";
    mime_types[".swf"] = "application/x-shockwave-flash";
    mime_types[".flv"] = "video/x-flv";
    mime_types[".py"] = "text/x-python";
    mime_types[".php"] = "application/x-php";
    mime_types[".sh"] = "application/x-shellscript";

    if (i == 0)
    {
        std::map<std::string, std::string>::iterator it = mime_types.find(to_find);
        if (it != mime_types.end())
            return (it->second);
        return ("text/plain");
    }
    // i == 1
    std::map<std::string, std::string>::iterator it1 = mime_types.begin();
    while (it1 != mime_types.end())
    {
        if (it1->second == to_find)
            return (it1->first);
        it1++;
    }
    return (".txt");
} 

long unsigned int     get_file_len(std::string file__name)
{
    std::ifstream file(file__name.c_str());
    std::cout << "file naaaame = [" << file__name << "]\n";
    if (!file.is_open())
        throw HTTPException(500);
    file.seekg(0, std::ios::end); // Move to the end of the file
    long unsigned int   __len = file.tellg(); // Get the current position (file size)
    file.close();
    return (__len);
}

int     which_type(std::string  pa_th)
{
    // to check if it's a file or directory or ...
    struct stat path_info;
    if (stat(pa_th.c_str(), &path_info) == 0)
    {
        if (S_ISREG(path_info.st_mode))
            return (0);
        else if(S_ISDIR(path_info.st_mode))
            return (1);  
        else
            return (2);
    }
    return (3);
}

int delete_directory_contents(std::string path)
{
    DIR* dir = opendir(path.c_str());
    if (!dir)
    {
        std::cerr << "Error opening directory" << std::endl;
        throw HTTPException(404);
    }

    dirent* entry;
    while ((entry = readdir(dir)))
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            if (path[path.length() - 1] != '/')
                path.append("/");
            std::string full_path = path + entry->d_name;
            if (access(full_path.c_str(), W_OK) != 0) // write access
                throw HTTPException(403);
            int dir_or_file = which_type(full_path);
            if (dir_or_file == 0) // file
            {
                if (unlink(full_path.c_str()) != 0)
                    throw HTTPException(500);
            }
            else if (dir_or_file == 1) // directory
            {
                delete_directory_contents(full_path);
            }
            else // Unknown type
                throw HTTPException(500);
        }
    }
    closedir(dir);
    if (rmdir(path.c_str()) != 0)
        throw HTTPException(500);
    return (0);
}

std::string     generate_html_page_dir(std::string dir_path)
{
    DIR* dir = opendir(dir_path.c_str());
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
    return (html_content);
}

int    create_file(std::string _ext_, std::string& naaaame)
{
    // Get the current time in milliseconds
    std::time_t currentTime = std::time(NULL);
    unsigned int milliseconds = static_cast<unsigned int>(currentTime);
    static long unsigned int num_files = 0;
    num_files++;
    long unsigned int randooooooomNum = milliseconds + num_files;
    std::stringstream ss;
    ss << randooooooomNum;
    std::string file_name = "/Users/iobba/goinfre/"; // path
    file_name += ss.str() + _ext_;
    // std::cout << "heeeeeeeeeere = " << file_name << std::endl;
    int fd = open(file_name.c_str(), O_CREAT | O_WRONLY, 0777);
    if (fd == -1)
    {
        std::cerr << "Failed to the uploaded file." << std::endl;
        throw HTTPException(500);
    }
    naaaame = file_name;
    return (fd);
}
