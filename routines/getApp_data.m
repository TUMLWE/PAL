function appData = getApp_data(wfc_df, modelName)
% this function exctracts the data relevant for PLC code generation for a
% specific sub-app (modelName) and from the main dataframe (wfc_df)

appData.slmodelName = wfc_df.slmodelName.(modelName);
appData.DestinationFolder = fullfile(wfc_df.folders.repo.SimulinkCCodeMainFolder, ...
    wfc_df.folders.slccode.(modelName));

appData.refCfiles = wfc_df.refCfiles.(modelName);

appData.BachmannFolder = fullfile(wfc_df.folders.repo.BachmannMainFolder, ...
    wfc_df.folders.bachmannApp.(modelName));

appData.slfolder = fullfile(wfc_df.folders.repo.SimulinkModelsMainFolder, ...
    wfc_df.folders.slmodel.(modelName));


end
