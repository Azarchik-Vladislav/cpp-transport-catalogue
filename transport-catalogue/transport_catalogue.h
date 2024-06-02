#pragma once

#include "geo.h"

#include <algorithm>
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std::literals;

using std::deque;
using std::set;
using std::string;
using std::string_view;
using std::vector;
using std::unordered_set;
using std::unordered_map;

struct Stop;

struct Bus {
	string name_bus_;
    vector <const Stop*> stops_for_bus_;
};

struct Stop {
    string name_stop_;
    Coordinates coordinates_;
};

struct BusInfo {
    string_view bus;
    size_t stops_on_route;
    size_t unique_stops;
    double route_length;
};

class TransportCatalogue {
public:

    void AddStop(string name_stop, Coordinates coordinates);
    void AddBus(string name_bus, vector<string_view>);

    const Stop* FindStop(string_view name_stop) const;
    const Bus* FindBus(string_view name_bus) const;

    std::optional<BusInfo> GetBusInfo(string_view bus) const;
    std::optional<set<string_view>> GetStopInfo(string_view stop) const;

private:
    deque<Bus> buses_;
    deque<Stop> stops_;
    unordered_map <string_view, const Stop*> stopname_to_stop;
    unordered_map <string_view, const Bus*> busname_to_bus;

    size_t ComputeUniqueStops(const Bus& bus) const;
    double ComputeRouteLength(const Bus& bus) const;

};