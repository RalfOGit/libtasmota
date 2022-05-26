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


/**
 * Class implementing an API for tasmota devices.
 */

namespace libtasmota {

    class TasmotaAPI {

    protected:

        std::string host_url;

        json_value* getJsonResponse(const std::string& command);
        static bool compareNames(const std::string& name1, const std::string& name2, const bool strict);
        static std::vector<std::string> getPathSegments(const std::string& path);

    public:

        TasmotaAPI(const std::string& host_url);
        ~TasmotaAPI(void);

        // Get accessor methods.
        std::string getValue(const std::string& key);                               // e.g. "Module"
        std::string getValueFromPath(const std::string& path);                      // e.g. "StatusSNS:ENERGY:Voltage"

        std::map<std::string, std::string> getModules(void);                        // get a vector of modules supported by the firmware
        int getPower(void);

        // Set accessor methods.
        std::string setValue(const std::string& key, const std::string& value);     // e.g. "Voltage", can be used if key is unique

    };

}   // namespace libtasmota

#endif