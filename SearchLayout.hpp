#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace ScrollOverview::SearchLayout {

enum class EAxis {
    HORIZONTAL,
    VERTICAL,
};

struct SBox {
    double x      = 0.0;
    double y      = 0.0;
    double width  = 0.0;
    double height = 0.0;
};

struct SItem {
    size_t id      = 0;
    SBox   box;
    bool   matches = false;
};

struct SResult {
    size_t id = 0;
    SBox   box;
};

// Boxes and targetBox use the same coordinate space. Filtering compacts matching
// boxes at an origin-independent position, then aligns their occupied bounds to
// targetBox. An all-matching input is returned unchanged.
std::vector<SResult> compact(const std::vector<SItem>& items, EAxis primaryAxis, const SBox& targetBox,
                             std::optional<size_t> selectedId = std::nullopt);

std::vector<size_t> matchingRowIndices(const std::vector<bool>& rowMatches);
std::optional<size_t> rowRank(const std::vector<size_t>& visibleRows, size_t workspaceIndex);

}
