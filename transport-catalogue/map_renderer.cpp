#include "map_renderer.h"

using namespace std::literals;
using namespace domain;
using namespace svg;

using std::set;
using std::string;
using std::vector;

namespace renderer{
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void MapRenderer::ApplySetting(const std::string& setting, svg::Point& value) {

    if(setting == "bus_label_offset"s) {
        renderer_settings_.bus_label_offset = value;
    }
    
    if(setting == "stop_label_offset"s) {
        renderer_settings_.stop_label_offset = value;
    }  
}

void MapRenderer::ApplySetting(const string& setting, Color& value) {

    if(setting == "color_palette"s) {
        renderer_settings_.color_palette.push_back(value);
    }

    if(setting == "underlayer_color"s) {
        renderer_settings_.underlayer_color = value;
    }
}

vector<Polyline> MapRenderer::CreatePolylines(const vector<const Bus*>& buses,
                                              const SphereProjector& sphere_projector) const {
    vector<Polyline> polylines;
    size_t index_in_palette = 0;

    for(const auto& bus : buses) {
        Polyline polyline;

        if(index_in_palette == renderer_settings_.color_palette.size()) {
            index_in_palette = 0;
        }

        for(size_t i = 0; i < bus->stops_for_bus.size(); ++i) {

            if(bus->is_roundtrip && i == bus->stops_for_bus.size()){
                break;
            }

            polyline.AddPoint(sphere_projector(bus->stops_for_bus[i]->coordinates));
        }

        polyline.SetFillColor(svg::Color())
                .SetStrokeColor(renderer_settings_.color_palette[index_in_palette])
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeWidth(renderer_settings_.line_width);

        polylines.push_back(polyline);

        ++index_in_palette;
    } 

    return polylines;
}

vector<Text> MapRenderer::CreateTextsBusLabel(const vector<const Bus*>& buses, const SphereProjector& sphere_projector) const {
    vector<Text> result;

    Text underlayer_name_bus;
    Text name_bus;

    size_t index_in_palette = 0;

    for(const auto& bus : buses) {

        if(index_in_palette == renderer_settings_.color_palette.size()) {
            index_in_palette = 0;
        }

        const Stop* start = bus->stops_for_bus[0];
        const Stop* finish = bus->stops_for_bus[bus->stops_for_bus.size() / 2];

        underlayer_name_bus.SetData(bus->name_bus);                         
        name_bus.SetData(bus->name_bus);

        underlayer_name_bus.SetPosition(sphere_projector(start->coordinates));
        name_bus.SetPosition(sphere_projector(start->coordinates));

        underlayer_name_bus.SetOffset(renderer_settings_.bus_label_offset);
        name_bus.SetOffset(renderer_settings_.bus_label_offset);

        underlayer_name_bus.SetFontSize(static_cast<uint32_t>(renderer_settings_.bus_label_font_size));
        name_bus.SetFontSize(static_cast<uint32_t>(renderer_settings_.bus_label_font_size));

        underlayer_name_bus.SetFontFamily("Verdana"s);
        name_bus.SetFontFamily("Verdana"s);

        underlayer_name_bus.SetFontWeight("bold"s);
        name_bus.SetFontWeight("bold"s);

        underlayer_name_bus.SetFillColor(renderer_settings_.underlayer_color);
        name_bus.SetFillColor(renderer_settings_.color_palette[index_in_palette]);

        underlayer_name_bus.SetStrokeColor(renderer_settings_.underlayer_color);
        underlayer_name_bus.SetStrokeWidth(renderer_settings_.underlayer_width);
        underlayer_name_bus.SetStrokeLineCap(StrokeLineCap::ROUND);
        underlayer_name_bus.SetStrokeLineJoin(StrokeLineJoin::ROUND);

        result.push_back(underlayer_name_bus);
        result.push_back(name_bus);

        ++index_in_palette;
        if(bus->is_roundtrip || (!bus->is_roundtrip 
                                 && start == finish)) {
            
            continue;
        }

        underlayer_name_bus.SetPosition(sphere_projector(finish->coordinates));
        name_bus.SetPosition(sphere_projector(finish->coordinates));
  
        result.push_back(underlayer_name_bus);
        result.push_back(name_bus);
    }

    return result;
}

vector<Circle> MapRenderer::CreatePointsOfStops(const vector<const Stop*>& stops, const SphereProjector& sphere_projector) const {
    vector<Circle> points;

    Circle point;

    for(const auto& stop : stops) {
        point.SetCenter(sphere_projector(stop->coordinates));
        point.SetRadius(renderer_settings_.stop_radius);
        point.SetFillColor(Color("white"s));

        points.push_back(point);
    }
    
    return points;
}

vector<Text> MapRenderer::CreateStopLabel(const vector<const Stop*>& stops, const SphereProjector& sphere_projector) const {
    vector<Text> lables;

    Text underlayer_name_stop;
    Text name_stop;

    underlayer_name_stop.SetOffset(renderer_settings_.stop_label_offset);
    name_stop.SetOffset(renderer_settings_.stop_label_offset);

    underlayer_name_stop.SetFontSize(static_cast<uint32_t>(renderer_settings_.stop_label_font_size));
    name_stop.SetFontSize(static_cast<uint32_t>(renderer_settings_.stop_label_font_size));

    underlayer_name_stop.SetFontFamily("Verdana"s);
    name_stop.SetFontFamily("Verdana"s);

    underlayer_name_stop.SetFillColor(renderer_settings_.underlayer_color);
    name_stop.SetFillColor(Color("black"s));

    underlayer_name_stop.SetStrokeColor(renderer_settings_.underlayer_color);
    underlayer_name_stop.SetStrokeWidth(renderer_settings_.underlayer_width);
    underlayer_name_stop.SetStrokeLineCap(StrokeLineCap::ROUND);
    underlayer_name_stop.SetStrokeLineJoin(StrokeLineJoin::ROUND);

    for(const auto& stop : stops) {
        underlayer_name_stop.SetData(stop->name_stop);
        name_stop.SetData(stop->name_stop);

        underlayer_name_stop.SetPosition(sphere_projector(stop->coordinates));
        name_stop.SetPosition(sphere_projector(stop->coordinates));

        lables.push_back(underlayer_name_stop);
        lables.push_back(name_stop);
    }

    return lables;
}

Document MapRenderer::CreateDocSVG(vector<const Bus*>&& buses,
                                   vector<const Stop*>&& stops) const {
    Document result;
    vector<geo::Coordinates> all_coordinates;

    std::sort(buses.begin(), buses.end(), [](const Bus* lhs,const Bus* rhs) {
                                          return lhs->name_bus < rhs->name_bus;}
              );

    std::sort(stops.begin(), stops.end(), [](const Stop* lhs,const Stop* rhs) {
                                          return lhs->name_stop < rhs->name_stop;}
              );

    for(const auto& bus : buses) {
        for(const auto& stop : bus->stops_for_bus) {
            all_coordinates.push_back(stop->coordinates);
        }
    }

    SphereProjector sphere_projector(all_coordinates.begin(), all_coordinates.end(),
                                     renderer_settings_.width,
                                     renderer_settings_.height,
                                     renderer_settings_.padding);

    for(const auto& polyline : CreatePolylines(buses, sphere_projector)) {
        result.Add(polyline);
    }

    for(const auto& bus_name : CreateTextsBusLabel(buses, sphere_projector)) {
        result.Add(bus_name);
    }

    for(const auto& point : CreatePointsOfStops(stops, sphere_projector)) {
        result.Add(point);
    }

    for(const auto& stop_name : CreateStopLabel(stops, sphere_projector)) {
        result.Add(stop_name);
    }
    
    return result;
}


bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
} // namespace renderer