#pragma once

#include <iostream>
#include <iomanip>
#include <set>
#include <string_view>
#include <vector>


#include "transport_catalogue.h"

using std::ostream;
using std::set;
using std::string_view;
using std::vector;

ostream& operator<<(ostream& out, const BusInfo& bus_info);

std::pair<string_view, string_view> ParseQuery(const string_view request);

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);