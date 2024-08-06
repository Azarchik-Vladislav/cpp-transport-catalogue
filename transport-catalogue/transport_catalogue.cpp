#include "transport_catalogue.h"

using namespace std::literals;

using std::deque;
using std::move;
using std::set;
using std::string;
using std::string_view;
using std::vector;
using std::unordered_set;
using std::unordered_map;

using PairStops = std::pair<const Stop*,const Stop*>;

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

void TransportCatalogue::AddDistance(string_view stop_from, string_view stop_to, double distance) { 
    const auto ptr_stop_from = FindStop(stop_from);
    const auto ptr_stop_to = FindStop(stop_to);

    distance_between_stops_.insert({std::make_pair(ptr_stop_from, ptr_stop_to), distance});
}

const Stop* TransportCatalogue::FindStop(string_view name_stop) const {
    auto iter = stopname_to_stop_.find(name_stop);

    if(iter == stopname_to_stop_.end()) {
        return nullptr;
    }

    return iter->second;
}

const Bus* TransportCatalogue::FindBus(string_view name_bus) const {
    auto iter = busname_to_bus_.find(name_bus);
    if(iter == busname_to_bus_.end()) {
        return nullptr;
    }

    return iter->second;
}

double TransportCatalogue::FindDistance(const Stop* stop_from, const Stop* stop_to) const {
    auto iter_from_to = distance_between_stops_.find(PairStops{stop_from, stop_to});

    if(iter_from_to != nullptr) {
        return iter_from_to->second;
    }
    auto iter_to_from = distance_between_stops_.find(PairStops{stop_to, stop_from});

    return iter_to_from->second;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(string_view bus) const {
    auto bus_info = FindBus(bus);

    if(bus_info == nullptr) {
        return std::nullopt;
    }

    size_t stop_count = bus_info->stops_for_bus.size();
    size_t unique_stops = ComputeUniqueStops(*bus_info);
    RouteDistanceInfo route_info = ComputeRouteDistanceInfo(*bus_info);

    return BusInfo{bus, stop_count, unique_stops, route_info};
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

RouteDistanceInfo TransportCatalogue::ComputeRouteDistanceInfo(const Bus& bus) const {
    double real_distance = 0;
    double geo_distance = 0;

    const auto& stops_for_bus = bus.stops_for_bus;

    for(size_t i = 0, j = 1; j < bus.stops_for_bus.size(); ++i, ++j) {
        auto stop_from = stops_for_bus[i];
        auto stop_to = stops_for_bus[j];

        real_distance += FindDistance(stop_from, stop_to);
        geo_distance += ComputeDistance(stop_from->coordinates, stop_to->coordinates);
    }
    double curvature = real_distance / geo_distance;
    return RouteDistanceInfo{real_distance, curvature};
}
