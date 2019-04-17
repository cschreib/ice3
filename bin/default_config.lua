game_options = {

    -- Game locale : available localizations are in the Locale folder.
    ["game_locale"] = "enGB",

-- Screen resolution

    -- Fullscreen toggle :
    -- 'true' will make the game run in fullscreen mode, with a resolution of :
    --                    screen_width x screen_height
    -- 'false' will make the game run in windowed mode, with a resolution of :
    --                    window_width x window_height.
    -- Most of the time, running the game in fullscreen mode will make it run faster.
    ["fullscreen"] = false,           
    ["window_width"] = 800,
    ["window_height"] = 600,
    ["screen_width"] = 1280,
    ["screen_height"] = 1024,
    
-- Render

    -- Synchronizes the frame rate with your screen's frequency.
    -- The frame rate *will* be lower, but you shouldn't be able to notice.
    -- VSync is always activated in windowed mode.
    ["vsync"] = false,
    -- Enables the use of vertex/fragment shaders (automatically set to 'true' if VBOs are enabled).
    -- Shouldn't affect performances. Shaders can be used to add fancy effects !
    ["enable_shaders"] = true,
    -- Enables the use of vertex buffer objects.
    -- VBOs are a caching mechanism that should improve performances when available.
    ["enable_vbos"] = true,
    -- Enables per vertex lighting and ambient occlusion.
    -- Rendering performances will decrease if you enable this option, but not by much.
    ["enable_smooth_lighting"] = false,
    -- Enables render caching of the GUI using render targets.
    -- Should improve performances at the expense of GPU memory usage.
    -- Note that render targets may not be supported by your graphics card or its current driver.
    
-- World
    
    -- Random seed for terrain generation, integer only (positive or negative).
    ["world_seed"] = 128,
    -- Disables saving, the world doesn't use the hard drive at all and regenerates everything.
    -- If you enable this, the changes you make to the world will never be saved.
    ["no_world_save"] = false,
    -- Duration of the day/night cycle (in seconds).
    ["day_duration"] = 60.0,
    -- Maximum render distance in blocks unit.
    -- In Minecraft, the maximum is 256 (Far) and default is 128 (Normal).
    ["view_distance"] = 200.0,
    -- Maximum number of generated chunks received by the main thread on each frame.
    -- If you have a very fast CPU or if you don't care about performances, you can try to increase
    -- this number. It will make chunk generation faster, at the expense of global performances.
    ["max_generated_chunks"] = 5,
    -- Sets how many times sunlight is updated at dawn and dusk.
    -- Minecraft uses something quite low, like 16.
    -- Use 255 to make it continuous (performances will suffer from this a little).
    ["lighting_update_rate"] = 32,
    -- Various light and sky colors, change at will.
    -- Each color must contain 3 number in the [0.0, 1.0] range.
    ["sky_day_color"]     = {0.470, 0.810, 0.890},
    ["sky_dawn_color"]    = {1.000, 0.682, 0.024},
    ["sky_night_color"]   = {0.000, 0.000, 0.000},
};
