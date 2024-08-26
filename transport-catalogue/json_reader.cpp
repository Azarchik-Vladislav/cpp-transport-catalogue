#include "json_reader.h"

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
        //Если размер равен 2, то это точка.
        if(value.AsArray().size() == 2) {
        svg::Point point {value.AsArray()[0].AsDouble(),
                          value.AsArray()[1].AsDouble()};

        renderer_.ApplySetting(key, point);
        }
    } catch (std::logic_error& err) {
        cerr << "Is not Point: "s << err.what() << "\n";
    }

    try{
         //Если размер равен 3 - rbg, 4 - rgba
        if(value.AsArray().size() == 3) {
            svg::Color color(svg::Rgb{value.AsArray()[0].AsInt(),
                                       value.AsArray()[1].AsInt(),
                                       value.AsArray()[2].AsInt()});

            renderer_.ApplySetting(key,color);
        }

        if(value.AsArray().size() == 4) {
            svg::Color color(svg::Rgba{value.AsArray()[0].AsInt(),
                                 value.AsArray()[1].AsInt(),
                                 value.AsArray()[2].AsInt(),
                                 value.AsArray()[3].AsDouble()});

            renderer_.ApplySetting(key, color);
        }
    } catch (std::logic_error& err) {
        cerr << "Color format of RGB/RGBA mismatch: "s << err.what() << "\n";
    }
}

void JSONReader::ApplyCommandToStop(const Node& node)
{
    std::string key_err;
    try {
        for(const auto& [key, value] : node.AsMap()){
            key_err = key;
            //Если не остановка выходим из цикла
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

    catalogue_.AddStop(stop);
}

void JSONReader::ApplyCommandToDistance(const Node &node) {
    string key_err;

    try {
        string stop_from;

        for(const auto& [key, value] : node.AsMap()){
            key_err = key;

            //Если не остановка выходим из функции
            if(key == "type"s && value.AsString() != "Stop"s) {
                return;
            }

            if(key == "name"s) {
                stop_from = value.AsString();
            }

            if(key == "road_distances"s) {
                for(const auto& [stop_to, distance] : value.AsMap()) {
                    catalogue_.AddDistance(stop_from, stop_to, distance.AsDouble());
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

        for(const auto& [key, value] : node.AsMap()) {
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

        catalogue_.AddBus(name_bus, buffer_of_stops, is_roundtrip);
    } catch(const std::logic_error& err) {
        cerr << "Incorrect value of key\""s  << key_err << "\" "s << err.what() << '\n'; 
    } catch(const std::exception& err) {
        cerr << err.what() << '\n';
    } catch(...) {
        cerr << "Unknow exeption"s << '\n';
    }

}
Dict JSONReader::ApplyCommandToBusInfo(const int id_request, const string& name_bus) const {
    using namespace domain;

    Dict response_to_request;
    optional<BusInfo> bus_info = request_handler.GetBusInfo(name_bus);

    response_to_request.insert({"request_id"s, Node(id_request)});

    if(bus_info == std::nullopt) { 
        response_to_request.insert({"error_message"s, Node("not found"s)});   
    } else {
        response_to_request.insert({"stop_count"s, Node(bus_info.value().stops_on_route)});
        response_to_request.insert({"unique_stop_count"s, Node(bus_info.value().unique_stops)});
        response_to_request.insert({"route_length"s, Node(bus_info.value().route_info.route_length)});
        response_to_request.insert({"curvature"s, Node(bus_info.value().route_info.route_curvature)});
    }

    return response_to_request;
}

Dict JSONReader::ApplyCommandToStopInfo(const int id_request, const std::string& name_stop) const {
    using namespace domain;

    Dict response_to_request;
    optional<set<string_view>> stop_info = request_handler.GetStopInfo(name_stop);

    response_to_request.insert({"request_id"s, Node(id_request)});

    if(stop_info == std::nullopt) {
        response_to_request.insert({"error_message"s, Node("not found"s)});
    } else {
        Array buses_for_stop;
        
        for(const auto& bus_for_stop : stop_info.value()) {
            Node node_bus (string {bus_for_stop});
            buses_for_stop.push_back(node_bus);
        }
        
        response_to_request.insert({"buses"s, Node(buses_for_stop)});
    }

    return response_to_request;
}

Dict JSONReader::ApplyCommandToMapInfo(const int id_request) const {
    using namespace domain;

    Dict response_to_request;
    response_to_request.insert({"request_id"s, Node(id_request)});

    std::ostringstream strm;

    request_handler.RenderMap().Render(strm);
    Node node_map(strm.str());
    response_to_request.insert({"map"s, node_map});

    return response_to_request;
}

void JSONReader::PrepareJSON(const Array& array_in, Array& array_out) const {
    using namespace json;

    Dict info;
    string key_err;
    string type_request;
    string name_for_type;
    int id_reqest = 0;
    
    try{
        for(const auto& dict : array_in) {
            key_err = "stat_requests Dicts"s;

            for(const auto& [key, value] : dict.AsMap()) {
                key_err = key;

                if(key == "id"s) {
                    id_reqest = value.AsInt();
                }

                if(key == "type"s) {
                    type_request = value.AsString();
                }

                if(key == "name"s) {
                    name_for_type = value.AsString();
                } 
            }
       
            if(type_request == "Bus"s) {
                info = ApplyCommandToBusInfo(id_reqest, name_for_type);
                Node node_info(info);
                array_out.push_back(node_info);
            } 

            if(type_request == "Stop"s) {
                info = ApplyCommandToStopInfo(id_reqest, name_for_type);
                Node node_info(info);
                array_out.push_back(node_info);
            }

            if(type_request == "Map"s) {
                info = ApplyCommandToMapInfo(id_reqest);
                Node node_info(info);
                array_out.push_back(node_info);
            }
        } 
    } catch(const std::logic_error& err) {
        cerr << "Incorrect value of key\""s  << key_err << "\" "s << err.what() << '\n'; 
    } catch(const std::exception& err) {
        cerr << err.what() << '\n';
    } catch(...) {
        cerr << "Unknow exeption"s << '\n';
    }
}

void JSONReader::LoadTransportCatalogue() {
    const auto iter_command = doc_.GetRoot().AsMap().find("base_requests"s);

    if(iter_command == doc_.GetRoot().AsMap().end()) {
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
    const auto iter_command = doc_.GetRoot().AsMap().find("render_settings"s);

    if(iter_command == doc_.GetRoot().AsMap().end()) {
        return; 
    }

    const auto dict = iter_command->second.AsMap();

    if(dict.empty()) {
        return;
    }

    for(const auto& [key, value] : dict) {
        if(value.IsInt()) {
            renderer_.ApplySetting<int>(key, value.AsInt());
            continue;
        }

        if(value.IsPureDouble()) {
            renderer_.ApplySetting<double>(key, value.AsDouble());
            continue;
        }

        //Если массив, в котором любое из значений является IsDouble() - это RGB/RGBA
        if(value.IsArray() && value.AsArray()[0].IsDouble()) {
            ApplyArrayOfColorCharacteristics(key, value.AsArray());
            continue;
        }
        
        if(value.IsString()) {
            svg::Color color (value.AsString());
            renderer_.ApplySetting(key, color);
        }

        if(value.IsArray()) {
            for(const auto& elem : value.AsArray()) {

                if(elem.IsString()) {
                    svg::Color color (elem.AsString());
                    renderer_.ApplySetting(key, color);
                    continue;
                }

                if(elem.IsArray()){
                    ApplyArrayOfColorCharacteristics(key, elem.AsArray());
                    continue; 
                }
                
                throw std::logic_error("Is not a color"s);
            }  
        }
    }
}

void JSONReader::PrintJSON() const {
    using namespace json;

    const auto iter_command = doc_.GetRoot().AsMap().find("stat_requests"s);

    if(iter_command == doc_.GetRoot().AsMap().end()) {
        return; 
    }
    
    Array array_in = iter_command->second.AsArray();
    if(array_in.empty()) {
        return;
    }
    Array array_out; 
    PrepareJSON(array_in, array_out);

    Node root(array_out);
    Document doc(root);

    Print(doc, std::cout);
}
