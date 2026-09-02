set_project("spritorium")
set_version("0.1.0")
set_languages("cxx20")
set_arch("x86_64")

add_rules("mode.debug", "mode.release")

add_requires("libsdl3 3.4.0")
add_requires("spdlog 1.17.0")
add_requires("zpp_bits 4.7.1")

target("imgui")
    set_kind("static");

    add_packages("libsdl3")

    add_defines("IMGUI_DEFINE_MATH_OPERATORS", { public = true })
    add_defines("IMGUI_DISABLE_OBSOLETE_FUNCTIONS", { public = true })

    add_includedirs({
        "spritorium/vendor/imgui",
        "spritorium/vendor/imgui/backends",
    }, { public = true })

    add_files(
        "spritorium/vendor/imgui/*.cpp",
        "spritorium/vendor/imgui/backends/imgui_impl_sdl3.cpp",
        "spritorium/vendor/imgui/backends/imgui_impl_opengl3.cpp"
    );

target("main")
    set_kind("binary")
    add_rules("utils.bin2c", {extensions = {".vert", ".frag"}})
    add_files("spritorium/src/shaders/*.frag", "spritorium/src/shaders/*.vert")

    add_packages("libsdl3")
    add_packages("spdlog")
    add_packages("zpp_bits")

    add_includedirs(
        "spritorium/src",
        "spritorium/vendor/misc",
        "spritorium/vendor/glad/include",
        "spritorium/vendor/stb"
    )

    add_files(
        "spritorium/src/**.cpp",
        "spritorium/vendor/glad/src/glad.c"
    )

    add_deps("imgui");

    before_build(function (target)
        if is_mode("debug") then
            cprint("\n${red}DEBUG MODE")
        elseif is_mode("release") then
            cprint("\n${cyan}RELEASE MODE")
        end

        cprint("- ${bright}target: %s", target:name())

        print()
    end)

    if is_mode("debug") then
        add_cxxflags("-O0 -march=native")

        add_defines("DEBUG")
        set_symbols("debug") -- force -g debug symbols
    end

    if is_mode("release") then
        add_cxxflags("-O2 -flto=auto")
        add_ldflags("-s")

        if is_plat("windows") or is_plat("mingw") then
            add_ldflags("-mwindows", { force = true })
        end
    end

    if is_plat("windows") or is_plat("mingw") then
        after_build(function(target)
            local target_dir = target:targetdir()
            os.cp("resources", target_dir, { rootdir = "." })
            cprint("${green}[Xmake] Build complete: %s", target_dir)
        end)
    end