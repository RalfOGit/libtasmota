#ifndef __LIBTASMOTA_TASMOTAAPI_HPP__
#define __LIBTASMOTA_TASMOTAAPI_HPP__

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
 *    notice, this list of conditionsand the following disclaimer in the
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

#include <string>
#include <vector>
#include <map>
#include <JsonCppWrapper.hpp>

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#else
namespace libtasmota {
#endif

    /**
     * Class implementing an API for tasmota devices.
     */
    class TasmotaAPI {

    protected:

        std::string host_url;

        std::string assembleHttpUrl(const std::string& command, const std::string& value = "") const;
        json_value* getJsonResponse(const std::string& command, std::string& http_status) const;
        static std::string getValueFromJson(const json_value* const json, const std::string& name);
        static bool compareNames(const std::string& name1, const std::string& name2, const bool strict);
        static std::vector<std::string> getPathSegments(const std::string& path);

    public:

        TasmotaAPI(const std::string& host_url);
        ~TasmotaAPI(void) {}

        // Get accessor methods.
        std::string getValue(const std::string& name) const;                        // e.g. "Module"
        std::string getValueFromPath(const std::string& path) const;                // e.g. "StatusSNS:ENERGY:Voltage"

        std::map<std::string, std::string> getModules(void) const;                  // get a vector of modules supported by the firmware

        // Set accessor methods.
        std::string setValue(const std::string& name, const std::string& value);    // e.g. "Power", can be used if name is well-known and documented

    };

}   // namespace libtasmota

#endif
