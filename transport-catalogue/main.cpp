#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"


#include <iostream>

using namespace std;

int main() {
    JSONReader json(std::cin);

    json.LoadTransportCatalogue();
    json.LoadSettings();
    json.PrintJSON(std::cout);

    return 0;
}   