#pragma once

#include <cstddef>
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

std::vector<SResult> compact(const std::vector<SItem>& items, EAxis primaryAxis);

}
