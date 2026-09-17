#pragma once

#include <string>
#include <string_view>

namespace ScrollOverview::Search {

std::string normalize(std::string_view text);
bool        matches(std::string_view normalizedQuery, std::string_view title, std::string_view appClass);
bool        isPrintable(std::string_view text);
bool        eraseLastCodepoint(std::string& text);

}
