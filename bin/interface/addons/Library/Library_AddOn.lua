-- AddOn class definition
AddOn = CreateClass(function (addon, name, version)
    addon.name = name;
    if (version ~= nil) then addon.version = version; end
    addon.locale = {};
end);

-- Member functions
function AddOn:GetInfos()
    local str = self.name;
    if (self.version ~= nil) then
        str = str..", version : "..self.version;
    end
    
    return str;
end

function AddOn:SetLocale(locale)
    self.locale = locale;
end

function AddOn:AddLocalizedString(key, value)
    self.locale[key] = value;
end

function AddOn:GetLocalizedString(key)
    if (self.locale[key]) then
        return self.locale[key];
    else
        Warning(self.name.." : Missing translation for : \""..key.."\".");
        return "<"..key..">";
    end
end

AddOns = {};
