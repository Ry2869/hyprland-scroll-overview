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

    const std::vector<SItem> middleHidden = {
        {.id = 1, .box = {0, 0, 100, 100}, .matches = true},
        {.id = 2, .box = {110, 0, 100, 100}, .matches = false},
        {.id = 3, .box = {220, 0, 100, 100}, .matches = true},
    };
    auto compacted = ScrollOverview::SearchLayout::compact(middleHidden, EAxis::HORIZONTAL);
    CHECK(compacted.size() == 2);
    CHECK(near(boxFor(compacted, 1).x, 0));
    CHECK(near(boxFor(compacted, 3).x, 110));

    auto firstHidden = middleHidden;
    firstHidden[0].matches = false;
    firstHidden[1].matches = true;
    compacted = ScrollOverview::SearchLayout::compact(firstHidden, EAxis::HORIZONTAL);
    CHECK(compacted.size() == 2);
    CHECK(near(boxFor(compacted, 2).x, 0));
    CHECK(near(boxFor(compacted, 3).x, 110));

    auto lastHidden = middleHidden;
    lastHidden[1].matches = true;
    lastHidden[2].matches = false;
    compacted = ScrollOverview::SearchLayout::compact(lastHidden, EAxis::HORIZONTAL);
    CHECK(compacted.size() == 2);
    CHECK(near(boxFor(compacted, 1).x, 0));
    CHECK(near(boxFor(compacted, 2).x, 110));

    auto singleMatch = middleHidden;
    singleMatch[0].matches = false;
    compacted = ScrollOverview::SearchLayout::compact(singleMatch, EAxis::HORIZONTAL);
    CHECK(compacted.size() == 1);
    CHECK(near(boxFor(compacted, 3).x, 0));

    auto noMatches = middleHidden;
    noMatches[0].matches = false;
    noMatches[2].matches = false;
    CHECK(ScrollOverview::SearchLayout::compact(noMatches, EAxis::HORIZONTAL).empty());

    const std::vector<SItem> stacked = {
        {.id = 1, .box = {0, 0, 100, 50}, .matches = true},
        {.id = 2, .box = {0, 60, 100, 50}, .matches = false},
        {.id = 3, .box = {0, 120, 100, 50}, .matches = true},
    };
    compacted = ScrollOverview::SearchLayout::compact(stacked, EAxis::HORIZONTAL);
    CHECK(near(boxFor(compacted, 3).y, 60));

    const std::vector<SItem> verticalColumns = {
        {.id = 1, .box = {0, 0, 50, 100}, .matches = true},
        {.id = 2, .box = {0, 110, 50, 100}, .matches = false},
        {.id = 3, .box = {0, 220, 50, 100}, .matches = true},
    };
    const auto verticalCompacted = ScrollOverview::SearchLayout::compact(verticalColumns, EAxis::VERTICAL);
    CHECK(near(boxFor(verticalCompacted, 3).y, 110));

    const std::vector<SItem> secondWorkspace = {
        {.id = 4, .box = {500, 400, 80, 80}, .matches = false},
        {.id = 5, .box = {590, 400, 80, 80}, .matches = true},
    };
    const auto secondRow = ScrollOverview::SearchLayout::compact(secondWorkspace, EAxis::HORIZONTAL);
    CHECK(near(boxFor(secondRow, 5).x, 500));
    CHECK(near(boxFor(compacted, 1).x, 0));

    auto allMatching = middleHidden;
    for (auto& item : allMatching)
        item.matches = true;
    const auto restored = ScrollOverview::SearchLayout::compact(allMatching, EAxis::HORIZONTAL);
    CHECK(near(boxFor(restored, 3).x, 220));

    return 0;
}
