#pragma once

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"


#include <iostream>
#include <string>
#include <string_view>
#include <sstream>

class JSONReader {
public:
    explicit JSONReader(std::istream& input) 
                        : request_handler(catalogue_, renderer_),
                          doc_ (json::Load(input)) {  
    }

    void LoadTransportCatalogue();
    void LoadSettings();

    void PrintJSON() const;

private:
    RequestHandler request_handler;
    TransportCatalogue catalogue_;
    renderer::MapRenderer renderer_;

    json::Document doc_;
    domain::Stop stop;
    
    
    void ApplyArrayOfColorCharacteristics(const std::string& key, const json::Node& array);

    void ApplyCommandToStop(const json::Node& node);
    void ApplyCommandToDistance(const json::Node& node);
    void ApplyCommandToBus(const json::Node& node);

    void ApplyCommandToBusInfo(const int id_request, const std::string& name_bus, json::Builder&  JSON_builder) const;
    void ApplyCommandToStopInfo(const int id_request, const std::string& name_stop, json::Builder& JSON_builder) const;
    void ApplyCommandToMapInfo(const int id_request, json::Builder& JSON_builder) const;

    void PrepareJSON(const json::Array& array_in, json::Builder&) const;
};

