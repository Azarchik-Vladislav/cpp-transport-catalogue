#pragma once

#include "domain.h"
#include "graph.h"
#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "router.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"


#include <iostream>
#include <string>
#include <string_view>
#include <sstream>

class JSONReader {
public:
    explicit JSONReader(std::istream& input) 
                        :  doc_ (json::Load(input)) { 
        request_handler = std::make_unique<RequestHandler>(*catalogue_, *renderer_, *router_); 
    }

    void LoadTransportCatalogue();
    void LoadSettings();

    void PrintJSON(std::ostream& out) const;

private:
    std::unique_ptr<RequestHandler> request_handler = nullptr;
    std::unique_ptr<TransportCatalogue> catalogue_ = std::make_unique<TransportCatalogue>(); 
    std::unique_ptr<renderer::MapRenderer> renderer_ = std::make_unique<renderer::MapRenderer>();
    std::unique_ptr<router::TransportRouter> router_ = nullptr;

    json::Document doc_;
    domain::Stop stop;
    
    
    void ApplyArrayOfColorCharacteristics(const std::string& key, const json::Node& array);

    void ApplyCommandToStop(const json::Node& node);
    void ApplyCommandToDistance(const json::Node& node);
    void ApplyCommandToBus(const json::Node& node);

    void ApplyCommandToBusInfo(const int id_request, const std::string& name_bus, json::Builder&  JSON_builder) const;
    void ApplyCommandToStopInfo(const int id_request, const std::string& name_stop, json::Builder& JSON_builder) const;
    void ApplyCommandToMapInfo(const int id_request, json::Builder& JSON_builder) const;
    void ApplyCommandToRouteInfo(const int id_request, 
                                 const std::string& stop_from, 
                                 const std::string& stop_to,
                                 json::Builder& JSON_builder) const;

    void LoadSettingsForRenderer();
    void LoadSettingsForRouter();

    void PrepareJSON(const json::Array& array_in, json::Builder&) const;
};

