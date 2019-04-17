function ButtonTemplate_Menu:AddItem(name, caption, func)
    local item = CreateFrame("Button", "$parent"..name, self.Sub, "ButtonTemplate_MenuItem");
    if (not item) then
        return;
    end
        
    if (not self.last_item) then
        item:SetPoint("TOPLEFT",  self.Sub, "TOPLEFT",   5, 5);
        item:SetPoint("TOPRIGHT", self.Sub, "TOPRIGHT", -5, 5);
        self.sub_height = 30 + 5 + 5;
    else
        item:SetPoint("TOPLEFT",  self.last_item, "BOTTOMLEFT",  0, 3);
        item:SetPoint("TOPRIGHT", self.last_item, "BOTTOMRIGHT", 0, 3);
        self.sub_height = self.sub_height + 3 + 30;
    end
    
    if (func) then
        item:SetScript("OnClick", func);
    end
    
    item:SetText(caption);
    
    self.Sub:SetHeight(self.sub_height);
    
    self.last_item = item;
end

ButtonTemplate_Menu:MarkForCopy("AddItem");
