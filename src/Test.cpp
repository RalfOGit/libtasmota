#if 1
#include <TasmotaAPI.hpp>
using namespace libtasmota;

int main(int argc, char** argv) {

    // configure tasmota API
    TasmotaAPI api1("http://192.168.178.117/");
    TasmotaAPI api2("http://192.168.178.118/");

    std::string val1 = api2.getValueFromPath("StatusSNS:ENERGY:Power");
    std::map<std::string, std::string> map = api2.getModules();

    std::string val2 = api2.getValue("Module0");
    std::string val3 = api2.getValue("Module1");
    std::string val4 = api2.getValue("AP");

    std::string val5 = api2.getValue("Power");
    std::string val6 = api2.setValue("Power", "OFF");
    std::string val7 = api2.setValue("Power", "ON");

    return 0;
}
#endif
