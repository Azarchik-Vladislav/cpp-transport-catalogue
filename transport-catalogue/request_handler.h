#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <optional>
#include <set>
#include <string_view>

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db),
                                                                                          renderer_(renderer) {
    }

    std::optional<domain::BusInfo> GetBusInfo(std::string_view bus) const;
    std::optional<std::set<std::string_view>> GetStopInfo(std::string_view stop) const;

    svg::Document RenderMap() const;

private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
