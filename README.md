## Webserv (3d Team project after minishell and cub3D)

## Introduction

The Webserv project involves creating a HTTP server in C++ 98, adhering to the HTTP 1.1 protocol specifications and handling various HTTP methods and functionalities.

## Overview

**Objective**
- Implement a HTTP server capable of serving static websites, handling CGI scripts, supporting multiple HTTP methods, and more.
  - `GET`
  - `POST`
  - `DELETE`
  - `...` 

**Key Concepts**

  - **HTTP Protocol**
     - The Hypertext Transfer Protocol (HTTP) is an application protocol for distributed, collaborative, hypermedia information systems. It is fundamental for data communication on the World Wide Web.
  
  - **NGINX**
    - NGINX is a high-performance web server and reverse proxy. Understanding its configuration and behavior can help in implementing similar functionalities in your HTTP server.
  
  - **CGI (Common Gateway Interface)**
    - CGI allows external programs to interact with web servers through environment variables and standard input/output. It's used to execute scripts dynamically on the server.
  
  - **Non-blocking I/O**
    - Using techniques like `poll()`, `select()`(the one that i used), or `epoll()` for non-blocking I/O operations ensures that the server remains responsive to multiple client requests simultaneously.

## Some Usefull Resources
  - `Server setup and an overview on the general Topics`
    - https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa
    - https://ncona.com/2019/04/building-a-simple-server-with-cpp/
    - https://www.tutorialspoint.com/cplusplus/cpp_web_programming.html
    - https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
    - https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/index.html [Only chapter 4 and 5]

  - `HTTP 1.1 (standard to follow)`
    - https://www.rfc-editor.org/rfc/rfc2616.html
    - https://www.rfc-editor.org/rfc/rfc7230.html
    - https://www.rfc-editor.org/rfc/rfc7231.html
    - https://www.rfc-editor.org/rfc/rfc7232.html

  - `HTTP Header Syntax`
    - https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol#Request_methods
    - https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    - https://stackoverflow.com/questions/5757290/http-header-line-break-style
    
  - `CGI`
    - http://www.mnuwer.dbasedeveloper.co.uk/dlearn/web/session01.htm
    - http://www.wijata.com/cgi/cgispec.html#4.0

