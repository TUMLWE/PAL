function Flag = can_create_sl_model(wfcfw, sm)
% this routine checks whether the submodel has all the simulink settings
% defined, such as model name and model folder
   
    Flag = false;
    if ~(strcmp(sm.slmodel_name,string) || strcmp(sm.slmodel_folder,string))
        Flag = true;
    end

%     Flag = ~(isempty(sm.slmodelName) || isempty(sm.slmodel));
%     Flag = ~(isempty(sm.slmodelName) || isempty(sm.slmodel));

    
    
end