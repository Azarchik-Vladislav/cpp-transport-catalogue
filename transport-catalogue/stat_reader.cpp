#include "stat_reader.h"

using namespace std::literals;

using std::ostream;
using std::optional;
using std::set;
using std::string_view;
using std::vector;

ostream& operator<<(ostream& out, const BusInfo& bus_info) {
    out << "Bus "s << bus_info.bus << ": "s;
    out << bus_info.stops_on_route << " stops on route, "s;
    out << bus_info.unique_stops << " unique stops, "s;
    out << std::setprecision(6) << bus_info.route_length << " route length"s;

    return out;
}

std::pair<string_view, string_view> ParseQuery(string_view request) {
    auto space_pos = request.find_first_of(' ');
    if(space_pos == request.npos){
        return {};
    }
    
    string_view command_query = request.substr(0, space_pos);

    request.remove_prefix(space_pos);
    auto name_query_pos = request.find_first_not_of(' ');
    string_view name_for_query = request.substr(name_query_pos);

    return {command_query, name_for_query};
}

void PrintBusInfo(const optional<BusInfo>& bus_info, string_view name_for_query, 
                      ostream& output) {
    if(bus_info.has_value()) {
        output << bus_info.value() << std::endl;
    } else {
        output << "Bus "s << name_for_query << ": "s
               <<"not found"s << std::endl;
    }
}

void PrintStopInfo(const optional<set<string_view>>& stop_info, string_view name_for_query, 
                   ostream& output) {
    if(stop_info.has_value()) {
        output << "Stop "s << name_for_query <<": "s;

        if(stop_info.value().empty()) {
            output << "no buses"s << std::endl;;
        } else {
            output << "buses "s;

            bool is_first = true;

            for(const auto& name_bus : stop_info.value()) {
                if(is_first) {
                    output << name_bus;
                    is_first = false;
                    continue;
                }
                output << " "s  << name_bus;
            }
            output << std::endl;
        } 
    } else {
        output << "Stop "s << name_for_query <<": "s;
        output << "not found"s << std::endl;
    }
}

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, string_view request,
                       std::ostream& output) {
    const auto [command_query, name_for_query] = ParseQuery(request);

    if(command_query == "Bus"s) {
        auto bus_info = transport_catalogue.GetBusInfo(name_for_query);
        PrintBusInfo(bus_info, name_for_query, output);
    }

    if(command_query == "Stop"s) {
        auto stop_info = transport_catalogue.GetStopInfo(name_for_query);
        PrintStopInfo(stop_info, name_for_query, output);
    }

}