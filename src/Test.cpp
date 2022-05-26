#if 1
#include <TasmotaAPI.hpp>
using namespace libtasmota;

int main(int argc, char** argv) {

    // configure go-eCharger API
    TasmotaAPI api1("http://192.168.178.117/");
    TasmotaAPI api2("http://192.168.178.118/");

//    api.refreshMap();
    api2.getPower();
    api2.getValueFromPath("StatusSNS:ENERGY:Power");
    api2.getModules();

    api2.getValue("Module0");
    api2.getValue("Module1");
    api2.getValue("AP");

    return 0;
}
#endif
