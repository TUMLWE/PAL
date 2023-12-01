% use this routine after creating the simulink models for your WFC
% framework, to test its correct operation on Simulink level

bdclose('all');
close all
clear all
clc
rmpath(genpath('.'));


Flag_create_empty_sl_host = 0; % if this flag is on, it will initialize an empty simulink model, 
                               % you need to add your sub models in there and connect them according to your needs 

%% Step 1: load submodels from input file
framework_inputs = 'InputFiles_WFC.xlsx';

wfcfw = wfc_framework;  % initialize empty object
wfcfw.create_new(framework_inputs);
wfcfw.add_folders(); % add matlab folders to current paths


%% Step 2: create the "host" simulink model. Not necessary if model is already present

to = test_wfc(framework_inputs, wfcfw); % initialize test object; 


if Flag_create_empty_sl_host
    duration = 40; % this is just for initialization purposes
    sample_time = 0.1;  % This is 10 Hz for Kirch Mulsow

    wfcfw.settings.sl_sim.duration = duration;  % this is just for initialization purposes
    wfcfw.settings.sl_sim.sample_time = sample_time; 
    wfcfw.settings.sl_sim.t = [0 : sample_time : duration]';

    to.create_empty_slmodel()
    % here you must detail the model with all the various submodels
    to.load_model_params();

    
    
end

%% Step 2.5: if model exists, no need to create a new one, just load the existing one
to.load_model_params(wfcfw);


%% Step 3: define host model inputs
modelfolder = '20230228_validation'; 
modelname = 'ValidModel';
sl_model = fullfile(repo.SimulinkModelsMainFolder, modelfolder, modelname);




