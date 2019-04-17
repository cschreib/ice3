
function ItemSelector:set_slot_texture(slot_id, item, quality)
    local slot = self["Slot"..(slot_id+1)];
    if (not slot) then return end;
    
    local file;
    local u1, u2, v1, v2;
    local a = 1.0;
    local r = 1.0;
    local g = 1.0;
    local b = 1.0;
    
    if (item < 256) then
        file = "textures/terrain.png";
        
        u1, v1, u2, v2 = get_block_uv(item);
        
        if (item == 18 or item == 2) then
            a = 1.0; r = 0.6; g = 1.0; b = 0.4;
        end
        
    elseif (item == 256) then
        slot:SetTexture("");
        return;
    
    elseif (item < 261) then
        file = "textures/items.png";
        
        u1 = (quality-1)/16.0;
        v1 = 4.0/16.0 + (item-257)/16.0;
        
        u2 = u1 + 1.0/16.0;
        v2 = v1 + 1.0/16.0;
    
    else
        log("Warning : ItemSelector : no texture defined for item "..item.." (quality : "..quality..")");
        return;
        
    end
    
    slot:set_texture(file);
    slot:set_tex_coord(u1, u2, v1, v2);
    slot:set_vertex_color(r, g, b, a);
end
