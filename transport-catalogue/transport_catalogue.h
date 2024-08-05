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
    std::unordered_map <std::string, double> distance_to_stop;
};

struct RouteDistanceInfo {
    double route_length;
    double route_curvature;
};

struct BusInfo {
    std::string_view bus;
    size_t stops_on_route;
    size_t unique_stops;
    RouteDistanceInfo route_info;
};

class TransportCatalogue {
public:

    void AddStop(const std::string& name_stop, const Coordinates& coordinates,
                                               std::unordered_map<std::string, double>&& distance_to_stop);
    void AddBus(const std::string& name_bus, const std::vector<std::string_view>& name_stops_for_bus);
    void AddDistance(std::string_view stop_from);

    const Stop* FindStop(std::string_view name_stop) const; 
    const Bus* FindBus(std::string_view name_bus) const;
    double FindDistance(const Stop*,const Stop*) const;

    std::optional<BusInfo> GetBusInfo(std::string_view bus) const;
    std::optional<std::set<std::string_view>> GetStopInfo(std::string_view stop) const;

private:
struct DistanceBetweenStopsHash {    
        size_t operator() (const std::pair<const Stop*,const Stop*>& stops) const {
            const auto& [stop_from, stop_to] = stops;

            size_t hash_stop_from = stop_hasher_(stop_from);
            size_t hash_stop_to = stop_hasher_(stop_to);

            return 67 * (67 * hash_stop_from + hash_stop_to);
        }
    private:
        std::hash<const void*> stop_hasher_;
    };

    std::deque<Bus> buses_;
    std::deque<Stop> stops_;
    std::unordered_map <std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map <std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map <std::string_view, std::set<std::string_view>> buses_for_stop_;
    std::unordered_map <std::pair<const Stop*, const Stop*>, double, DistanceBetweenStopsHash> distance_between_stops_;
    
    RouteDistanceInfo ComputeRouteDistanceInfo(const Bus& bus) const;
    size_t ComputeUniqueStops(const Bus& bus) const;
    
};