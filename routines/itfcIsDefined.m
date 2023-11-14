function Flag = itfcIsDefined(wfcfw, sm)

Flag = false;

sf = sm.appTag;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.PLCApp_folder;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.refC_name;
if isempty(sf) || strcmp(sf,string)
    return
end

Flag = true;

end