#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <type_traits>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "svg.h"

namespace renderer {
inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RednererSettings {
    std::vector<svg::Color> color_palette;

    svg::Color underlayer_color;

    svg::Point bus_label_offset;
    svg::Point stop_label_offset;

    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    double underlayer_width = 0;

    int bus_label_font_size = 0;
    int stop_label_font_size = 0;   
};

class MapRenderer {
public:
    void ApplySetting(const std::string& setting, svg::Point& value);
    void ApplySetting(const std::string& setting, svg::Color& value);

    template <typename Value>
    void ApplySetting(const std::string& setting,const Value& value) {
        using namespace std::literals;
        
        if(setting == "width"s) { 
            renderer_settings_.width = value;
        }

        if(setting == "height"s) {
            renderer_settings_.height = value;
        }

        if(setting == "padding"s) {
            renderer_settings_.padding = value;
        }

        if(setting == "stop_radius"s) {
            renderer_settings_.stop_radius = value;
        }

        if(setting == "underlayer_width"s) {
            renderer_settings_.underlayer_width = value;
        }
        
        if(setting == "stop_label_font_size"s) {
            renderer_settings_.stop_label_font_size = static_cast<int>(value);
        }

        if(setting == "bus_label_font_size"s) {
            renderer_settings_.bus_label_font_size = static_cast<int>(value);
        }
        
        if(setting == "line_width"s) {
            renderer_settings_.line_width = value;
        }
    }

    svg::Document CreateDocSVG(std::vector<const domain::Bus*>&& buses,
                               std::vector<const domain::Stop*>&& stops) const;

private:
    RednererSettings renderer_settings_;

    void AddPolylines(const std::vector<const domain::Bus*>& buses,
                                            const SphereProjector& sphere_projector,
                                            svg::Document& xml_render) const;

    void AddTextsBusLabel(const std::vector<const domain::Bus*>& buses,
                                            const SphereProjector& sphere_projector,
                                            svg::Document& xml_render) const;

    void AddPointsOfStops(const std::vector<const domain::Stop*>& stops,
                                            const SphereProjector& sphere_projector,
                                            svg::Document& xml_render) const;

    void AddStopLabel(const std::vector<const domain::Stop*>& stops,
                                        const SphereProjector& sphere_projector,
                                        svg::Document& xml_render) const;
    
};
}//namespace renderer