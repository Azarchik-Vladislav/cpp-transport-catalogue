#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std::literals;

using std::move;
using std::string;
using std::string_view;
using std::vector;

/**
 * Удаляет пробелы в начале и конце строки
 */
string_view Trim(string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
vector<string_view> Split(string_view string, string_view delim) {
    vector<string_view> result;

    size_t pos = 0;
    while (pos < string.length()) {
        auto delim_pos = string.find(delim, pos);

        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }

        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }

        pos = delim_pos + delim.size();
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
vector<string_view> ParseRoute(string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, ">");
    }

    auto stops = Split(route, "-");
    vector<string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
} 

CommandDescription ParseCommandDescription(string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {string(line.substr(0, space_pos)),
            string(line.substr(not_space, colon_pos - not_space)),
            string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for(const auto& command : commands_) {
        
        if(command.command == "Stop"sv) {
            vector<string_view> buffer_for_queries = Split(command.description, ",");

            if(buffer_for_queries.empty() || buffer_for_queries.size() < 2) {
                continue;
            }

            double latitude = std::stod(string(buffer_for_queries[0]));
            double longitude = std::stod(string(buffer_for_queries[1]));

            catalogue.AddStop(command.id, Coordinates{latitude, longitude});
        }
    }

    for(const auto& command : commands_) {
        if(command.command == "Stop"sv) {
            vector<string_view> buffer_for_queries = Split(command.description, ",");

            for(size_t i = 2; i < buffer_for_queries.size(); ++i) {
                auto distance_to_stop = Split(buffer_for_queries[i], "m to ");

                double distance = std::stod(string(distance_to_stop[0]));
                string stop_to = string(distance_to_stop[1]);

                catalogue.AddDistance(command.id, stop_to, distance);
            }
            
        }
    }

    for(const auto& command : commands_) {
        vector<string_view> name_stops_for_bus = ParseRoute(command.description);

        if(command.command == "Bus"sv) {
            catalogue.AddBus(command.id, move(name_stops_for_bus));
        }
    }
}

