#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace svg {

struct Point {
    Point() = default;
    Point(double _x, double _y)
        : x(_x)
        , y(_y) {
    }

    double x = 0;
    double y = 0;
};

struct Rgb {
    int red = 0;
    int green = 0;
    int blue = 0;
};

struct Rgba {
    int red = 0;
    int green = 0;
    int blue = 0;
    double opacity = 0.;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& _out);
    RenderContext(std::ostream& _out, int _indent_step, int _indent = 0);

    RenderContext Indented() const;
    void RenderIndent() const;

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

using std::optional;

class Color final : private std::variant<std::monostate, std::string, Rgb, Rgba> {
public:
    Color() = default;
    Color(std::string color);
    Color(Rgb color);
    Color(Rgba color);

    bool IsString() const;
    const std::string& AsString() const;

    bool IsRgb() const;
    const Rgb& AsRgb() const;

    bool IsRgba() const;
    const Rgba& AsRgba() const;

    bool IsMonostate() const;

    const variant& GetValue() const {
        return *this;
    }
};

inline const Color NoneColor;

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cup);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

void PrintValue(const std::string &color, std::ostream &out);

void PrintValue(std::monostate color, std::ostream& out);

void PrintValue(const svg::Rgb& color, std::ostream& out);

void PrintValue(const svg::Rgba& color, std::ostream& out);   

void PrintColor(const Color& color, std::ostream& out);

template<typename Owner>
class PathProps {
public:
    Owner& SetFillColor(const Color& color) {
        fill_color_ = color;

        return AsOwner();
    }

    Owner& SetStrokeColor(const Color& color) {
        stroke_color_ = color;

        return AsOwner();
    }

    Owner& SetStrokeWidth(const double& width) {
        stroke_width_ = width;

        return AsOwner();
    }

    Owner& SetStrokeLineCap(const StrokeLineCap& line_cap) {
        stroke_line_cap_ = line_cap;

        return AsOwner();
    }

    Owner& SetStrokeLineJoin(const StrokeLineJoin& line_join) {
        stroke_line_join_ = line_join;

        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        out << " fill=\""sv;
        PrintColor(fill_color_, out);
        out << "\""sv;

        if(!stroke_color_.IsMonostate()) {
            out << " stroke=\""sv;
            PrintColor(stroke_color_, out);
            out << "\""sv;
        }

        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }

        if(stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv; 
        }

        if(stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }


private:
    Color fill_color_;
    Color stroke_color_;
    optional<double> stroke_width_;
    optional<StrokeLineCap> stroke_line_cap_;
    optional<StrokeLineJoin> stroke_line_join_;

    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;
private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
private:
    Point center_;
    double radius_ = 1.0;

    void RenderObject(const RenderContext& context) const override;
};

class Polyline final : public Object, public PathProps<Polyline> { 
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);
private:
    std::vector<Point> points_;

    void AddCoordinates(std::ostream& out) const;
    void RenderObject(const RenderContext& context) const override; 
};

class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    Point position_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_family_ = "";
    std::string font_weight_ = "";
    std::string data_ = "";

    void EscapingCharactersInData(std::ostream& out) const;
    void RenderObject(const RenderContext& context) const override;
};

class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
protected:
    ~ObjectContainer() = default;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;  
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
    virtual ~Drawable() = default; 
    virtual void Draw(ObjectContainer& container) const  = 0;
};

} // namespace svg