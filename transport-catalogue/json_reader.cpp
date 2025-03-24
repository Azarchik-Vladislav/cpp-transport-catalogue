#include "json_reader.h"

using namespace graph;
using namespace std::literals;
using namespace json;

using std::cerr;
using std::optional;
using std::set;
using std::string;
using std::string_view;
using std::vector;

void JSONReader::ApplyArrayOfColorCharacteristics(const string& key, const Node& value) {
    try {
        if(value.AsArray().size() == 2) {
            svg::Point point {value.AsArray()[0].AsDouble(),
                              value.AsArray()[1].AsDouble()};

            renderer_->ApplySetting(key, point);
        }
    } catch(std::logic_error& err) {
        cerr << "Is not Point: "s << err.what() << "\n";
    }

    try {
        if(value.AsArray().size() == 3) {
            svg::Color color(svg::Rgb{value.AsArray()[0].AsInt(),
                                      value.AsArray()[1].AsInt(),
                                      value.AsArray()[2].AsInt()});

            renderer_->ApplySetting(key,color);
        }

        if(value.AsArray().size() == 4) {
            svg::Color color(svg::Rgba{value.AsArray()[0].AsInt(),
                                       value.AsArray()[1].AsInt(),
                                       value.AsArray()[2].AsInt(),
                                       value.AsArray()[3].AsDouble()});

            renderer_->ApplySetting(key, color);
        }
    } catch(std::logic_error& err) {
        cerr << "Color format of RGB/RGBA mismatch: "s << err.what() << "\n";
    }
}

void JSONReader::ApplyCommandToStop(const Node& node) {
    std::string key_err;
    try {
        for(const auto& [key, value] : node.AsDict()){
            key_err = key;
            if(key == "type"s && value.AsString() != "Stop"s) {
                return;
            }

            if(key == "name"s) {
                stop.name_stop = value.AsString();
            }

            if(key == "latitude"s) {
                stop.coordinates.lat = value.AsDouble();
            }

            if(key == "longitude"s) {
                stop.coordinates.lng = value.AsDouble();
            }          
        } 
    } catch(const std::logic_error& err) {
        cerr << "Incorrect value of key\""s  << key_err << "\" "s << err.what() << '\n'; 
    } catch(const std::exception& err) {
        cerr << err.what() << '\n';
    } catch(...) {
        cerr << "Unknow exeption"s << '\n';
    }

    catalogue_->AddStop(stop);
}

void JSONReader::ApplyCommandToDistance(const Node& node) {
    string key_err;

    try {
        string stop_from;

        for(const auto& [key, value] : node.AsDict()){
            key_err = key;

            //Если не остановка выходим из функции
            if(key == "type"s && value.AsString() != "Stop"s) {
                return;
            }

            if(key == "name"s) {
                stop_from = value.AsString();
            }

            if(key == "road_distances"s) {
                for(const auto& [stop_to, distance] : value.AsDict()) {
                    catalogue_->AddDistance(stop_from, stop_to, distance.AsDouble());
                }
            }       
        } 
    } catch(const std::logic_error& err) {
        cerr << "Incorrect value for key\""s  << key_err << "\" "s << err.what() << '\n'; 
    } catch(const std::exception& err) {
        cerr << err.what() << '\n';
    } catch(...) {
        cerr << "Unknow exeption"s << '\n';
    }   
}

void JSONReader::ApplyCommandToBus(const json::Node& node) {
    string key_err;

    try {
        string name_bus;
        vector<string_view> buffer_of_stops;
        bool is_roundtrip = false;

        for(const auto& [key, value] : node.AsDict()) {
            key_err = key;

            //Если не маршрут выходим из функции
            if(key == "type"s && value.AsString() != "Bus"s) {
                return;
            }

            if(key == "name"s) {
                name_bus = value.AsString();
            }

            if(key == "stops"s) {
                for(const auto& _stop : value.AsArray()) {
                    buffer_of_stops.push_back(_stop.AsString());
                }
            }

            if(key == "is_roundtrip"s) {
                is_roundtrip = value.AsBool();
            }
        }

        if(!is_roundtrip) {
            buffer_of_stops.insert(buffer_of_stops.end(), std::next(buffer_of_stops.rbegin()), buffer_of_stops.rend());
        }

        catalogue_->AddBus(name_bus, buffer_of_stops, is_roundtrip);
    } catch(const std::logic_error& err) {
        cerr << "Incorrect value of key\""s  << key_err << "\" "s << err.what() << '\n'; 
    } catch(const std::exception& err) {
        cerr << err.what() << '\n';
    } catch(...) {
        cerr << "Unknow exeption"s << '\n';
    }
}

void JSONReader::ApplyCommandToBusInfo(const int id_request, const string &name_bus, Builder &JSON_builder) const {
    using namespace domain;

    optional<BusInfo> bus_info = request_handler->GetBusInfo(name_bus);

    JSON_builder.StartDict().Key("request_id"s).Value(id_request);

    if(bus_info == std::nullopt) {
        JSON_builder.Key("error_message"s).Value("not found"s); 
    } else {
        JSON_builder.Key("stop_count"s).Value(bus_info.value().stops_on_route)
                    .Key("unique_stop_count"s).Value(bus_info.value().unique_stops)
                    .Key("route_length"s).Value(bus_info.value().route_info.route_length)
                    .Key("curvature"s).Value(bus_info.value().route_info.route_curvature);
    }

    JSON_builder.EndDict();
}

void JSONReader::ApplyCommandToStopInfo(const int id_request, const std::string& name_stop, Builder& JSON_builder) const {
    using namespace domain;

    optional<set<string_view>> stop_info = request_handler->GetStopInfo(name_stop);

    JSON_builder.StartDict().Key("request_id"s).Value(id_request);

    if(stop_info == std::nullopt) {
        JSON_builder.Key("error_message"s).Value("not found"s);
    } else {
        JSON_builder.Key("buses"s).StartArray();
        
        for(const auto& bus_for_stop : stop_info.value()) {
            JSON_builder.Value(string {bus_for_stop});
        }
        JSON_builder.EndArray();
    }

    JSON_builder.EndDict();
}

void JSONReader::ApplyCommandToMapInfo(const int id_request, Builder& JSON_builder) const {
    using namespace domain;

    JSON_builder.StartDict().Key("request_id"s).Value(id_request);

    std::ostringstream strm;
    request_handler->RenderMap().Render(strm);
    JSON_builder.Key("map"s).Value(strm.str())
                .EndDict();
}

void JSONReader::ApplyCommandToRouteInfo(const int id_request, 
                                         const string& stop_from, 
                                         const string& stop_to, 
                                         Builder& JSON_builder) const {
    JSON_builder.StartDict().Key("request_id"s).Value(id_request);

    const auto route = router_->BuildOptimazedRoute(stop_from, stop_to);

    if(!route) {
        JSON_builder.Key("error_message"s).Value("not found"s);
    } else {
        JSON_builder.Key("items"s).StartArray();

        for(const auto& edge : route->edges) {
            JSON_builder.StartDict();

            if(edge->span_count == 0) {
                JSON_builder.Key("stop_name"s).Value(edge->name)
                            .Key("time"s).Value(edge->weight)
                            .Key("type"s).Value("Wait"s);
            } else {
                JSON_builder.Key("bus"s).Value(edge->name)
                            .Key("span_count"s).Value(static_cast<int>(edge->span_count))
                            .Key("time"s).Value(edge->weight)
                            .Key("type"s).Value("Bus"s);
            }
            JSON_builder.EndDict();
            
        } 
        JSON_builder.EndArray();
        JSON_builder.Key("total_time"s).Value(route->weight);
    }
    JSON_builder.EndDict();
}   

void JSONReader::LoadSettingsForRenderer() {
    const auto iter_command = doc_.GetRoot().AsDict().find("render_settings"s);

    if(iter_command == doc_.GetRoot().AsDict().end()) {
        return; 
    }

    const auto dict = iter_command->second.AsDict();

    if(dict.empty()) {
        return;
    }

    for(const auto& [key, value] : dict) {
        if(value.IsInt()) {
            renderer_->ApplySetting<int>(key, value.AsInt());
            continue;
        }

        if(value.IsPureDouble()) {
            renderer_->ApplySetting<double>(key, value.AsDouble());
            continue;
        }

        //Если массив, в котором любое из значений является IsDouble() - это RGB/RGBA
        if(value.IsArray() && value.AsArray()[0].IsDouble()) {
            ApplyArrayOfColorCharacteristics(key, value.AsArray());
            continue;
        }
        
        if(value.IsString()) {
            svg::Color color (value.AsString());
            renderer_->ApplySetting(key, color);
        }

        if(value.IsArray()) {
            for(const auto& elem : value.AsArray()) {

                if(elem.IsString()) {
                    svg::Color color (elem.AsString());
                    renderer_->ApplySetting(key, color);
                    continue;
                }

                if(elem.IsArray()) {
                    ApplyArrayOfColorCharacteristics(key, elem.AsArray());
                    continue; 
                }
                
                throw std::logic_error("Is not a color"s);
            }  
        }
    }
}

void JSONReader::LoadSettingsForRouter() {
    const auto iter_command = doc_.GetRoot().AsDict().find("routing_settings"s);

    if(iter_command == doc_.GetRoot().AsDict().end()) {
        return; 
    };

    const auto dict = iter_command->second.AsDict();

    if(dict.empty()) {
        return;
    }
    router::SettingsTransportRouter settings;

    for(const auto& [key, value] : dict) {
        if(key == "bus_wait_time"s) {
            settings.wait_time = value.AsInt();
        }
        
        if(key == "bus_velocity"s) {
            settings.velocity = value.AsDouble();
        }
    }

    router_ = std::make_unique<router::TransportRouter>(settings, *catalogue_);
}

void JSONReader::PrepareJSON(const Array& array_in, Builder& JSON_builder) const {
    using namespace json;

    JSON_builder.StartArray();

    string key_err;
    string type_request;
    string name_for_type;
    int id_request = 0;

    string stop_from;
    string stop_to;
    
    try {
        for(const auto& dict : array_in) {
            key_err = "stat_requests Dicts"s;

            for(const auto& [key, value] : dict.AsDict()) {
                key_err = key;

                if(key == "id"s) {
                    id_request = value.AsInt();
                }

                if(key == "type"s) {
                    type_request = value.AsString();
                }

                if(key == "name"s) {
                    name_for_type = value.AsString();
                } 

                if(key == "from"s) {
                    stop_from = value.AsString();
                }

                if(key == "to"s) {
                    stop_to = value.AsString();
                }
            }
       
            if(type_request == "Bus"s) {
                ApplyCommandToBusInfo(id_request, name_for_type, JSON_builder);
            } 

            if(type_request == "Stop"s) {
                ApplyCommandToStopInfo(id_request, name_for_type, JSON_builder);
            }

            if(type_request == "Map"s) {
                ApplyCommandToMapInfo(id_request, JSON_builder);
            } 
            
            if(type_request == "Route"s) {
                ApplyCommandToRouteInfo(id_request, stop_from, stop_to, JSON_builder);
            }
        }

         JSON_builder.EndArray(); 
    } catch(const std::logic_error& err) {
        cerr << "Incorrect value of key\""s  << key_err << "\" "s << err.what() << '\n'; 
    } catch(const std::exception& err) {
        cerr << err.what() << '\n';
    } catch(...) {
        cerr << "Unknow exeption"s << '\n';
    }
}

void JSONReader::LoadTransportCatalogue() {
    const auto iter_command = doc_.GetRoot().AsDict().find("base_requests"s);

    if(iter_command == doc_.GetRoot().AsDict().end()) {
        return; 
    }

    const Array* data_of_transport_catalogue = &iter_command->second.AsArray();
    
    for(const auto& element : *data_of_transport_catalogue) {
        ApplyCommandToStop(element);
    }

    for(const auto& element : *data_of_transport_catalogue) {
        ApplyCommandToDistance(element);
    }

    for(const auto& element : *data_of_transport_catalogue) {
        ApplyCommandToBus(element);
    }
}

void JSONReader::LoadSettings() {
    LoadSettingsForRenderer();
    LoadSettingsForRouter();
}

void JSONReader::PrintJSON(std::ostream& out) const {
    using namespace json;

    const auto iter_command = doc_.GetRoot().AsDict().find("stat_requests"s);

    if(iter_command == doc_.GetRoot().AsDict().end()) {
        return; 
    }
    
    Array array_in = iter_command->second.AsArray();
    if(array_in.empty()) {
        return;
    }

    Builder JSON_builder;
    PrepareJSON(array_in, JSON_builder);
    Print (Document(JSON_builder.Build()), out);
}
