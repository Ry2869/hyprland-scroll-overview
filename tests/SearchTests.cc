#include "Search.hpp"

#include <iostream>
#include <string>

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

    return 0;
}
