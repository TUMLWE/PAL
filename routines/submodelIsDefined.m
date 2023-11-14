function Flag = submodelIsDefined(wfcfw, sm)

Flag = false;

sf = sm.modelTag;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.PLCApp_folder;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.slmodel_folder;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.slmodel_name;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.slccode_folder;
if isempty(sf) || strcmp(sf,string)
    return
end

sf = sm.refC_name;
if isempty(sf) || strcmp(sf,string)
    return
end

% sf = sm.host_apptag;
% if isempty(sf) || strcmp(sf,string)
%     return
% end

Flag = true;

end