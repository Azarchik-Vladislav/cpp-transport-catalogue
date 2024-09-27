#include "transport_router.h"

using namespace domain;
using namespace graph;
using namespace std::literals;

using std::optional;
using std::string;
using std::string_view;
using std::vector;

static const double CONVERT_COEF = 60. / 1000.;

namespace router {
const optional<Router<double>::RouteInfo> TransportRouter::BuildOptimazedRoute(string_view from, 
                                                                               string_view to) const {
    return router_->BuildRoute(start_routes_id_.at(string(from)), start_routes_id_.at(string(to)));
}

vector<vector<double>> TransportRouter::ComputeWeightForEdges(const vector<const Stop*>& stops) {
    vector<vector<double>> result;

    //Заранее создаем матрицу, размером равным количеству остановок в маршруте
    result.resize(stops.size(), vector<double>(stops.size()));

    //Заполняем вес в галвную диагональ
    for(size_t identical_stop = 0; identical_stop < stops.size(); ++identical_stop) {
        auto distance_from_to = catalogue_.FindDistance(stops[identical_stop], stops[identical_stop]);
        if(!distance_from_to){
            continue;
        }

        result[identical_stop][identical_stop] = (*distance_from_to / settings_.velocity) * CONVERT_COEF;
    }

    //Заполняем вес для соседних остановок в маршруте
    for(size_t stop_from = 0, stop_to = 1; stop_to < stops.size(); ++stop_from, ++stop_to) {
        auto distance_from_to = catalogue_.FindDistance(stops[stop_from], stops[stop_to]);
        if(!distance_from_to) {
            continue;
        }
        result[stop_from][stop_to] = (*distance_from_to / settings_.velocity) * CONVERT_COEF;

        auto distance_to_from = catalogue_.FindDistance(stops[stop_to], stops[stop_from]);
        if(!distance_to_from) {
            continue;
        }
        result[stop_to][stop_from] = (*distance_to_from / settings_.velocity) * CONVERT_COEF;
    }

    //Заполняем остальные остановки, посредством сложения веса смежных маршрутов
    //Например 0->5  = 0->4 + 4->5, т.е время на проезд от остановки 0 до 5 равно
    //сумме на путешествие от 0 остновки до 4 и от 4 до 5 без ожидания
    for(size_t i = 0; i < stops.size(); ++i) {
        for(size_t j = 2 + i; j < stops.size(); ++j) {
            result[i][j] = result[i][j-1] + result[j-1][j];
            result[j][i] = result[j-1][i] + result[j][j-1]; 
        }
    }

    return result;
}

void TransportRouter::AddWaitEdges(const vector<const Stop*>& stops) {
    VertexId current_id = 0;
    VertexId next_id = current_id + 1;
    for(const auto& stop : stops) {
        start_routes_id_.insert({stop->name_stop, current_id});
        route_graph_.AddEdge(BuildEdge<double>()
                                      .SetName(stop->name_stop)
                                      .SetIdFrom(current_id++)
                                      .SetIdTo(next_id++)
                                      .SetWeight(settings_.wait_time)
                                      .Build());
        ++current_id;
        ++next_id;
    }
}

void TransportRouter::AddBusEdges() {
    for(const auto& bus : catalogue_.GetBuses(true)) {                               
        const vector<const Stop*>& stops = bus->stops_for_bus;
        const auto weight_for_edges = ComputeWeightForEdges(stops);

        for(size_t identical_stop = 0; identical_stop < stops.size(); ++identical_stop) {
            if(weight_for_edges[identical_stop][identical_stop] != 0){
                route_graph_.AddEdge(BuildEdge<double>()
                                     .SetName(bus->name_bus)
                                     .SetIdFrom(start_routes_id_.at(stops[identical_stop]->name_stop))
                                     .SetIdTo(start_routes_id_.at(stops[identical_stop]->name_stop))
                                     .SetWeight(weight_for_edges[identical_stop][identical_stop])
                                     .Build()
                                    );
            }
        }   

        for(size_t stop_from = 0; stop_from < stops.size(); ++stop_from) {
            for(size_t stop_to = stop_from + 1; stop_to < stops.size(); ++stop_to) {
                route_graph_.AddEdge(BuildEdge<double>()
                                     .SetName(bus->name_bus)
                                     .SetSpanCount(stop_to - stop_from)
                                     .SetIdFrom(start_routes_id_.at(stops[stop_from]->name_stop) + 1)
                                     .SetIdTo(start_routes_id_.at(stops[stop_to]->name_stop))
                                     .SetWeight(weight_for_edges[stop_from][stop_to])
                                     .Build()
                                     );
            }
        }
    }
}

void TransportRouter::CreateGraph() {
    AddWaitEdges(catalogue_.GetStops(true));     
    AddBusEdges();
}
}//namespace router