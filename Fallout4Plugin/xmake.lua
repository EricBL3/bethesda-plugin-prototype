-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibf4")

-- set project
local projectName = "Fallout4PrototypePlugin"

set_project(projectName)
set_version("0.0.1")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- Define the plugin output folder for different environments
local fallout4_folder = os.getenv("FALLOUT4_FOLDER")
local fallout4_mods_folder = os.getenv("FALLOUT4_MODS_FOLDER")
local final_target_dir

if fallout4_folder and os.isdir(path.join(fallout4_folder, "Data")) then
    -- Set output folder to Data within the Fallout4 folder
    final_target_dir = path.join(fallout4_folder, "Data")
elseif fallout4_mods_folder and os.isdir(fallout4_mods_folder) then
    -- Set output folder within Mods folder
    final_target_dir = path.join(fallout4_mods_folder, projectName)
end

-- targets
target(projectName)
    -- add dependencies to target
    add_deps("commonlibf4")

    -- add commonlibsse plugin
    add_rules("commonlibf4.plugin", {
        name = projectName,
        author = "Eric",
        description = "F4SE plugin prototype using CommonLibF4"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    -- After build steps: copy the .dll to the Plugins folder
    after_build(function (target)
        local dll_folder = path.join(final_target_dir, "F4SE", "Plugins")

        -- Create the folder if it doesn't exist
        os.mkdir(dll_folder)

        -- Copy the built .dll to the target folder
        os.cp(target:targetfile(), path.join(dll_folder, target:name() .. ".dll"))

        -- If in Debug mode, copy .pdb for debug symbols
        if get_config("mode") == "debug" then
            os.cp(target:targetfile(true), path.join(dll_folder, target:name() .. ".pdb"))
        end
    end)
