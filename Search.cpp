#include "Search.hpp"

#include <glib.h>

namespace ScrollOverview::Search {

std::string normalize(std::string_view text) {
    if (text.empty())
        return {};

    const std::string owned{text};
    if (!g_utf8_validate(owned.c_str(), static_cast<gssize>(owned.size()), nullptr))
        return {};

    gchar* normalized = g_utf8_normalize(owned.c_str(), static_cast<gssize>(owned.size()), G_NORMALIZE_ALL_COMPOSE);
    if (!normalized)
        return {};

    gchar* folded = g_utf8_casefold(normalized, -1);
    g_free(normalized);
    if (!folded)
        return {};

    std::string result{folded};
    g_free(folded);
    return result;
}

bool matches(std::string_view normalizedQuery, std::string_view title, std::string_view appClass) {
    if (normalizedQuery.empty())
        return true;

    const auto normalizedTitle = normalize(title);
    if (normalizedTitle.find(normalizedQuery) != std::string::npos)
        return true;

    return normalize(appClass).find(normalizedQuery) != std::string::npos;
}

bool isPrintable(std::string_view text) {
    if (text.empty())
        return false;

    const std::string owned{text};
    if (!g_utf8_validate(owned.c_str(), static_cast<gssize>(owned.size()), nullptr))
        return false;

    const char* cursor = owned.c_str();
    const char* end    = cursor + owned.size();
    while (cursor < end) {
        const auto codepoint = g_utf8_get_char(cursor);
        if (!g_unichar_isprint(codepoint))
            return false;
        cursor = g_utf8_next_char(cursor);
    }

    return true;
}

bool eraseLastCodepoint(std::string& text) {
    if (text.empty())
        return false;

    const char* begin = text.c_str();
    const char* last  = g_utf8_find_prev_char(begin, begin + text.size());
    if (!last) {
        text.clear();
        return true;
    }

    text.resize(static_cast<size_t>(last - begin));
    return true;
}

}
