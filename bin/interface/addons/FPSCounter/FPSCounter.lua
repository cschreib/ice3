FPSCounter = {
    ["updateTimer1"] = 0.5, -- Time to wait between each update of the FPS
    ["frameNbr"]     = 0,   -- Frame counter
    ["FPS"]          = 0,   -- The FPS as it is displayed on the screen
    
    ["updateTimer2"]    = 0.2, -- Time to wait between each update of the render time
    ["totalRenderTime"] = 0,   -- Total render time between each update
    ["totalUpdateTime"] = 0,   -- Total render time between each update
    ["frameNbr2"]       = 0,   -- Total number of frames between two updates
    ["renderTime"]      = 0,   -- Render time displayed on screen
    ["updateTime"]      = 0,   -- Render time displayed on screen
};

function FPSCounter.on_update()
    FPSCounter.updateTimer1 = FPSCounter.updateTimer1 + arg1;
    FPSCounter.frameNbr = FPSCounter.frameNbr + 1;
    if (FPSCounter.updateTimer1 >= 0.5) then
        FPSCounter.FPS = math.floor(FPSCounter.frameNbr / FPSCounter.updateTimer1);
        FPSCounter.updateTimer1 = 0;
        FPSCounter.frameNbr = 0;
    end
    
    local chunk, chunkTot, vertex, renderTime, updateTime, updQueuedChunks, crtdQueuedChunk = get_render_stats();
    
    FPSCounter.updateTimer2 = FPSCounter.updateTimer2 + arg1;
    FPSCounter.frameNbr2 = FPSCounter.frameNbr2 + 1;
    FPSCounter.totalRenderTime = FPSCounter.totalRenderTime + renderTime;
    FPSCounter.totalUpdateTime = FPSCounter.totalUpdateTime + updateTime;
    if (FPSCounter.updateTimer2 >= 0.2) then
        FPSCounter.renderTime = math.floor(FPSCounter.totalRenderTime / FPSCounter.frameNbr2);
        FPSCounter.updateTime = math.floor(FPSCounter.totalUpdateTime / FPSCounter.frameNbr2);
        FPSCounter.updateTimer2 = 0;
        FPSCounter.frameNbr2 = 0;
        FPSCounter.totalRenderTime = 0;
        FPSCounter.totalUpdateTime = 0;
    end
    
    FPSCounter_Text:set_text("FPS : "..FPSCounter.FPS
        .."\nWorld render : "..FPSCounter.renderTime
        .."\nWorld update : "..FPSCounter.updateTime
        .."\nWorld total : "..(FPSCounter.renderTime+FPSCounter.updateTime)
        .."\nChunks : "..chunk.."/"..chunkTot
        .."\nChunks upd : "..updQueuedChunks
        .."\nChunks crtd : "..crtdQueuedChunk
        .."\nVertex : "..vertex);
    
    local x, y, z = get_current_unit_pos();
    local info;
    if (x and y and z) then
        info = "ux : "..math.floor(x)..", uy : "..math.floor(y)..", uz : "..math.floor(z);
    else
        info = "ux : --, uy : --, uz : --";
    end
        
    x, y, z = get_selected_block_pos();
    if (x and y and z) then
        info = info.."\nx : "..x..", y : "..y..", z : "..z;
    else
        info = info.."\nx : --, y : --, z : --";
    end
    
    Info_Text:set_text(info);
    
    local smooth, vbo, shaders = get_render_states();
    
    States_Text:set_text("Smooth lighting : "..smooth.."\nVBOs : "..vbo.."\nShaders : "..shaders);
    
end
