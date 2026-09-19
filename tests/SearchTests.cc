#include "Search.hpp"
#include "SearchLayout.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#define CHECK(condition)                                                                                                                                        \
    do {                                                                                                                                                        \
        if (!(condition)) {                                                                                                                                     \
            std::cerr << "check failed at line " << __LINE__ << ": " #condition << '\n';                                                                      \
            return 1;                                                                                                                                           \
        }                                                                                                                                                       \
    } while (false)

int main() {
    using namespace ScrollOverview::Search;

    CHECK(normalize("TeRmInAl") == "terminal");
    CHECK(normalize("A\u030A") == normalize("Å"));
    CHECK(matches(normalize("fire"), "Mozilla Firefox", "firefox"));
    CHECK(matches(normalize("ång"), "Ångström Notes", "notes"));
    CHECK(matches(normalize("STRASSE"), "Straße", "notes"));
    CHECK(matches(normalize("term"), "Shell", "org.example.Terminal"));
    CHECK(matches(normalize("alpha"), "Project Alpha", "code"));
    CHECK(!matches(normalize("alpha"), "Project Beta", "code"));
    CHECK(!matches(normalize("editor"), "Terminal", "foot"));

    CHECK(isPrintable("a"));
    CHECK(isPrintable("£"));
    CHECK(isPrintable("界"));
    CHECK(isPrintable("two words"));
    CHECK(!isPrintable("\n"));
    CHECK(!isPrintable(std::string{"\xFF", 1}));

    std::string query = "ab界";
    CHECK(eraseLastCodepoint(query));
    CHECK(query == "ab");
    CHECK(eraseLastCodepoint(query));
    CHECK(query == "a");
    CHECK(eraseLastCodepoint(query));
    CHECK(query.empty());
    CHECK(!eraseLastCodepoint(query));

    using ScrollOverview::SearchLayout::EAxis;
    using ScrollOverview::SearchLayout::SBox;
    using ScrollOverview::SearchLayout::SItem;
    const auto boxFor = [](const auto& results, size_t id) -> SBox {
        const auto it = std::ranges::find_if(results, [id](const auto& result) { return result.id == id; });
        return it == results.end() ? SBox{} : it->box;
    };
    const auto near = [](double lhs, double rhs) { return std::abs(lhs - rhs) < 0.001; };
    const auto sameBox = [&near](const SBox& lhs, const SBox& rhs) {
        return near(lhs.x, rhs.x) && near(lhs.y, rhs.y) && near(lhs.width, rhs.width) && near(lhs.height, rhs.height);
    };
    const SBox viewport = {0, 0, 1920, 1080};

    CHECK(ScrollOverview::SearchLayout::matchingRowIndices({true, false, true}) == std::vector<size_t>({0, 2}));
    CHECK(ScrollOverview::SearchLayout::matchingRowIndices({false, true, false, false, true}) == std::vector<size_t>({1, 4}));
    CHECK(ScrollOverview::SearchLayout::matchingRowIndices({false, false, false}).empty());
    CHECK(ScrollOverview::SearchLayout::matchingRowIndices({true}) == std::vector<size_t>({0}));
    const auto compactRows = ScrollOverview::SearchLayout::matchingRowIndices({true, false, true});
    CHECK(ScrollOverview::SearchLayout::rowRank(compactRows, 0) == 0);
    CHECK(ScrollOverview::SearchLayout::rowRank(compactRows, 2) == 1);
    CHECK(!ScrollOverview::SearchLayout::rowRank(compactRows, 1));

    const std::vector<SItem> oneFilteredMatch = {
        {.id = 1, .box = {0, 0, 900, 600}, .matches = true},
        {.id = 2, .box = {910, 0, 900, 600}, .matches = false},
    };
    const auto centredSingle = ScrollOverview::SearchLayout::compact(oneFilteredMatch, EAxis::HORIZONTAL, viewport);
    CHECK(centredSingle.size() == 1);
    CHECK(sameBox(boxFor(centredSingle, 1), {510, 240, 900, 600}));

    const std::vector<std::vector<SItem>> singleMatchLayouts = {
        {{{.id = 3, .box = {510, 240, 900, 600}, .matches = true}}},
        {{{.id = 1, .box = {0, 0, 900, 600}, .matches = false}, {.id = 3, .box = {910, 0, 900, 600}, .matches = true}}},
        {{{.id = 1, .box = {0, 0, 900, 600}, .matches = false},
          {.id = 2, .box = {910, 0, 900, 600}, .matches = false},
          {.id = 3, .box = {1820, 0, 900, 600}, .matches = true}}},
        {{{.id = 3, .box = {0, 0, 900, 600}, .matches = true},
          {.id = 4, .box = {910, 0, 900, 600}, .matches = false},
          {.id = 5, .box = {1820, 0, 900, 600}, .matches = false}}},
        {{{.id = 1, .box = {0, 0, 900, 600}, .matches = false},
          {.id = 3, .box = {910, 0, 900, 600}, .matches = true},
          {.id = 4, .box = {1820, 0, 900, 600}, .matches = false}}},
    };
    for (const auto& layoutItems : singleMatchLayouts) {
        const auto compacted = ScrollOverview::SearchLayout::compact(layoutItems, EAxis::HORIZONTAL, viewport);
        CHECK(compacted.size() == 1);
        CHECK(sameBox(boxFor(compacted, 3), {510, 240, 900, 600}));
    }

    const std::vector<SItem> twoAdjacentMatches = {
        {.id = 1, .box = {0, 0, 700, 600}, .matches = true},
        {.id = 2, .box = {710, 0, 700, 600}, .matches = true},
        {.id = 3, .box = {1420, 0, 700, 600}, .matches = false},
    };
    auto compacted = ScrollOverview::SearchLayout::compact(twoAdjacentMatches, EAxis::HORIZONTAL, viewport);
    CHECK(sameBox(boxFor(compacted, 1), {255, 240, 700, 600}));
    CHECK(sameBox(boxFor(compacted, 2), {965, 240, 700, 600}));

    const auto firstSelected = ScrollOverview::SearchLayout::compact(twoAdjacentMatches, EAxis::HORIZONTAL, viewport, 1);
    const auto secondSelected = ScrollOverview::SearchLayout::compact(twoAdjacentMatches, EAxis::HORIZONTAL, viewport, 2);
    CHECK(sameBox(boxFor(firstSelected, 1), {510, 240, 700, 600}));
    CHECK(sameBox(boxFor(firstSelected, 2), {1220, 240, 700, 600}));
    CHECK(sameBox(boxFor(secondSelected, 1), {0, 240, 700, 600}));
    CHECK(sameBox(boxFor(secondSelected, 2), {710, 240, 700, 600}));
    CHECK(sameBox(ScrollOverview::SearchLayout::interpolate(boxFor(firstSelected, 1), boxFor(secondSelected, 1), 0.5), {255, 240, 700, 600}));

    const std::vector<SItem> hiddenBetween = {
        {.id = 1, .box = {0, 0, 700, 600}, .matches = true},
        {.id = 2, .box = {710, 0, 700, 600}, .matches = false},
        {.id = 3, .box = {1420, 0, 700, 600}, .matches = true},
    };
    compacted = ScrollOverview::SearchLayout::compact(hiddenBetween, EAxis::HORIZONTAL, viewport);
    CHECK(compacted.size() == 2);
    CHECK(sameBox(boxFor(compacted, 1), {255, 240, 700, 600}));
    CHECK(sameBox(boxFor(compacted, 3), {965, 240, 700, 600}));

    const std::vector<SItem> verticalPrimary = {
        {.id = 1, .box = {0, 0, 600, 400}, .matches = true},
        {.id = 2, .box = {0, 410, 600, 400}, .matches = false},
        {.id = 3, .box = {0, 820, 600, 400}, .matches = true},
    };
    compacted = ScrollOverview::SearchLayout::compact(verticalPrimary, EAxis::VERTICAL, viewport);
    CHECK(sameBox(boxFor(compacted, 1), {660, 135, 600, 400}));
    CHECK(sameBox(boxFor(compacted, 3), {660, 545, 600, 400}));

    const std::vector<SItem> secondaryStack = {
        {.id = 1, .box = {0, 0, 900, 300}, .matches = true},
        {.id = 2, .box = {0, 310, 900, 300}, .matches = false},
        {.id = 3, .box = {0, 620, 900, 300}, .matches = true},
    };
    compacted = ScrollOverview::SearchLayout::compact(secondaryStack, EAxis::HORIZONTAL, viewport);
    CHECK(sameBox(boxFor(compacted, 1), {510, 235, 900, 300}));
    CHECK(sameBox(boxFor(compacted, 3), {510, 545, 900, 300}));

    const std::vector<SItem> verticalSecondaryStack = {
        {.id = 1, .box = {0, 0, 300, 900}, .matches = true},
        {.id = 2, .box = {310, 0, 300, 900}, .matches = false},
        {.id = 3, .box = {620, 0, 300, 900}, .matches = true},
    };
    compacted = ScrollOverview::SearchLayout::compact(verticalSecondaryStack, EAxis::VERTICAL, viewport);
    CHECK(sameBox(boxFor(compacted, 1), {655, 90, 300, 900}));
    CHECK(sameBox(boxFor(compacted, 3), {965, 90, 300, 900}));

    for (const auto& matches : {std::pair{true, false}, std::pair{false, true}}) {
        auto soleStackMatch       = secondaryStack;
        soleStackMatch[0].matches = matches.first;
        soleStackMatch[2].matches = matches.second;
        compacted = ScrollOverview::SearchLayout::compact(soleStackMatch, EAxis::HORIZONTAL, viewport);
        CHECK(compacted.size() == 1);
        CHECK(sameBox(compacted.front().box, {510, 390, 900, 300}));
    }

    const SBox topBarViewport = {0, 30, 1920, 1050};
    compacted = ScrollOverview::SearchLayout::compact(oneFilteredMatch, EAxis::HORIZONTAL, topBarViewport);
    CHECK(sameBox(boxFor(compacted, 1), {510, 255, 900, 600}));

    const SBox positiveOrigin = {1920, 30, 1920, 1050};
    compacted = ScrollOverview::SearchLayout::compact(oneFilteredMatch, EAxis::HORIZONTAL, positiveOrigin);
    CHECK(sameBox(boxFor(compacted, 1), {2430, 255, 900, 600}));

    const SBox negativeOrigin = {-1920, 30, 1920, 1050};
    compacted = ScrollOverview::SearchLayout::compact(oneFilteredMatch, EAxis::HORIZONTAL, negativeOrigin);
    CHECK(sameBox(boxFor(compacted, 1), {-1410, 255, 900, 600}));

    auto noMatches = hiddenBetween;
    for (auto& item : noMatches)
        item.matches = false;
    CHECK(ScrollOverview::SearchLayout::compact(noMatches, EAxis::HORIZONTAL, viewport).empty());

    auto allMatching = hiddenBetween;
    for (auto& item : allMatching)
        item.matches = true;
    const auto restored = ScrollOverview::SearchLayout::compact(allMatching, EAxis::HORIZONTAL, positiveOrigin);
    for (const auto& item : allMatching)
        CHECK(sameBox(boxFor(restored, item.id), item.box));

    const std::vector<SItem> oversized = {
        {.id = 1, .box = {0, 0, 900, 600}, .matches = true},
        {.id = 2, .box = {910, 0, 900, 600}, .matches = true},
        {.id = 3, .box = {1820, 0, 900, 600}, .matches = true},
        {.id = 4, .box = {2730, 0, 900, 600}, .matches = false},
    };
    compacted = ScrollOverview::SearchLayout::compact(oversized, EAxis::HORIZONTAL, viewport, 2);
    CHECK(near(boxFor(compacted, 2).x, 510));
    CHECK(near(boxFor(compacted, 1).x, -400));
    compacted = ScrollOverview::SearchLayout::compact(oversized, EAxis::HORIZONTAL, viewport, 3);
    CHECK(near(boxFor(compacted, 3).x, 1020));
    CHECK(near(boxFor(compacted, 1).x, -800));

    return 0;
}
