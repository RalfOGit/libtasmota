/*
 * Copyright(C) 2022 RalfO. All rights reserved.
 * https://github.com/RalfOGit/libtasmota
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#include <Ws2tcpip.h>
#define poll(a, b, c)  WSAPoll((a), (b), (c))
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <arpa/inet.h>
#endif

#include <HttpClient.hpp>
#include <Url.hpp>
using namespace libtasmota;


/**
 *  Close the given socket in a platform portable way.
 */
static void close_socket(const int socket_fd) {
#ifdef _WIN32
    int result = closesocket(socket_fd);
#else
    int result = close(socket_fd);
#endif
}


/**
 *  Constructor.
 */
HttpClient::HttpClient(void) {
#ifdef _WIN32
    // initialize Windows Socket API with given VERSION.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        perror("WSAStartup failure");
    }
#endif
}


/**
 * Send http get request and receive http response and content payload
 * @param url http get request url
 * @param response http response string returned by server
 * @param content http content string retured by server
 * @return http return code
 */
int HttpClient::sendHttpGetRequest(const std::string& url, std::string& response, std::string& content) {

    std::string host;
    std::string path;
    int socket_fd = connect_to_server(url, host, path);

    // assemble http get request
    std::string request;
    request.reserve(256);
    request.append("GET ").append(path).append(" HTTP/1.1\r\n");
    request.append("Host: ").append(host).append("\r\n");
    request.append("User-Agent: libtasmota/1.0\r\n");
    request.append("Accept: application/json\r\n");
    request.append("Accept-Language: de,en-US;q=0.7,en;q=0.3\r\n");
    request.append("Connection: keep-alive\r\n");
    request.append("\r\n");

    // send http get request string, receive response and content
    int http_return_code = communicate_with_server(socket_fd, request, response, content);
    return http_return_code;
}


/**
 * Send http put request, receive http response and content payload
 * @param url http put request url
 * @param response http response string returned by server
 * @param content http content string retured by server
 * @return http return code, or -1 if socket connect failed
 */
int HttpClient::sendHttpPutRequest(const std::string& url, std::string& response, std::string& content) {

    // establish tcp connection to server
    std::string host;
    std::string path;
    int socket_fd = connect_to_server(url, host, path);
    if (socket_fd < 0) {
        return socket_fd;
    }

    // assemble http put request
    std::string request;
    request.reserve(256);
    request.append("PUT ").append(path).append(" HTTP/1.1\r\n");
    request.append("Host: ").append(host).append("\r\n");
    request.append("User-Agent: libtasmota/1.0\r\n");
    request.append("Accept: application/json\r\n");
    request.append("Accept-Language: de,en-US;q=0.7,en;q=0.3\r\n");
    request.append("Connection: keep-alive\r\n");
    request.append("\r\n");

    // send http get request string, receive response and content
    int http_return_code = communicate_with_server(socket_fd, request, response, content);
    return http_return_code;
}


/**
 * Connect to the given server url.
 * @param url http put request url
 * @param host host part extracted from the given http put request url
 * @param path path part extracted from the given http put request url
 * @return socket file descriptor, or -1 if the connect attempt failed.
 */
int HttpClient::connect_to_server(const std::string& url, std::string& host, std::string& path) {

    // parse the given url
    std::string protocol;
    int         port;
    std::string path_tmp;
    std::string query;
    std::string fragment;
    if (Url::parseUrl(url, protocol, host, port, path_tmp, query, fragment) < 0) {
        perror("url failure");
        return -1;
    }
    path = path_tmp;
    path.append(query);
    path.append(fragment);

    // open socket
    int socket_fd = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        perror("socket open failure");
        return -1;
    }

    // open connection to http server
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", port);
    struct addrinfo* addr = NULL;
    if (getaddrinfo(host.c_str(), buffer, NULL, &addr) == 0) {
        while (addr != NULL) {
            if (connect(socket_fd, addr->ai_addr, (int)addr->ai_addrlen) >= 0) {
                freeaddrinfo(addr);
                return socket_fd;
            }
            addr = addr->ai_next;
        }
       freeaddrinfo(addr);
    }
    perror("connecting stream socket failure");
    close_socket(socket_fd);
    return -1;
}


/**
 * Communicate with the given server - send http request, receive response and content.
 * @param socket_fd socket file descriptor
 * @param request http request string to be sent to server
 * @param response http response string returned by server
 * @param content http content string retured by server
 * @return http return code, or -1 if either the socket send or the socket recv request failed
 */
int HttpClient::communicate_with_server(const int socket_fd, const std::string& request, std::string& response, std::string& content) {

    // send http request string
    if (send(socket_fd, request.c_str(), (int)request.length(), 0) != (int)request.length()) {
        perror("send stream socket failure");
        close_socket(socket_fd);
        return -1;
    }

    // receive http get response data
    char recv_buffer[4096];
    size_t nbytes_total = recv_http_response(socket_fd, recv_buffer, sizeof(recv_buffer));
    if (nbytes_total != (size_t)-1) {
        std::string answer = std::string(recv_buffer, nbytes_total);

        // parse http response data
        int http_return_code = parse_http_response(answer, response, content);
        close_socket(socket_fd);
        return http_return_code;
    }
    close_socket(socket_fd);
    return -1;
}


/**
 * Receive http response and content
 * @param socket_fd socket file descriptor
 * @param recv_buffer a pointer to a buffer big enough to receive the http response including content data
 * @param recv_buffer_size the size of the recv_buffer
 * @return number of bytes received
 */
size_t HttpClient::recv_http_response(int socket_fd, char* recv_buffer, int recv_buffer_size) {
    struct pollfd fds;
    size_t nbytes_total = 0;
    recv_buffer[nbytes_total] = '\0';
    bool http_header_complete = false;
    bool chunked_encode = false;
    size_t content_length = 0;
    size_t content_offset = 0;

    while (1) {
        fds.fd = socket_fd;
        fds.events = POLLIN;
        fds.revents = 0;

        // wait for a packet on the configured socket
        int pollresult = poll(&fds, 1, 5000);
        if (pollresult == 0) {
            perror("poll timeout");
            return (nbytes_total > 0 ? nbytes_total: -1);
        }
        if (pollresult < 0) {
            perror("poll failure");
            return (nbytes_total > 0 ? nbytes_total : -1);
        }

        bool pollnval = (fds.revents & POLLNVAL) != 0;
        bool pollerr  = (fds.revents & POLLERR)  != 0;
        bool pollhup  = (fds.revents & POLLHUP)  != 0;
        bool pollin   = (fds.revents & POLLIN)   != 0;

        // check poll result
        if (pollin == true) {
            // receive data
            int nbytes = recv(socket_fd, recv_buffer + nbytes_total, (int)(recv_buffer_size - nbytes_total - 1), 0);
            if (nbytes < 0) {
                perror("recv stream socket failure");
                return (nbytes_total > 0 ? nbytes_total : -1);
            }
            nbytes_total += nbytes;
            recv_buffer[nbytes_total] = '\0';

            // check if the entire http response header has been received and obtain content length information
            if (http_header_complete == false) {
                size_t content_offs = get_content_offset(recv_buffer, nbytes_total);
                if (content_offs != (size_t)-1) {
                    http_header_complete = true;
                    content_offset = content_offs;
                    chunked_encode = is_chunked_encoding(recv_buffer, content_offset);
                    content_length = get_content_length(recv_buffer, content_offset);
                }
            }
            // if the entire http response header has been received
            if (http_header_complete == true) {
                // check if the content length is explicitly given and if the entire content has been received
                if (content_length != (size_t)-1 &&
                    nbytes_total >= content_offset + content_length) {
                    //printf("recv: nbytes %d  nbytes_total %d  content_offset %d  content_length %d => done\n", nbytes, (int)nbytes_total, (int)content_offset, (int)content_length);
                    break;
                }
                // check if chunked transfer encoding is used and if all chunks have been received
                else if (chunked_encode == true) {
                    char* ptr = recv_buffer + content_offset;
                    size_t next_chunk_offset = -2;
                    while (next_chunk_offset != (size_t)-1 && next_chunk_offset != 0) {
                        next_chunk_offset = get_next_chunk_offset(ptr, nbytes_total - (ptr - recv_buffer));
                        //printf("recv: nbytes %d  nbytes_total %d  content_offset %d  next_chunk_offset %d\n", nbytes, (int)nbytes_total, (int)content_offset, (int)next_chunk_offset);
                        ptr += next_chunk_offset;
                    }
                    if (next_chunk_offset == 0) {
                        //printf("recv: nbytes %d  nbytes_total %d  content_offset %d  next_chunk_offset %d => done\n", nbytes, (int)nbytes_total, (int)content_offset, (int)next_chunk_offset);
                        break;
                    }
                }
            }
            //printf("recv: nbytes %d  nbytes_total %d  content_length %d  content_offset %d\n", nbytes, (int)nbytes_total, (int)content_length, (int)content_offset);
        }
        if (pollnval == true) {
            perror("pollnval");
            return (nbytes_total > 0 ? nbytes_total : -1);
        }
        if (pollerr == true) {
            perror("pollerr");
            return (nbytes_total > 0 ? nbytes_total : -1);
        }
        if (pollhup == true) {  // this is a perfectly valid way to determine a transfer
            perror("pollhup");
            return (nbytes_total > 0 ? nbytes_total : -1);
        }
    }
    return nbytes_total;
}


/**
 * Parse http answer and split into response and content.
 * @param answer input - a string holding both the http response header and response content
 * @param http_response output - a string holding the http response header
 * @param http_content output - a string holding the http response data
 * @return the http return code value or -1 in case of error
 */
int HttpClient::parse_http_response(const std::string& answer, std::string& http_response, std::string& http_content) {

    // extract http return code
    int http_return_code = get_http_return_code((char*)answer.c_str(), answer.length());
    if (http_return_code < 0) {
        http_response = answer;
        return -1;
    }

    // check if chunked encoding is used
    bool chunked_encoding = is_chunked_encoding((char*)answer.c_str(), answer.length());
    if (chunked_encoding == true) {
        std::string temp_content;
        temp_content.reserve(answer.length());

        // determine content offset
        size_t content_offset = get_content_offset((char*)answer.c_str(), answer.length());
        if (content_offset == (size_t)-1) {
            http_response = answer;
            return -1;
        }

        // assemble chunked content
        size_t ptr = content_offset;
        size_t next_chunk_offset = -2;
        while (next_chunk_offset != (size_t)-1 && next_chunk_offset != 0) {
            next_chunk_offset   = get_next_chunk_offset((char*)answer.data() + ptr, answer.length() - ptr);

            size_t chunk_length = get_chunk_length((char*)answer.data() + ptr, answer.length() - ptr);
            size_t chunk_offset = get_chunk_offset((char*)answer.data() + ptr, answer.length() - ptr);
            temp_content.append(answer, ptr + chunk_offset, chunk_length);

            ptr += next_chunk_offset;
        }

        // prepare response and content strings
        http_response = answer.substr(0, content_offset);
        http_content  = temp_content;
    }
    else {
        // extract content length
        size_t content_length = get_content_length((char*)answer.c_str(), answer.length());
        if (content_length == (size_t)-1) {
            http_response = answer;
            return -1;
        }

        // determine content offset
        size_t content_offset = get_content_offset((char*)answer.c_str(), answer.length());
        if (content_offset == (size_t)-1) {
            http_response = answer;
            return -1;
        }

        // prepare response and content strings
        http_response = answer.substr(0, content_offset);
        http_content  = answer.substr(content_offset);
    }

    return http_return_code;
}


/**
 * Parse http header and determine http return code.
 * @param buffer pointer to a buffer holding an http header 
 * @param buffer_size size of the buffer; buffer_size must be one byte less than the underlying buffer size
 * @return the http return code if it is described in the http header, -1 otherwise
 */
int HttpClient::get_http_return_code(char* buffer, size_t buffer_size) {
    char saved_character = buffer[buffer_size];
    buffer[buffer_size] = '\0';
    char* substr = strstr(buffer, "HTTP/1.1 ");
    buffer[buffer_size] = saved_character;
    if (substr != NULL) {
        substr += strlen("HTTP/1.1 ");
        int return_code = -1;
        if ((size_t)(substr - buffer) < buffer_size && sscanf(substr, " %d", &return_code) == 1) {
            return return_code;
        }
    }
    return -1;
}


/**
 * Parse http header and determine content length.
 * @param buffer pointer to a buffer holding an http header
 * @param buffer_size size of the buffer; buffer_size must be one byte less than the underlying buffer size
 * @return the content length if it is described in the http header, -1 otherwise
 */
size_t HttpClient::get_content_length(char* buffer, size_t buffer_size) {
    char saved_character = buffer[buffer_size];
    buffer[buffer_size] = '\0';
    char* substr = strstr(buffer, "\r\nContent-Length: ");
    buffer[buffer_size] = saved_character;
    if (substr != NULL) {
        substr += strlen("\r\nContent-Length: ");
        long long length = -1;
        if ((size_t)(substr-buffer) < buffer_size && sscanf(substr, " %lld", &length) == 1) {
            return length;
        }
    }
    return -1;
}


/**
 * Parse http header and determine content offset.
 * @param buffer pointer to a buffer holding an http header
 * @param buffer_size size of the buffer
 * @return the offset of the first content data byte after the header
 */
size_t HttpClient::get_content_offset(char* buffer, size_t buffer_size) {
    char* substr = strstr(buffer, "\r\n\r\n");
    if (substr != NULL) {
        substr += strlen("\r\n\r\n");
        return (substr - buffer);
    }
    return -1;
}


/**
 * Parse http header and check if chunked transfer encoding is used.
 * @param buffer pointer to a buffer holding an http header
 * @param buffer_size size of the buffer; buffer_size must be one byte less than the underlying buffer size
 * @return true, if chunked encoding is used; false otherwise
 */
bool HttpClient::is_chunked_encoding(char* buffer, size_t buffer_size) {
    char saved_character = buffer[buffer_size];
    buffer[buffer_size] = '\0';
    char* substr = strstr(buffer, "\r\nTransfer-Encoding: chunked");
    buffer[buffer_size] = saved_character;
    if (substr != NULL) {
        return true;
    }
    return false;
}


/**
 * Parse http chunk header and get chunk size.
 * @param buffer pointer to a buffer holding a chunk header
 * @param buffer_size size of the buffer
 * @return the chunk length, -1 otherwise
 */
size_t HttpClient::get_chunk_length(char* buffer, size_t buffer_size) {
    long long length = -1;
    if (sscanf(buffer, "%llx", &length) == 1) {
        return length;
    }
    return -1;
}


/**
 * Parse http chunk header and determine chunk content offset.
 * @param buffer pointer to a buffer holding a chunk header
 * @param buffer_size size of the buffer
 * @return the offset of the first content data byte after the chunk header
 */
size_t HttpClient::get_chunk_offset(char* buffer, size_t buffer_size) {
    char* substr = strstr(buffer, "\r\n");
    if (substr != NULL) {
        substr += strlen("\r\n");
        return (substr - buffer);
    }
    return -1;
}


/**
 * Parse http chunk header and determine offset to next chunk.
 * @param buffer pointer to a buffer holding a chunk header
 * @param buffer_size size of the buffer
 * @return the offset to the next chunk header; 0 if this is the last chunk; -1 if there is not enough data
 */
size_t HttpClient::get_next_chunk_offset(char* buffer, size_t buffer_size) {
    size_t chunk_length = get_chunk_length(buffer, buffer_size);
    if (chunk_length == 0) {
        return 0;
    }
    if (chunk_length == (size_t)-1) {
        return -1;
    }
    size_t chunk_offset = get_chunk_offset(buffer, buffer_size);
    if (chunk_offset == (size_t)-1) {
        return -1;
    }
    char *ptr = buffer + chunk_offset + chunk_length;
    if (ptr + 2 > buffer + buffer_size) {
        return -1;
    }
    if (ptr[0] != '\r' || ptr[1] != '\n') {
        return -1;
    }
    return ptr + 2 - buffer;
}
