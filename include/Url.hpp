#ifndef __RALFOGIT_URL_HPP__
#define __RALFOGIT_URL_HPP__

#include <string>

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#else
namespace libralfogit {
#endif

    /**
     *  Class implementing url parsing.
     */
    class Url {
    public:

        Url(void);
        Url(const std::string& url);
        Url(const std::string& protocol, const std::string& host, const std::string& path, const std::string& query, const std::string& fragment);

        std::string getUrl(void);
        std::string getProtocol(void);
        std::string getHost(void);
        int         getPort(void);
        std::string getPath(void);
        std::string getQuery(void);
        std::string getFragment(void);

        static int parseUrl(const std::string& url, std::string& protocol, std::string& host, int& port, std::string& path, std::string& query, std::string& fragment);

        static std::string percentEncode(const std::string& url_component, const std::string::value_type url_component_identifier);

    private:
        std::string url;
        std::string protocol;
        std::string host;
        int         port;
        std::string path;
        std::string query;
        std::string fragment;
    };

}   // namespace ralfogit

#endif
