#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <optional>
#include <set>
#include <string_view>

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::TransportRouter& tr) 
                    : db_(db),
                      renderer_(renderer),
                      tr_(tr) {
    }

    std::optional<domain::BusInfo> GetBusInfo(std::string_view bus) const;
    std::optional<std::set<std::string_view>> GetStopInfo(std::string_view stop) const;

    svg::Document RenderMap() const;
    std::optional<graph::Router<double>::RouteInfo> BuildOptimasedRoute(const std::string_view stop_from, 
                                                                        const std::string_view stop_to) const;
    

private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const router::TransportRouter& tr_;
};
