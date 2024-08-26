#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream &operator<<(std::ostream& out, StrokeLineCap stroke_line_cup) {

    switch (stroke_line_cup) {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;

        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;

        default:
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, StrokeLineJoin stroke_line_join) {

    switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;

        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;

        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;

        default:
            break;
    }

    return out;
}

void PrintValue(std::monostate, std::ostream& out) {
    out << "none";
}

void PrintValue(const std::string& color, std::ostream& out) {
    out << color;
}

void PrintValue(const svg::Rgb& color, std::ostream& out) {
    out << "rgb(";
    out << color.red << ",";
    out << color.green << ",";
    out << color.blue << ")";
}

void PrintValue(const svg::Rgba& color, std::ostream& out) {
    out << "rgba(";
    out << color.red << ",";
    out << color.green << ",";
    out << color.blue << ",";
    out << color.opacity << ")";
}

void PrintColor(const Color& color, std::ostream& out) {
    std::visit(
        [&out](const auto& _color) {
            PrintValue(_color, out);
        },
    color.GetValue());
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle"sv;
    out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);

    return *this;
}

void Polyline::AddCoordinates(std::ostream& out) const {
    bool it_first = true;
    for(const auto& point : points_) {
        if(it_first) {
            out << point.x << ","sv << point.y;
            it_first = false;
            continue;
        }

        out << " "sv << point.x << ","sv << point.y;
    }
    out << "\""sv;
}

void Polyline::RenderObject(const RenderContext &context) const {
    auto& out = context.out;
    
    out << "<polyline"sv;
    out << " points=\""sv;
    AddCoordinates(out);
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;

    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;

    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;

    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;

    return *this;
}

Text &Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;

    return *this;
}

Text &Text::SetData(std::string data) {
    data_ = data;

    return *this;
}

void Text::EscapingCharactersInData(std::ostream& out) const {
    for (auto& c : data_) {
        switch (c) {
            case '"':
                out << "&quot;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            default:
                out.put(c);
        }
    }

}

void Text::RenderObject(const RenderContext &context) const {
    auto& out = context.out;

    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""s << position_.x << "\" "s << "y=\""s << position_.y << "\" "s;
    out << "dx=\""s << offset_.x << "\" "s << "dy=\""s << offset_.y << "\" "s;
    out << "font-size=\""s << size_ << "\""s;

    if(!font_family_.empty()) {
        out << " font-family=\""s << font_family_ << "\""s;
    }

    if(!font_weight_.empty()) {
        out << " font-weight=\""s << font_weight_ << "\""s;
    }

    out << ">"s;
    EscapingCharactersInData(out);
    out << "</text>"s;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream &out) const {
    RenderContext context(out, 2, 2);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for(const auto& obj : objects_) {
        obj->Render(context);
    }

    out << "</svg>"sv;
}
} // namespace svg