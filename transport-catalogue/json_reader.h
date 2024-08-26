#pragma once

#include "domain.h"
#include "json.h"
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

    json::Dict ApplyCommandToBusInfo(const int id_request, const std::string& name_bus) const;
    json::Dict ApplyCommandToStopInfo(const int id_request, const std::string& name_stop) const;
    json::Dict ApplyCommandToMapInfo(const int id_request) const;

    void PrepareJSON(const json::Array& array_in, json::Array& array_out) const;
};

