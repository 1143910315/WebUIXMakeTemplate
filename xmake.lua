add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")
if is_mode("debug") then
    set_runtimes("MDd")
    add_requires("webui", {debug = true})
    add_requires("date", {debug = true})
else
    set_runtimes("MD")
    add_requires("webui")
    add_requires("date")
end
includes("ui/xmake.lua")

target("WebUIXMakeTemplate")
    add_packages(
        "webui",
        "date"
    )
    add_cxflags(
        "cl::/utf-8",
        "cl::/W4",
        "gcc::-Wall",
        "cl::/INCREMENTAL"
    )
    if is_mode("debug") then
        add_defines("_DEBUG")
        set_symbols("debug", "edit")
    else
        add_ldflags("-subsystem:windows")
    end
    set_kind("binary")
    add_deps("ui")
    add_includedirs("src")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    set_languages("clatest", "cxxlatest")