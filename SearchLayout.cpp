#include "SearchLayout.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ScrollOverview::SearchLayout {
namespace {

constexpr double BAND_TOLERANCE = 1.0;

double primaryStart(const SBox& box, EAxis axis) {
    return axis == EAxis::HORIZONTAL ? box.x : box.y;
}

double primarySize(const SBox& box, EAxis axis) {
    return axis == EAxis::HORIZONTAL ? box.width : box.height;
}

double secondaryStart(const SBox& box, EAxis axis) {
    return axis == EAxis::HORIZONTAL ? box.y : box.x;
}

double secondarySize(const SBox& box, EAxis axis) {
    return axis == EAxis::HORIZONTAL ? box.height : box.width;
}

void setPrimaryStart(SBox& box, EAxis axis, double value) {
    if (axis == EAxis::HORIZONTAL)
        box.x = value;
    else
        box.y = value;
}

void setSecondaryStart(SBox& box, EAxis axis, double value) {
    if (axis == EAxis::HORIZONTAL)
        box.y = value;
    else
        box.x = value;
}

double axisStart(const SBox& box, EAxis axis) {
    return axis == EAxis::HORIZONTAL ? box.x : box.y;
}

double axisSize(const SBox& box, EAxis axis) {
    return axis == EAxis::HORIZONTAL ? box.width : box.height;
}

void translateAxis(SBox& box, EAxis axis, double value) {
    if (axis == EAxis::HORIZONTAL)
        box.x += value;
    else
        box.y += value;
}

struct SBand {
    std::vector<const SItem*> items;
    double                    start = 0.0;
    double                    end   = 0.0;
};

double minimumGap(const std::vector<SBand>& bands) {
    double gap = std::numeric_limits<double>::max();
    for (size_t i = 1; i < bands.size(); ++i)
        gap = std::min(gap, std::max(0.0, bands[i].start - bands[i - 1].end));
    return gap == std::numeric_limits<double>::max() ? 0.0 : gap;
}

double minimumSecondaryGap(std::vector<const SItem*> items, EAxis axis) {
    if (items.size() < 2)
        return 0.0;

    std::ranges::sort(items, [axis](const auto* lhs, const auto* rhs) { return secondaryStart(lhs->box, axis) < secondaryStart(rhs->box, axis); });
    double gap = std::numeric_limits<double>::max();
    for (size_t i = 1; i < items.size(); ++i) {
        const auto previousEnd = secondaryStart(items[i - 1]->box, axis) + secondarySize(items[i - 1]->box, axis);
        gap                    = std::min(gap, std::max(0.0, secondaryStart(items[i]->box, axis) - previousEnd));
    }
    return gap == std::numeric_limits<double>::max() ? 0.0 : gap;
}

SBox occupiedBounds(const std::vector<SResult>& results) {
    if (results.empty())
        return {};

    double left   = results.front().box.x;
    double top    = results.front().box.y;
    double right  = left + results.front().box.width;
    double bottom = top + results.front().box.height;
    for (const auto& result : results) {
        left   = std::min(left, result.box.x);
        top    = std::min(top, result.box.y);
        right  = std::max(right, result.box.x + result.box.width);
        bottom = std::max(bottom, result.box.y + result.box.height);
    }

    return {.x = left, .y = top, .width = right - left, .height = bottom - top};
}

double alignmentOffset(const SBox& occupied, const SBox& target, const SBox* selected, EAxis axis) {
    const double occupiedStart = axisStart(occupied, axis);
    const double occupiedSize  = axisSize(occupied, axis);
    const double targetStart   = axisStart(target, axis);
    const double targetSize    = axisSize(target, axis);

    if (targetSize <= 0.0)
        return -occupiedStart;

    if (occupiedSize <= targetSize)
        return targetStart + (targetSize - occupiedSize) / 2.0 - occupiedStart;

    if (!selected)
        return targetStart - occupiedStart;

    const double desired = targetStart + targetSize / 2.0 - (axisStart(*selected, axis) + axisSize(*selected, axis) / 2.0);
    const double minimum = targetStart + targetSize - (occupiedStart + occupiedSize);
    const double maximum = targetStart - occupiedStart;
    return std::clamp(desired, minimum, maximum);
}

}

std::vector<SResult> compact(const std::vector<SItem>& items, EAxis primaryAxis, const SBox& targetBox, std::optional<size_t> selectedId) {
    std::vector<const SItem*> ordered;
    ordered.reserve(items.size());
    for (const auto& item : items) {
        if (item.box.width > 0.0 && item.box.height > 0.0)
            ordered.emplace_back(&item);
    }

    if (!ordered.empty() && std::ranges::all_of(ordered, [](const auto* item) { return item->matches; })) {
        std::vector<SResult> unchanged;
        unchanged.reserve(ordered.size());
        for (const auto* item : ordered)
            unchanged.push_back({.id = item->id, .box = item->box});
        return unchanged;
    }

    std::ranges::sort(ordered, [primaryAxis](const auto* lhs, const auto* rhs) {
        const auto primaryDifference = primaryStart(lhs->box, primaryAxis) - primaryStart(rhs->box, primaryAxis);
        if (std::abs(primaryDifference) > BAND_TOLERANCE)
            return primaryDifference < 0.0;
        return secondaryStart(lhs->box, primaryAxis) < secondaryStart(rhs->box, primaryAxis);
    });

    std::vector<SBand> bands;
    for (const auto* item : ordered) {
        const auto start = primaryStart(item->box, primaryAxis);
        const auto end   = start + primarySize(item->box, primaryAxis);
        if (bands.empty() || std::abs(start - bands.back().start) > BAND_TOLERANCE) {
            bands.push_back({.items = {item}, .start = start, .end = end});
            continue;
        }

        bands.back().items.emplace_back(item);
        bands.back().end = std::max(bands.back().end, end);
    }

    if (bands.empty())
        return {};

    const auto PRIMARYGAP = minimumGap(bands);
    double     nextPrimary = 0.0;
    std::vector<SResult> results;

    for (const auto& band : bands) {
        std::vector<const SItem*> matching;
        for (const auto* item : band.items) {
            if (item->matches)
                matching.emplace_back(item);
        }
        if (matching.empty())
            continue;

        std::ranges::sort(matching, [primaryAxis](const auto* lhs, const auto* rhs) {
            return secondaryStart(lhs->box, primaryAxis) < secondaryStart(rhs->box, primaryAxis);
        });

        const auto SECONDARYGAP = minimumSecondaryGap(band.items, primaryAxis);
        double nextSecondary = 0.0;

        double bandWidth = 0.0;
        for (const auto* item : matching) {
            auto box = item->box;
            setPrimaryStart(box, primaryAxis, nextPrimary);
            setSecondaryStart(box, primaryAxis, nextSecondary);
            nextSecondary += secondarySize(box, primaryAxis) + SECONDARYGAP;
            bandWidth = std::max(bandWidth, primarySize(box, primaryAxis));
            results.push_back({.id = item->id, .box = box});
        }

        nextPrimary += bandWidth + PRIMARYGAP;
    }

    if (results.empty())
        return results;

    const auto BOUNDS      = occupiedBounds(results);
    const auto SELECTED    = selectedId ? std::ranges::find_if(results, [selectedId](const auto& result) { return result.id == *selectedId; }) : results.end();
    const auto SELECTEDBOX = SELECTED == results.end() ? nullptr : &SELECTED->box;
    const auto XOFFSET     = alignmentOffset(BOUNDS, targetBox, SELECTEDBOX, EAxis::HORIZONTAL);
    const auto YOFFSET     = alignmentOffset(BOUNDS, targetBox, SELECTEDBOX, EAxis::VERTICAL);
    for (auto& result : results) {
        translateAxis(result.box, EAxis::HORIZONTAL, XOFFSET);
        translateAxis(result.box, EAxis::VERTICAL, YOFFSET);
    }

    return results;
}

}
