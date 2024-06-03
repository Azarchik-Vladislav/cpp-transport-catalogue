#pragma once

#include <iostream>
#include <iomanip>
#include <optional>
#include <set>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

std::ostream& operator<<(std::ostream& out, const BusInfo& bus_info);

std::pair<std::string_view, std::string_view> ParseQuery(std::string_view request);

void PrintBusInfo(const std::optional<BusInfo>& bus_info, std::string_view name_for_query, 
                            std::ostream& output);
void PrintStopInfo(const std::optional<std::set<std::string_view>>& stop_info, std::string_view name_for_query,
                    std::ostream& output);

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);