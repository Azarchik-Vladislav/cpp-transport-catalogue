#include "request_handler.h"

using namespace domain;


using std::string_view;
using std::set;

std::optional<BusInfo> RequestHandler::GetBusInfo(string_view bus) const
{
    auto bus_info = db_.FindBus(bus);

    if(bus_info == nullptr) {
        return std::nullopt;
    }

    int stop_count = static_cast<int>(bus_info->stops_for_bus.size());
    int unique_stops = db_.ComputeUniqueStops(*bus_info);
    RouteDistanceInfo route_info = db_.ComputeRouteDistanceInfo(*bus_info);

    return BusInfo{bus, stop_count, unique_stops, route_info};
}

std::optional<set<string_view>> RequestHandler::GetStopInfo(string_view stop) const {
    bool check_stop_in_catalogue = db_.FindStop(stop);

    if(!check_stop_in_catalogue) {
        return std::nullopt;
    }

    return db_.FindBusesForStop(stop);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.CreateDocSVG(db_.GetBuses(), db_.GetStops());  
}
