#include "transport_catalogue.h"

using namespace std::literals;
using namespace domain;

using geo::Coordinates;

using std::deque;
using std::move;
using std::set;
using std::string;
using std::string_view;
using std::vector;
using std::unordered_set;
using std::unordered_map;

using PairStops = std::pair<const Stop*,const Stop*>;

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);

    buses_for_stop_[stops_.back().name_stop];
    stopname_to_stop_[stops_.back().name_stop] = &stops_.back();
}

void TransportCatalogue::AddBus(const string& name_bus, const vector<string_view>& name_stops_for_bus, bool is_roundtrip) {
    vector<const Stop*> stops_for_bus;
    stops_for_bus.reserve(name_stops_for_bus.size());

    buses_.push_back({name_bus, stops_for_bus, is_roundtrip});
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

const set<string_view> TransportCatalogue::FindBusesForStop(std::string_view name_stop) const {
    auto iter = buses_for_stop_.find(name_stop);
    if(iter == buses_for_stop_.end()) {
        throw std::invalid_argument("Stop not in the catalogue"s);
    }
    return buses_for_stop_.find(name_stop)->second;
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

std::optional<double> TransportCatalogue::FindDistance(const Stop* stop_from, const Stop* stop_to) const{
    auto iter_from_to = distance_between_stops_.find(PairStops{stop_from, stop_to});
    if(iter_from_to != nullptr) {
        return iter_from_to->second;
    }

    auto iter_to_from = distance_between_stops_.find(PairStops{stop_to, stop_from});
    if(iter_to_from != nullptr) {
        return iter_to_from->second;
    }
    
    return std::nullopt;
}

//false - предоставить сведения о машрутах, в которой есть хоят бы одна остановка
//true - предоставить сведения о всех маршрутах
vector<const Bus*> TransportCatalogue::GetBuses(bool get_all_buses) const {
    vector<const Bus*> buses;

    for(const auto& bus : buses_) {

        if(!get_all_buses && bus.stops_for_bus.empty()){
            continue;
        }
        buses.push_back(&bus);
    }

    return buses;
}

//false - предоставить сведения об остановках, через которые проходит хотя бы один маршрут
//true - предоставить сведения о всех остановках
std::vector<const domain::Stop*> TransportCatalogue::GetStops(bool get_all_stops) const {
    vector<const Stop*> stops;
    for(const auto& stop : stops_) {
        
        if(!get_all_stops && FindBusesForStop(stop.name_stop).empty()) {
            continue;
        }
        stops.push_back(&stop);
    }

    return stops;
}

size_t TransportCatalogue::GetBusesCount() const {
    return buses_.size();
}

size_t TransportCatalogue::GetStopsCount() const {
    return stops_.size();
}

int TransportCatalogue::ComputeUniqueStops(const Bus& bus) const {
        std::vector<const Stop*> unique_stops = bus.stops_for_bus;
        std::sort(unique_stops.begin(), unique_stops.end());
        unique_stops.erase(std::unique(unique_stops.begin(), unique_stops.end()), unique_stops.end());

        return static_cast<int>(unique_stops.size()); 
    }

    RouteDistanceInfo TransportCatalogue::ComputeRouteDistanceInfo(const Bus& bus) const {
        double real_distance = 0;
        double geo_distance = 0;

        const auto& stops_for_bus = bus.stops_for_bus;

        for(size_t i = 0, j = 1; j < bus.stops_for_bus.size(); ++i, ++j) {
            auto stop_from = stops_for_bus[i];
            auto stop_to = stops_for_bus[j];

            auto distance = FindDistance(stop_from, stop_to);
            if(distance){
                real_distance += *distance;
            }
            
            geo_distance += ComputeDistance(stop_from->coordinates, stop_to->coordinates);
        }
        double curvature = real_distance / geo_distance;
        return RouteDistanceInfo{real_distance, curvature};
    }