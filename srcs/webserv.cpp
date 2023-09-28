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

std::string get_conetnt_type(std::string to_find, int i)
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
