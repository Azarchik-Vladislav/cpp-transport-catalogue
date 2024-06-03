#include "transport_catalogue.h"

#include <iostream>

using namespace std::literals;

using std::deque;
using std::move;
using std::set;
using std::string;
using std::string_view;
using std::vector;
using std::unordered_set;
using std::unordered_map;

void TransportCatalogue::AddStop(const string& name_stop, const Coordinates& coordinates) {
    stops_.push_back({name_stop, coordinates});

    buses_for_stop_[stops_.back().name_stop];
    stopname_to_stop_[stops_.back().name_stop] = &stops_.back();
}

void TransportCatalogue::AddBus(const string& name_bus, const vector<string_view>& name_stops_for_bus) {
    vector<const Stop*> stops_for_bus;
    stops_for_bus.reserve(name_stops_for_bus.size());

    buses_.push_back({name_bus, stops_for_bus});
    busname_to_bus_[buses_.back().name_bus] = &buses_.back();

    for(const auto& stop : name_stops_for_bus) {
        stops_for_bus.push_back(FindStop(stop));
        buses_for_stop_.at(stop).insert(buses_.back().name_bus);
    }
    buses_.back().stops_for_bus = move(stops_for_bus);
}

const Stop* TransportCatalogue::FindStop(string_view name_stop) const{
    auto iter = stopname_to_stop_.find(name_stop);

    if(iter == stopname_to_stop_.end()){
        return nullptr;
    }

    return iter->second;
}

const Bus* TransportCatalogue::FindBus(string_view name_bus) const {
    auto iter = busname_to_bus_.find(name_bus);
    if(iter == busname_to_bus_.end()){
        return nullptr;
    }

    return iter->second;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(string_view bus) const {
    auto bus_info = FindBus(bus);

    if(bus_info == nullptr) {
        return std::nullopt;
    }

    size_t stop_count = bus_info->stops_for_bus.size();
    size_t unique_stops = ComputeUniqueStops(*bus_info);
    double route_length = ComputeRouteLength(*bus_info);

    return BusInfo{bus, stop_count, unique_stops, route_length};
}

std::optional<set<string_view>> TransportCatalogue::GetStopInfo(string_view stop) const {
    auto poinet_to_stop = buses_for_stop_.find(stop);
    if(poinet_to_stop == buses_for_stop_.end()) {
        return std::nullopt;
    }

    return poinet_to_stop->second;
}

size_t TransportCatalogue::ComputeUniqueStops(const Bus& bus) const {
    std::unordered_set<string_view> unique_stops;

    for(const auto& stop : bus.stops_for_bus) {
        unique_stops.insert(stop->name_stop);
    }

    return unique_stops.size();
}

double TransportCatalogue::ComputeRouteLength(const Bus& bus) const {
    double result = 0;

    for(size_t stop_from = 0, stop_to= 1; 
        stop_to < bus.stops_for_bus.size(); 
        ++stop_from, ++stop_to) {
        
        result += ComputeDistance(bus.stops_for_bus[stop_from]->coordinates,
                                  bus.stops_for_bus[stop_to]->coordinates);
    }
    return result;
}
