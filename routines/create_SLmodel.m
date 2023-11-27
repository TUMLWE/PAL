function create_SLmodel(modelname,foldername, repo)
% this routine allows you to create a new simulink model for integration to
% the PLC

%% Modify header



systemTargetFile = 'ert.tlc'; % default is Embedded Coder
TargetLang = 'C';  % target language
%% %%%%%%%%% Initialize Empty Model

ff = fullfile(repo.SimulinkModels,foldername);

if sum(exist(ff, 'dir') == [1 2 4])

    answer = questdlg('The simulink model you are trying to create already exists. If you proceed, all its content will be erased. Do you want to proceed?', ...
        'Warning Dialog', ...
        'Yes','No','No');
    % Handle response
    switch answer
        case 'Yes'
            rmdir(ff,'s');
        case 'No'
            return
            %         error('process interrupted by user.')
        case []
            return
    end


end


mkdir(ff); % create empty folder for simulink model

slmodel = fullfile(repo.SimulinkModels, foldername, modelname);

h = new_system(modelname); % create new simulink model

% h = load_system(slmodel);
cs = getActiveConfigSet(h);

% specify target options
switchTarget(cs, systemTargetFile,[]);
set_param(h, 'TargetLang', TargetLang)
set_param(h, 'SimTargetLang', TargetLang)

% simulation options and solver
set_param(h, 'StartTime', '0')
set_param(h, 'StopTime', 'duration')
set_param(h, 'SolverType', 'fixed-step')
set_param(h, 'Solver', 'FixedStepDiscrete')
set_param(h, 'FixedStep', 'sample_time')

% Load external inputs
set_param(h, 'LoadExternalInput', 'on');

set_param(h, 'ZeroExternalMemoryAtStartup', 'on');
set_param(h, 'ZeroInternalMemoryAtStartup', 'on');
set_param(h, 'ReturnWorkspaceOutputsName', 'off');
set_param(h, 'ReturnWorkspaceOutputs', 'off');

save_system(h, slmodel);
bdclose(slmodel);

%% Create init_ empty routine

initFN = fullfile(repo.SimulinkModels, foldername, ['init_' modelname '.m']);

fid = fopen(initFN,'w');

str = ['function init_' modelname '(params_filename)'];
fprintf(fid, str);

str = ['%%this is the initialization file for the ' modelname ' Simulink model, here you can specify constants and variables necessary for running and compiling your model \n'];
fprintf(fid, str);

fprintf(fid, '\n');
fprintf(fid, 'load(params_filename)\n');
fprintf(fid, '%% Variable definition\n');
fprintf(fid, '\n\n\n\n\n\n');
fprintf(fid, '%% Sect 2: assignments. Here all the defined variables that are useful for running the model are assigned to the parameter file\n');
fprintf(fid, '%% save(params_filename,var_name,''-append'')  %%prototype of the saving call\n\n');

fprintf(fid, 'end \n');

fclose(fid);

%
disp('Empty model successfully created')
end








