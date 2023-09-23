#ifndef RESPONSE_HPP
#define RESPONSE_HPP


#include "webserv.hpp"
#include "server.hpp"

class   Response
{
    private:
        Location _serving_location;    
        int      _status_code;
    public:
        void    build_response();

};








#endif