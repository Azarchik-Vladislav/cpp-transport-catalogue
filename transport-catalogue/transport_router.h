#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <optional>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>

namespace router {
struct SettingsTransportRouter {
    int wait_time;
    double velocity;
};

class TransportRouter {
public:
    TransportRouter() = default;
    TransportRouter(const SettingsTransportRouter& settings,
                    const TransportCatalogue& catalogue)
                    : catalogue_(catalogue),
                      route_graph_(catalogue.GetStopsCount() * 2),
                      settings_(settings) {
        CreateGraph();
        router_ = std::make_unique<graph::Router<double>>(route_graph_);
    };

    const std::optional<graph::Router<double>::RouteInfo> BuildOptimazedRoute(std::string_view from,
                                                                              std::string_view to) const;   
    const graph::DirectedWeightedGraph<double>& GetGraph() const;


private:
    const TransportCatalogue& catalogue_;

    std::unordered_map<std::string, graph::VertexId> start_routes_id_; 
    graph::DirectedWeightedGraph<double> route_graph_;
    SettingsTransportRouter settings_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;

    //Создает весовую матрицу для маршрутов
    std::vector<std::vector<double>> ComputeWeightForEdges(const std::vector<const domain::Stop*> stops);

    void AddWaitEdges(const std::vector<const domain::Stop*>& stops);
    void AddBusEdges(const std::vector<std::vector<double>>& weight_for_edges,
                                           const std::vector<const domain::Stop*>& stops, 
                                           const domain::Bus& bus);

    void CreateGraph();   
};
}//namespace router