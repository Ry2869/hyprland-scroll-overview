# ScrollOverview

ScrollOverview is an overview plugin like niri.

https://github.com/user-attachments/assets/e5eb1ad2-79bc-492a-82cd-02cd8b960d3e

## Installation

### Using Hyprpm (recommended)

1. Add the plugin repository:
   ```bash
   hyprpm add https://github.com/yayuuu/hyprland-scroll-overview.git
   ```
   If you use a Git build of Hyprland, add the plugin from the `new-release` branch instead:
   ```bash
   hyprpm add https://github.com/yayuuu/hyprland-scroll-overview origin/new-release
   ```
2. Build and fetch dependencies:
   ```bash
   hyprpm update
   ```
3. Enable the plugin:
   ```bash
   hyprpm enable scrolloverview
   ```
4. Configure and Enjoy.

For more installation methods, including building from source and loading the plugin automatically, see the [Installation guide](https://github.com/yayuuu/hyprland-scroll-overview/wiki/Installation).

## Configuration

### Hyprlang

```ini
# .config/hypr/hyprland.conf
plugin {
    scrolloverview {
        gesture_distance = 300 # how far is the "max" for the gesture
        scale = 0.5 # preferred overview scale
        workspace_gap = 100
        layout = vertical # vertical, horizontal, or auto (per-monitor orientation)
        wallpaper = 2 # 0: global only, 1: per-workspace only, 2: both
        blur = true # blur only the main overview wallpaper

        search {
            enabled = true # type while the overview is open to filter windows
            background_color = rgba(111318ff)
            background_opacity = 0.92
            text_color = rgba(f2f4f8ff)
            placeholder_color = rgba(a0a7b4ff)
            placeholder_opacity = 0.58
            border_color = rgba(5b6472ff)
            border_opacity = 0.75
            border_thickness = 2
            accent_color = rgba(7aa2f7ff)
            error_color = rgba(ff5c57ff)
            corner_radius = -1 # -1 follows decoration:rounding
            padding_x = 18
            padding_y = 12
            font_size = 16
            width_ratio = 0.5
            max_width = 640
            top_offset = 20
        }

        shadow {
            enabled = true
            range = 50
        }
    }
}

# Toggle ScrollOverview with SUPER+g
bind = SUPER, g, scrolloverview:overview, "toggle all"
```

### Lua

```lua
-- .config/hypr/hyprland.lua
hl.config({
    plugin = {
        scrolloverview = {
            gesture_distance = 300, -- how far is the "max" for the gesture
            scale = 0.5, -- preferred overview scale
            workspace_gap = 100,
            layout = "vertical", -- vertical, horizontal, or auto (per-monitor orientation)
            wallpaper = 2, -- 0: global only, 1: per-workspace only, 2: both
            blur = true, -- blur only the main overview wallpaper

            shadow = {
                enabled = true,
                range = 50,
            },
        },
    },
})

-- Toggle ScrollOverview with SUPER+g
hl.bind("SUPER + g", function()
    hl.plugin.scrolloverview.overview("toggle all")
end)
```

Set `cross_monitor_drag = true` to drag windows between monitors through ScrollOverview and adopt Hyprland move drags when another overview is open. It defaults to `false`. Native adoption uses Hyprland internals; if unavailable, overview-origin cross-monitor dragging still works.

This setting does not affect overview-origin drags with `toggle all` because every overview is already open. Existing multi-monitor commands remain available when it is disabled.

## Type-to-search

With the overview open, typing text filters windows by title and application class/app ID. The query is shared by all open monitor overviews. Matching tiled windows are visually compacted and centred in each workspace's usable area (including reserved bar space), preserving their order without changing the real Hyprland layout. Floating and pinned windows keep their normal positions. Clearing the query restores the exact unfiltered overview geometry.

If a compacted result group is larger than its workspace's usable area, it is not resized. The selected result is centred where the group bounds permit; at an outer edge, the group is clamped so the viewport does not expose empty space. Arrow-key navigation updates this filtered presentation while the real scrolling layout continues to provide normal focus synchronization.

The top-centred native search card shows a placeholder before typing and a per-monitor result count. Its colors, opacity, border, sizing, rounding, and placement are configurable through the generic `search` options above, so external theme tooling can update it without the plugin depending on a particular desktop shell.

Use Backspace to edit, Escape to clear a non-empty query (or close when it is empty), the arrow keys to navigate matches, and Enter to activate the selected match. Set `search.enabled = false` to retain the pre-search keyboard behavior.

Search input supports UTF-8 characters produced directly by the active XKB layout. Full input-method composition through fcitx/ibus is not currently available because the plugin does not own a Wayland text-input surface. Command-modified keys such as `SUPER+SHIFT+Left` remain available to Hyprland and overview submaps.

## Documentation

The complete documentation is available in the [ScrollOverview wiki](https://github.com/yayuuu/hyprland-scroll-overview/wiki). It covers all configuration options, keybinds, submaps, gestures, dispatchers, and advanced Lua examples.

## Supported plugins

- `hyprbars`

<br>
<br>

## Star History

<a href="https://www.star-history.com/?repos=yayuuu%2Fhyprland-scroll-overview&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/chart?repos=yayuuu/hyprland-scroll-overview&type=date&theme=dark&legend=top-left&sealed_token=dnoTQqdWontVtW1U-Qv5yJAQVYpafoKJ0ytKAwVNsCxYO8lor7ZbsD4oVIRHBW_9LjjRPMV-wHAXTw70cn4SbI_2OIrdFEMbOyjv9GVHi9EOBFIgwzKwvMTGWBhkYg5q1f9oV6mQqQZFgec__fA790Nj_OS5WOyVjeHFwceIJRFBlgSvBWyGMn3WUqym" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/chart?repos=yayuuu/hyprland-scroll-overview&type=date&legend=top-left&sealed_token=dnoTQqdWontVtW1U-Qv5yJAQVYpafoKJ0ytKAwVNsCxYO8lor7ZbsD4oVIRHBW_9LjjRPMV-wHAXTw70cn4SbI_2OIrdFEMbOyjv9GVHi9EOBFIgwzKwvMTGWBhkYg5q1f9oV6mQqQZFgec__fA790Nj_OS5WOyVjeHFwceIJRFBlgSvBWyGMn3WUqym" />
   <img alt="Star History Chart" src="https://api.star-history.com/chart?repos=yayuuu/hyprland-scroll-overview&type=date&legend=top-left&sealed_token=dnoTQqdWontVtW1U-Qv5yJAQVYpafoKJ0ytKAwVNsCxYO8lor7ZbsD4oVIRHBW_9LjjRPMV-wHAXTw70cn4SbI_2OIrdFEMbOyjv9GVHi9EOBFIgwzKwvMTGWBhkYg5q1f9oV6mQqQZFgec__fA790Nj_OS5WOyVjeHFwceIJRFBlgSvBWyGMn3WUqym" />
 </picture>
</a>
