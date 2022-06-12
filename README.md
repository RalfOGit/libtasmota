# libtasmota
A C++ library to access tasmota devices.

It provides methods to get and set all defined properties of a tasmota device via the local http network api. The API is coded in class TasmotaAPI. Usage of this class is rather straightforward:

        TasmotaAPI api("http://192.168.178.117/");          // configure the tasmota device, "http://tasmota-123456-7890/" would also work
        
        std::string val1 = api.getValue("Module");          // get name of the tasmota device
        std::string val2 = api.getValue("AP");              // get name of the wifi access point
        
        std::string val3 = api.getValue("Power");           // get the power switch state
        std::string val4 = api.setValue("Power", "OFF");    // switch power off
        std::string val5 = api.setValue("Power", "ON");     // switch power on
        
        std::string val6 = api.getValueFromPath("StatusSNS:ENERGY:Power");  // get the amount of active power passing through the tasmota plug

        std::map<std::string, std::string> map = api.getModules();  // get a map of all tasmota devices supported by the firmware

In the example above, getValue("AP") is invoking "http://192.168.178.117/cm?cmnd=AP" to get the name of the wifi access point. Of course, you can use any other command that is understood by your tasmota device.
A call to setValue("Power", "OFF") is invoking "http://192.168.178.117/cm?cmnd=Power%20OFF" to switch off the given tasmota plug. If successful, it typically returns the the result of the operation, i.e. "OFF" in this case.

Method getValueFromPath("StatusSNS:ENERGY:Power") internally invokes "http://192.168.178.117/cm?cmnd=STATUS%200" to get the full status report tree data structure from the tasmota device. It then traverses the status json response tree by following the path description until it finally gets the "Power" value. In this context, power refers to the amount of active power passing through the tasmota plug.

In case of errors, the result string, may contain error information. To help distinguish error reports from the expected return values, error reports will always start with "HTTP-Returncode:":

        "HTTP-Returncode: 200 : {\"Command\":\"Unknown\"}"  => indicating an unknown command has been received by the tasmota device
        "HTTP-Returncode: -1 : "                            => typically indicating some kind of networking error

Libtasmota is self-contained, i.e. it does not have any external library dependencies. Cudos to the very small footprint json parser written by James McLaughlin: https://github.com/udp/json-parser, which is included in the library.

The simplest way to build this library together with your code is to checkout this library into a separate folder and use unix symbolic links (ln -s ...) or ntfs junctions (mklink /J ...) to integrate it as a sub-folder within your projects folder.

For example, if you are developing on a Windows host and your projects reside in C:\workspaces:

    cd C:\workspaces
    mkdir libtasmota
    git clone https://github.com/RalfOGit/libtasmota
    cd ..\YOUR_PROJECT_FOLDER
    mklink /J libtasmota ..\libtasmota
    Now you can start Visual Studio
    And in Visual Studio open folder YOUR_PROJECT_FOLDER

And if you are developing on a Linux host and your projects reside in /home/YOU/workspaces:

    cd /home/YOU/workspaces
    mkdir libtasmota
    git clone https://github.com/RalfOGit/libtasmota
    cd ../YOUR_PROJECT_FOLDER
    ln -s ../libtasmota
    Now you can start VSCode
    And in VSCode open folder YOUR_PROJECT_FOLDER

The source code contains doxygen comments, so that you can generate documentation for the library.

For now, libtasmota supports just plain http as the underlying network protocol. There is neither authentication nor encryption support.

Keep in mind, the software comes as is. No warrantees whatsoever are given and no responsibility is assumed in case of failure or damage being caused.

The code has been tested against the following environment:

        OS: CentOS 8(TM), IDE: VSCode (TM)
        OS: Windows 10(TM), IDE: Visual Studio Community Edition 2019 (TM)
