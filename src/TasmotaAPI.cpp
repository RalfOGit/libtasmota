#include <TasmotaAPI.hpp>
#include <HttpClient.hpp>
#include <JsonCppWrapper.hpp>
#include <locale>

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

using namespace libtasmota;

TasmotaAPI::TasmotaAPI(const std::string& url) :
    host_url(url)
{
    if (host_url.length() > 0 && host_url[host_url.length() - 1] != '/') {
        host_url.append("/");
    }
}

TasmotaAPI::~TasmotaAPI(void) {
}


// get a vector of modules supported by the firmware
std::map<std::string, std::string> TasmotaAPI::getModules(void) {

    // get json response from device
    json_value* json = getJsonResponse("Modules");
    if (json != NULL) {

        std::map<std::string, std::string> modules;

        // get Modules element and all sub-elements
        const JsonCppWrapper::JsonNamedValueVector& roots = JsonCppWrapper::getNamedValues(json);
        const JsonCppWrapper::JsonNamedValue&       root  = roots[0];
        if (compareNames(root.getName(), "Modules", true) && root.getType() == json_object) {
            const json_object_entry* elements = root.getObject().getElements();
            int64_t              num_elements = root.getObject().getNumElements();
            for (int i = 0; i < num_elements; ++i) {
                const json_object_entry& element = elements[i];
                modules[element.name] = std::string(element.value->u.string.ptr, element.value->u.string.length);
            }
            return modules;
        }
    }
    return std::map<std::string, std::string>();
}


int TasmotaAPI::getPower(void) {
    std::string value = getValue("Power");
    if (value == "ON") {
        return 1;
    }
    else if (value == "OFF") {
        return 0;
    }
    return -1;
}


std::string TasmotaAPI::getValue(const std::string& key) {

    // get json response from device
    json_value* json = getJsonResponse(key);
    if (json != NULL) {

        const JsonCppWrapper::JsonNamedValueVector& roots = JsonCppWrapper::getNamedValues(json);
        const JsonCppWrapper::JsonNamedValue&       root  = roots[0];
        if (compareNames(root.getName(), key, false)) {
            if (root.getType() == json_object) {
                const json_object_entry* elements = root.getObject().getElements();
                int64_t              num_elements = root.getObject().getNumElements();
                for (int i = 0; i < num_elements; ++i) {
                    const json_object_entry& element = elements[i];
                    const std::string name = std::string(element.name);
                    switch (element.value->type) {
                    case json_string:   return JsonCppWrapper::JsonString(&element).getValue();
                    case json_double:   return JsonCppWrapper::JsonDouble(&element).getValueAsString();
                    case json_integer:  return JsonCppWrapper::JsonInt   (&element).getValueAsString();
                    case json_boolean:  return JsonCppWrapper::JsonBool  (&element).getValueAsString();
                    case json_null:     return "null";
                    }
                }
            }
            switch (root.getType()) {
            case json_string:   return root.getString().getValue();
            case json_double:   return root.getDouble().getValueAsString();
            case json_integer:  return root.getInt().getValueAsString();
            case json_boolean:  return root.getBool().getValueAsString();
            case json_null:     return "null";
            }
        }
    }
    return "INVALID";
}


std::string TasmotaAPI::getValueFromPath(const std::string& path) {

    // get json response from device
    json_value* json = getJsonResponse("Status%200");
    if (json != NULL) {

        // split path into segments
        std::vector<std::string> path_segments = getPathSegments(path);

        // declare name comparators for json nodes and json leafs
        struct NameComparator {
            static bool compare_node_names(const std::string& lhs, const std::string& rhs) { return compareNames(lhs, rhs, true); }
            static bool compare_leaf_names(const std::string& lhs, const std::string& rhs) { return compareNames(lhs, rhs, false); }
        };
        
        // traverse path
        const json_object_entry* values = NULL;
        size_t                   length = 0;
        if (path_segments.size() > 1 && json->type == json_object) {
            values = json->u.object.values;
            length = json->u.object.length;
            for (size_t i = 0; i < path_segments.size() - 1; ++i) {
                JsonCppWrapper::JsonNamedValue node = JsonCppWrapper::getValue(values, length, path_segments[i], NameComparator::compare_node_names);
                if (node.getType() != json_object) {
                    break;
                }
                values = node.getObject().getElements();
                length = node.getObject().getNumElements();
            }
        }
        if (values != NULL && length != 0 && path_segments.size() > 0) {
            JsonCppWrapper::JsonNamedValue leaf = JsonCppWrapper::getValue(values, length, path_segments[path_segments.size()-1], NameComparator::compare_leaf_names);
            return leaf.getValueAsString();
        }
    }
    return "NOT_FOUND";
}


json_value* TasmotaAPI::getJsonResponse(const std::string& command) {
    // assemble host_url + command
    std::string device_url(host_url);
    device_url.append("cm?cmnd=");
    device_url.append(command);  // invalid characters in url are replaced by the URL class

    // send http status request
    HttpClient http_client;
    std::string response;
    std::string content;
    int http_return_code = http_client.sendHttpGetRequest(device_url, response, content);

    // check if the http return code is 200 OK
    if (http_return_code == 200) {
        // parse json content
        json_value* json = json_parse(content.c_str(), content.length());
        return json;
    }
    return NULL;
}


bool TasmotaAPI::compareNames(const std::string& name1, const std::string& name2, const bool strict) {

    // first check if both strings are identical
    if (name1 == name2) {
        return true;
    }

    // extract base names by removing any trailing digits
    std::string base_name1 = name1;
    std::string base_name2 = name2;
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
