function Flag = flag_SLmodel_ready(wfcfw, sm)

    Flag = false;

%     SLmainFolder = wfcfw.folders.repo.SimulinkModelsMainFolder;
    SLmainFolder = wfcfw.folders.repo.SimulinkModels;
    
    if sm.slmodel_folder == string
       return 
    end

    if sm.slmodel_name == string
       return 
    end
    
    fn = fullfile(SLmainFolder, sm.slmodel_folder , [sm.slmodel_name '.slx']);
    
    if ~(exist(fn) == 4)        
        warning(['Could not find Simulink model ' fn])
        return
        
    end

    %% Insert other checks on the details of the simulink model

    
        Flag = true;
           
 

end