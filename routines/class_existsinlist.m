function flag = class_existsinlist(list, item)

flag = false;

    for ielem = 1 : length(list)

        if strcmp(list(ielem).TagName,item.TagName) && strcmp(list(ielem).VarName,item.VarName) && ...
                strcmp(list(ielem).AppName,item.AppName)
            flag = true;
            return
        end

    end


end
