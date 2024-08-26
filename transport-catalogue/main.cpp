#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"


#include <iostream>
#include <cstdlib>
using namespace std;

int main() {
    JSONReader json(std::cin);

    json.LoadTransportCatalogue();
    json.LoadSettings();
    json.PrintJSON();

    return 0;
}