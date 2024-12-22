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
#define _CRT_SECURE_NO_WARNINGS

#include <TasmotaAPI.hpp>
#include <HttpClient.hpp>
#include <Url.hpp>
#include <JsonCpp.hpp>
#include <locale>

#ifdef LIB_NAMESPACE
using namespace LIB_NAMESPACE;
#else
using namespace libtasmota;
#endif

/**
 * Constructor.
 * @param url the first part of the tasmota device url, e.g. "http://192.168.1.2"
 */
TasmotaAPI::TasmotaAPI(const std::string& url) :
    host_url(url)
{}


/**
 * Get a vector of modules supported by the firmware.
 * @return a map of module id and module name pairs
 */
std::map<std::string, std::string> TasmotaAPI::getModules(void) const {
    std::map<std::string, std::string> modules;

    // get json response from device
    std::string http_status;
    json_value* json = getJsonResponse("Modules", http_status);
    if (json != NULL) {

        // get Modules element and all sub-elements
        const JsonCpp::JsonNamedValueVector roots = JsonCpp::getNamedValues(json);
        const JsonCpp::JsonNamedValue&      root  = roots[0];
        if (compareNames(root.getName(), "Modules", true) && root.isObject()) {
            for (const auto element : root.asObject()) {
                modules[element.getName()] = std::string(element.getValue());
            }
       }
        json_value_free(json);
    }
    return modules;
}


/**
 * Get the value for the given key from the tasmota device; the value is converted to a string.
 * @param key the name of the key value pair
 * @return the value of the key value pair
 */
std::string TasmotaAPI::getValue(const std::string& name) const {
    std::string http_status, value;

    // get json response from device
    json_value* json = getJsonResponse(name, http_status);
    if (json != NULL) {
        // search json for the given name
        value = getValueFromJson(json, name);
        json_value_free(json);
    }
    if (value.length() > 0) {
        return value;
    }
    return http_status;
}


/**
 * Get the value for the given key path from the tasmota device; the value is converted to a string.
 * The key path is a string containing path segments, separated by ':' characters. The path is defining
 * the traversal through the result of a "Status 0" command to the tasmota device.
 * @param key the key path of the key value pair. e.g. "StatusSNS:ENERGY:Power" to get the power consumption
 * @return the value of the key value pair
 */
/**
 * Get the value for the given key path from the phoscon device; the value is converted to a string.
 * The key path is a string containing path segments, separated by ':' characters. The path is defining
 * the traversal through the result of a "Status 0" command to the phoscon device.
 * @param gw phoscon gateway
 * @param device zigbee device id
 * @param path the path to the leaf key value pair. e.g. "subdevices:1:state:power:value" to get the power consumption
 * @return the value of the leaf key value pair
 */
std::string TasmotaAPI::getValueFromPath(const std::string& path) const {
    std::string http_status, result;

    // get json response from device
    json_value* json = getJsonResponse("Status%200", http_status);
    if (json != NULL && json->type != json_null && json->type != json_none) {

        // split path into segments
        std::vector<std::string> path_segments = getPathSegments(path);

        // declare name comparators for json nodes and json leafs
        struct NameComparator {
            static bool compare_node_names(const std::string& lhs, const std::string& rhs) { return compareNames(lhs, rhs, true); }
            static bool compare_leaf_names(const std::string& lhs, const std::string& rhs) { return compareNames(lhs, rhs, false); }
        };

        // traverse path
        JsonCpp::JsonValue traveler(json);

        if (path_segments.size() > 1) {
            for (size_t i = 0; i < path_segments.size() - 1; ++i) {
                if (traveler.isObject()) {
                    traveler = JsonCpp::getValue(traveler.asObject(), path_segments[i], NameComparator::compare_node_names);
                }
                else if (traveler.isArray()) {
                    unsigned int index = 0;
                    if (sscanf(path_segments[i].c_str(), "%u", &index) == 1 && index < traveler.asArray().size()) {
                        traveler = traveler.asArray()[index];
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
        if (path_segments.size() > 0 && traveler.isObject()) {
            JsonCpp::JsonValue leaf = JsonCpp::getValue(traveler.asObject(), path_segments[path_segments.size() - 1], NameComparator::compare_leaf_names);
            result = std::string(leaf);
        }
        json_value_free(json);
    }
    if (result.length() > 0) {
        return result;
    }
    return http_status;
}



/**
 * Set the value in the tasmota device.
 */
std::string TasmotaAPI::setValue(const std::string& name, const std::string& value) {
    // assemble the tasmota device url
    std::string device_url = assembleHttpUrl(name, value);

    // send http put request
    HttpClient http_client;
    std::string response;
    std::string content;
    int http_return_code = http_client.sendHttpPutRequest(device_url, "", response, content);

    // check if the http return code is 200 OK
    if (http_return_code == 200) {

        // parse json response
        json_value* json = json_parse(content.c_str(), content.length());
        if (json != NULL) {

            // search json for the given name
            std::string value = getValueFromJson(json, name);
            if (value.length() > 0) {
                json_value_free(json);
                return value;
            }
        }
        json_value_free(json);
        return content;
    }
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "HTTP-Returncode: %d : ", http_return_code);
    return std::string(buffer).append(content);
}


/**
 * Assemble a tasmota device url.
 * @param name the tasmota command name
 * @param value the value if this is a set command; provide an empty string for get commands
 * @return the device url, i.e. something like "http://192.168.1.2:80/cm?cmnd=Status%200"
 */
std::string TasmotaAPI::assembleHttpUrl(const std::string& command, const std::string& value) const {
    // parse the given host_url to separate protocol and host
    std::string protocol, user, password, host, path, query, fragment; int port;
    Url::parseUrl(host_url, protocol, user, password, host, port, path, query, fragment);

    // overwrite path, query and fragment url components to reflect the given command
    path  = "/cm";
    query = "?cmnd=";
    query.append(command);
    if (value.length() > 0) {
        query.append(" ");
        query.append(value);
    }
    fragment.clear();

    // assemble the tasmota device url
    Url url(protocol, user, password, host, path, query, fragment);
    return url.getUrl();
}


/**
 * Get the json response for the given command om the tasmota device.
 * An http get request is send to "http://'host_url'/cm?cmnd='command'.
 * @param command the tasmota command string.
 * @param error
 * @return the json response tree.
 */
json_value* TasmotaAPI::getJsonResponse(const std::string& command, std::string& http_status) const {
    // assemble the tasmota device url
    std::string device_url = assembleHttpUrl(command);

    // send http get status request
    HttpClient http_client;
    std::string response;
    std::string content;
    int http_return_code = http_client.sendHttpGetRequest(device_url, response, content);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "HTTP-Returncode: %d : ", http_return_code);
    http_status = std::string(buffer).append(content);

    // check if the http return code is 200 OK
    if (http_return_code == 200) {
        // parse json content
        json_value* json = json_parse(content.c_str(), content.length());
        return json;
    }
    return NULL;
}


/**
 * Get the value for the given name from the given json tree.
 * @param json the json tree
 * @param name the name of the name value pair
 * @return the value of the name value pair
 */
std::string TasmotaAPI::getValueFromJson(const json_value* const json, const std::string& name) {
    if (json != NULL) {

        // analyze the json response
        const JsonCpp::JsonNamedValueVector roots = JsonCpp::getNamedValues(json);
        const JsonCpp::JsonNamedValue&      root  = roots[0];
        if (compareNames(root.getName(), name, false)) {
            if (root.isObject()) {
                for (const auto element : root.asObject()) {
                    const std::string name  = element.getName();
                    const std::string value = std::string(element.getValue());
                    return value;
                }
            }
            const std::string value = std::string(root);
            return value;
        }
    }
    return "";
}


/**
 * Compare tasmota key names.
 * @param name1 the first name to compare
 * @param name2 the second name to compare
 * @param strict false: name extensions with digits are ignored; true: only differences in lower and upper case are ignored
 * @return true, if the two names are considered to be equal; false, if the two names are considered to be different
 */
bool TasmotaAPI::compareNames(const std::string& name1, const std::string& name2, const bool strict) {

    // first check if both strings are identical
    if (name1 == name2) {
        return true;
    }

    // extract base names by removing any trailing digits
    std::string base_name1 = name1;
    std::string base_name2 = name2;

    // if the comparison is not strict, consider "Module0" and "Module" as the same name
    if (strict == false) {
        while (base_name1.length() > 0) {
            char last_char = base_name1.at(base_name1.length() - 1);
            if (last_char < '0' || last_char > '9') break;
            base_name1 = base_name1.substr(0, base_name1.length() - 1);
        }
        while (base_name2.length() > 0) {
            char last_char = base_name2.at(base_name2.length() - 1);
            if (last_char < '0' || last_char > '9') break;
            base_name2 = base_name2.substr(0, base_name2.length() - 1);
        }
    }

    // check if lengths of both strings are identical
    if (base_name1.length() == base_name2.length()) {

        // check if both strings are identical, when converted to lower case
        bool same = true;
        for (int i = 0; i < base_name1.length(); ++i) {
            std::string::value_type char1 = base_name1[i];
            std::string::value_type char2 = base_name2[i];
            if (char1 != char2) {
                std::string::value_type char1_lower = std::tolower(char1);
                std::string::value_type char2_lower = std::tolower(char2);
                if (char1_lower != char2_lower) {
                    same = false;
                    break;
                }
            }
        }
        if (same == true) {
            return true;
        }
    }

    return false;
}


/**
 * Segment a given key path into a vector of path segments.
 * The key path is a string containing path segments, separated by ':' characters. The path is defining
 * the traversal through the result of a "Status 0" command to the tasmota device.
 * @param key the key path of the key value pair. e.g. "StatusSNS:ENERGY:Power" to get the power consumption
 * @return a vector containing path segments, e.g. "StatusSNS", "ENERGY", "Power"
 */
std::vector<std::string> TasmotaAPI::getPathSegments(const std::string& path) {
    std::vector<std::string> segments;
    std::string::size_type index = 0;
    while (true) {
        std::string::size_type index_next = path.find(':', index);
        if (index_next == std::string::npos) {
            std::string segment = path.substr(index);
            if (segment.size() > 0) {
                segments.push_back(segment);
            }
            break;
        }
        std::string segment = path.substr(index, index_next-index);
        if (segment.size() > 0) {
            segments.push_back(segment);
        }
        index = index_next + 1;
    }
    return segments;
}
