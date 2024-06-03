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

struct Stop;

struct Bus {
	std::string name_bus;
    std::vector<const Stop*> stops_for_bus;
};

struct Stop {
    std::string name_stop;
    Coordinates coordinates;
};

struct BusInfo {
    std::string_view bus;
    size_t stops_on_route;
    size_t unique_stops;
    double route_length;
};

class TransportCatalogue {
public:

    void AddStop(const std::string& name_stop, const Coordinates& coordinates);
    void AddBus(const std::string& name_bus, const std::vector<std::string_view>& name_stops_for_bus);

    const Stop* FindStop(std::string_view name_stop) const; 
    const Bus* FindBus(std::string_view name_bus) const;

    std::optional<BusInfo> GetBusInfo(std::string_view bus) const;
    std::optional<std::set<std::string_view>> GetStopInfo(std::string_view stop) const;

private:
    std::deque<Bus> buses_;
    std::deque<Stop> stops_;
    std::unordered_map <std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map <std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map <std::string_view, std::set<std::string_view>> buses_for_stop_;

    size_t ComputeUniqueStops(const Bus& bus) const;
    double ComputeRouteLength(const Bus& bus) const;
};