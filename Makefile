CXX ?= g++

EXTRA_FLAGS =
LUA_PKG ?= $(shell pkg-config --exists 'lua5.4 >= 5.4' && echo lua5.4 || echo lua)
VERSION_HEADER = .build/PluginVersion.hpp
VERSION_SCRIPT = scripts/generate-plugin-version.sh

ifeq ($(CXX),g++)
    EXTRA_FLAGS += -fno-gnu-unique
endif

.PHONY: all clean test FORCE

all: $(VERSION_HEADER)
	$(CXX) -shared -fPIC $(EXTRA_FLAGS) -I.build main.cpp Config.cpp DropIndicator.cpp NativeDrag.cpp OverviewGesture.cpp OverviewManager.cpp OverviewPassElement.cpp OverviewRender.cpp Search.cpp SearchLayout.cpp Window.cpp scrollOverview.cpp -o scrolloverview.so -g `pkg-config --cflags pixman-1 glib-2.0 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon '$(LUA_PKG) >= 5.4'` `pkg-config --libs glib-2.0` -std=c++2b -Wno-narrowing

test:
	$(CXX) -std=c++23 -I. Search.cpp SearchLayout.cpp tests/SearchTests.cc `pkg-config --cflags --libs glib-2.0` -o /tmp/scrolloverview-search-tests
	/tmp/scrolloverview-search-tests

$(VERSION_HEADER): FORCE $(VERSION_SCRIPT)
	sh $(VERSION_SCRIPT) $@ .

FORCE:

clean:
	rm -f ./scrolloverview.so $(VERSION_HEADER)
