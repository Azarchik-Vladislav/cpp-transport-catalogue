#pragma once

#include <algorithm>
#include <deque>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "geo.h"
#include "domain.h"

class TransportCatalogue {
public:
    void AddStop(const domain::Stop& stop);
    void AddBus(const std::string& name_bus, const std::vector<std::string_view>& name_stops_for_bus, bool is_roundtrip);
    void AddDistance(std::string_view stop_from, std::string_view stop_to, double distance);

    const std::set<std::string_view>FindBusesForStop(std::string_view name_stop) const;
    const domain::Stop* FindStop(std::string_view name_stop) const; 
    const domain::Bus* FindBus(std::string_view name_bus) const;
    std::optional<double> FindDistance(const domain::Stop* stop_from,const domain::Stop* stop_to) const;

    std::vector<const domain::Bus*> GetBuses(bool get_all_buses) const;
    std::vector<const domain::Stop*> GetStops(bool get_all_stops) const;

    size_t GetBusesCount() const;
    size_t GetStopsCount() const;
    
    domain::RouteDistanceInfo ComputeRouteDistanceInfo(const domain::Bus& bus) const;
    int ComputeUniqueStops (const domain::Bus& bus) const;

private:
struct DistanceBetweenStopsHash {    
        size_t operator() (const std::pair<const domain::Stop*,const domain::Stop*>& stops) const {
            const auto& [stop_from, stop_to] = stops;

            size_t hash_stop_from = stop_hasher_(stop_from);
            size_t hash_stop_to = stop_hasher_(stop_to);

            return 67 * (67 * hash_stop_from + hash_stop_to);
        }
    private:
        std::hash<const void*> stop_hasher_;
    };

    std::deque<domain::Bus> buses_;
    std::deque<domain::Stop> stops_;
    std::unordered_map <std::string_view, const domain::Stop*> stopname_to_stop_;
    std::unordered_map <std::string_view, const domain::Bus*> busname_to_bus_;
    std::unordered_map <std::string_view, std::set<std::string_view>> buses_for_stop_;
    std::unordered_map <std::pair<const domain::Stop*, const domain::Stop*>, double, DistanceBetweenStopsHash> distance_between_stops_;
};


