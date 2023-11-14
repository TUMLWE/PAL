
appData = getApp_data(wfc_df, modeltag);

ModelName = appData.slmodelName;
DestinationFolder = appData.DestinationFolder;
AppName = appData.refCfiles;
BachmannFolder = appData.BachmannFolder;
slfolder = appData.slfolder;

% Get the current configuration
cfg = Simulink.fileGenControl('getConfig');

% Change the parameters to non-default locations
% for the cache and code generation folders
mkdir(fullfile(DestinationFolder, 'matlab'))
cfg.CacheFolder = fullfile(DestinationFolder, 'matlab');
cfg.CodeGenFolder = fullfile(DestinationFolder, 'matlab');
% cfg.CodeGenFolderStructure = 'TargetEnvironmentSubfolder';
Simulink.fileGenControl('setConfig', 'config', cfg);

%%% Get SL model input

% For some reason, it needs to load some cache data before being able to read the inputs, ...
% so here it is forced to crash
try  
    sim(ModelName)
catch 
end

h_Inports  = find_system(ModelName,'FindAll','On','SearchDepth',1,'BlockType','Inport');
Name_Inports  = get(h_Inports,'Name');

% generate fictitious inputs
t = wfc_df.settings.sl_sim.t; 
u = rand(length(t), length(Name_Inports));

%%% Load simulink model data (needed to use simulink coder)
sample_time = wfc_df.settings.sl_sim.sample_time;

init_fn = dir(fullfile(slfolder,'init_*.m'));

if ~isempty(init_fn) % no need to run it if file is not existing   
    run(fullfile(slfolder,init_fn.name))    
end

% INPUT
if wfc_df.settings.FlagGenerate_BachmannApplications
    bdclose('all');
    ReferenceCFiles = wfc_df.folders.repo.ReferenceCFiles;
    ExcelPLCFileName = wfc_df.settings.ExcelPLCFileName;
    ExcelIOFileName = wfc_df.settings.ExcelIOFileName;
    InterfaceFilesFolder = wfc_df.folders.repo.InterfaceFilesFolder;
    
    PLCCodeGenerator(ModelName, AppName, DestinationFolder, BachmannFolder, ...
        ReferenceCFiles, ExcelPLCFileName, ExcelIOFileName, InterfaceFilesFolder);
    
end


