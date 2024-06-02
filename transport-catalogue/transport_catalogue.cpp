#include "transport_catalogue.h"

#include <iostream>
using std::move;

void TransportCatalogue::AddStop(string name_stop, Coordinates coordinates) {
    
    stops_.push_back({move(name_stop), move(coordinates)});
    
    stopname_to_stop[stops_.back().name_stop_] = &stops_.back();
}



void TransportCatalogue::AddBus(string name_bus, vector<string_view> name_stops_for_bus) {
    vector<const Stop*> stops_for_bus;
    stops_for_bus.reserve(name_stops_for_bus.size());

    buses_.push_back({move(name_bus), move(stops_for_bus)});
    busname_to_bus[buses_.back().name_bus_] = &buses_.back();

    for(const auto& stop : name_stops_for_bus) {
        stops_for_bus.push_back(FindStop(stop));
    }
    buses_.back().stops_for_bus_ = std::move(stops_for_bus);
}

const Stop* TransportCatalogue::FindStop(string_view name_stop) const{
    auto iter = stopname_to_stop.find(name_stop);

    if(iter == stopname_to_stop.end()){
        return nullptr;
    }

    return iter->second;
}

const Bus* TransportCatalogue::FindBus(string_view name_bus) const {
    auto iter = busname_to_bus.find(name_bus);
    if(iter == busname_to_bus.end()){
        return nullptr;
    }

    return iter->second;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(string_view bus) const {
    auto bus_info = FindBus(bus);

    if(bus_info == nullptr){
        return std::nullopt;
    }

    size_t stop_count = bus_info->stops_for_bus_.size();
    size_t unique_stops = ComputeUniqueStops(*bus_info);
    double route_length = ComputeRouteLength(*bus_info);

    return BusInfo{bus, stop_count, unique_stops, route_length};
}

std::optional<set<string_view>> TransportCatalogue::GetStopInfo(string_view stop) const {
    auto poinet_to_stop = FindStop(stop);
    if(poinet_to_stop == nullptr) {
        return std::nullopt;
    }

    set<string_view> buses_to_stop;

    for(const auto& bus : busname_to_bus) {
        auto begin = bus.second->stops_for_bus_.begin();
        auto end = bus.second->stops_for_bus_.end();
        auto iter_to_stop = std::find(begin, end, poinet_to_stop);

        if(iter_to_stop == end){
            continue;
        }

        buses_to_stop.insert(bus.second->name_bus_); 
    }

    return buses_to_stop;
}

size_t TransportCatalogue::ComputeUniqueStops(const Bus& bus) const {
    std::unordered_set<string_view> unique_stops;

    for(const auto& stop : bus.stops_for_bus_) {
        unique_stops.insert(stop->name_stop_);
    }

    return unique_stops.size();
}

double TransportCatalogue::ComputeRouteLength(const Bus& bus) const {
    double result = 0;

    for(size_t stop_from = 0, stop_to= 1; 
        stop_to < bus.stops_for_bus_.size(); 
        ++stop_from, ++stop_to) {
        
        result += ComputeDistance(bus.stops_for_bus_[stop_from]->coordinates_,
                                  bus.stops_for_bus_[stop_to]->coordinates_);
    }
    return result;
}
