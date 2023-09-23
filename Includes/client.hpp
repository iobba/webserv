#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "webserv.hpp"
#include "request.hpp"
#include "response.hpp"

class   Client
{
    private:
        int         _socket;
        // ... other info

    public:

        Request     _request;
        Response    _response;
        bool        _first_send;
        unsigned long int _sending_offset;


        // setters
        void    set_socket(int socket);
        // ...

        // getters
        const int& get_socket(void)     const;
        // ...




};











#endif