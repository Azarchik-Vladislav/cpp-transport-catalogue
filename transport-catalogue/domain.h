#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
namespace domain {
struct Stop;

struct Bus {
	std::string name_bus;
    std::vector<const Stop*> stops_for_bus;
    bool is_roundtrip;
};

struct Stop {
    std::string name_stop;
    geo::Coordinates coordinates;
};

struct RouteDistanceInfo {
    double route_length;
    double route_curvature;
};

struct BusInfo {
    std::string_view bus;
    int stops_on_route;
    int unique_stops;
    RouteDistanceInfo route_info;
};
} //namespace domain