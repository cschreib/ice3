
function KeyConfig:set_binded_key(key)

    if (key) then
        self.bindedKey = key:get_parent();
        self.Selector:show();
        self.Selector:set_all_points(key);
    else
        self.bindedKey = nil;
        self.Selector:clear_all_points();
        self.Selector:hide();
    end
    
end
