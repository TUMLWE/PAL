classdef submodel < handle
    
    properties
        
        modelTag {mustBeText} = strings
        PLCApp_folder {mustBeText} = strings
        slmodel_folder {mustBeText} = strings
        slmodel_name {mustBeText} = strings 
        slccode_folder {mustBeText} = strings
        refC_name {mustBeText} = strings
        Flag_Generate_PLC_app {mustBeNumericOrLogical} = false
        Comment {mustBeText} = strings
        params_filename {mustBeText} = strings
        host_apptag {mustBeText} = strings
        Flags 
        Variables
        
    end
    
    methods
        
        function obj = create_new(obj, inputs)

            for i = 1 : length(inputs.Properties.VariableNames)
                vn = inputs.Properties.VariableNames{i};
                value = inputs.(vn);
                
%                 if strcmp(vn,'Comment') && isnan(value)
%                     obj.(vn) = strings;
%                     break
%                 end
                
                if strcmp(vn,'modelTag') || strcmp(vn,'PLCApp_folder') || strcmp(vn,'slmodel_folder') || ...
                        strcmp(vn,'slmodel_name') || strcmp(vn,'slccode_folder') || strcmp(vn,'refC_name') || ...
                        strcmp(vn,'host_apptag') || strcmp(vn,'Comment')
                    value = recastToType(vn, value,'string');
                    obj.(vn) = value;                    
                end                
                
                if strcmp(vn,'Flag_Generate_PLC_app') 
                    value = recastToType(vn, value,'logical');
                    obj.(vn) = value;                    
                    
                end

                
%                 if isa(value,'double') % flags or numbers
%                     obj.(vn) = value;
%                 elseif isa(value,'cell')
%                     obj.(vn) = value{1}; % this works for strings
%                 end
            end
            
            
%         Flag_SLmodel_Ready {mustBeNumericOrLogical} = false 
%         Flag_PLCgen_Ready {mustBeNumericOrLogical} = false 
            obj.Flags = submodel_Flags;
            
        end
        
        function obj = add_folder(obj, main_folder)
            % main_folder: this is the repository folder in which all
            % simulnk models are stored
            addpath(fullfile(main_folder, ...
                obj.slmodel_folder))
            
        end
        
        function obj = load_model_params(obj, repo, settings, runmodel_inputs)
            % this method saves the variables specified in the init_ file
            % to model workspaces, then loads it to base workspace, needed
            % for c code generation
            
            DestinationFolder = fullfile(repo.SimulinkCCode, obj.slccode_folder);
            slfolder = fullfile(repo.SimulinkModels, obj.slmodel_folder);
            
            % Get the current configuration
            cfg = Simulink.fileGenControl('getConfig');
            
            % Change the parameters to non-default locations
            % for the cache and code generation folders
            mkdir(fullfile(DestinationFolder, 'matlab'))
            cfg.CacheFolder = char(fullfile(DestinationFolder, 'matlab'));
            cfg.CodeGenFolder = char(fullfile(DestinationFolder, 'matlab'));
            % cfg.CodeGenFolderStructure = 'TargetEnvironmentSubfolder';
            Simulink.fileGenControl('setConfig', 'config', cfg);
            
            %%% Get SL model input
            
            % For some reason, it needs to load some cache data before being able to read the inputs, ...
            % so here it is forced to crash
% % % %             try
% % % %                 sim(obj.slmodelName)
% % % %             catch
% % % %             end
% % % %             
            load_system(obj.slmodel_name)            
%             mdlWks = get_param(obj.slmodelName,'ModelWorkspace');
            
            h_Inports  = find_system(obj.slmodel_name,'FindAll','On','SearchDepth',1,'BlockType','Inport');
            Name_Inports  = get(h_Inports,'Name');
            if ischar(Name_Inports) % if there is only one input element
                Name_Inports = {Name_Inports};
            end
            
            % try to load model workspace
            params_fn = fullfile(slfolder, [obj.slmodel_name '_params.mat']); %% adjust this

            obj.params_filename = params_fn;

            delete(params_fn);
           
            if isempty(runmodel_inputs)

            %%% Load simulink model data (needed to use simulink coder)
            sample_time = settings.sample_time;
            duration = 40;  % this number is not important, since it is only used for initialization purposes
              
            % generate fictitious inputs
            t = [0 : settings.sample_time : duration]';            
            u = rand(length(t), length(Name_Inports));
                        
            else
                t = runmodel_inputs.t;
                u = runmodel_inputs.u;
                sample_time = runmodel_inputs.sample_time;
                duration = runmodel_inputs.duration;
            end

            save(obj.params_filename,'t')
            save(obj.params_filename,'u','-append')
            save(obj.params_filename,'sample_time','-append')
            save(obj.params_filename,'duration','-append')
                        
            % assign few variables to model workspace
%             save_system()
            
            init_fn = dir(fullfile(slfolder,'init_*.m'));
            
            if ~isempty(init_fn) % no need to run it if file is not existing 
                
                fn = split(init_fn.name,'.');
                hh = str2func(fn{1});                
                hh(obj.params_filename);
                
%                 main_dir = pwd; 
%                 cd(slfolder);
%                 cd(main_dir);
            end
            
            % load workspace in base
            evalin('base', ['load(''' obj.params_filename ''')'])
        
        end
        
        function obj = generate_plc_code(obj, repo, settings, host)
            
            if obj.Flag_Generate_PLC_app && obj.Flags.PLC_codeGen_Ready
                
                bdclose('all');
                ReferenceCFiles = repo.ReferenceCFiles;
                DestinationFolder = fullfile(repo.SimulinkCCode, obj.slccode_folder);
                PLCFolder = fullfile(repo.PLCApps, obj.PLCApp_folder);

%                 InterfaceFilesFolder = repo.InterfaceFilesFolder;
% 
%                 ExcelPLCFileName = settings.ExcelPLCFileName;
%                 ExcelIOFileName = settings.ExcelIOFileName;
                
                ModelName = obj.slmodel_name;
                AppName = obj.refC_name;
                input_params_fn = obj.params_filename;
                               
% % %                 PLCCodeGeneratorV3(ModelName, AppName, DestinationFolder, BachmannFolder, ...
% % %                     ReferenceCFiles, ExcelPLCFileName, ExcelIOFileName, InterfaceFilesFolder, ...
% % %                     input_params_fn);

                if strcmp(settings.PLC_system,'Bachmann')
                PLCCodeGeneratorV4(ModelName, AppName, DestinationFolder, PLCFolder, ...
                    ReferenceCFiles, input_params_fn, obj, settings.plcdict, host);
                else
                    error('PLC system not found. So far, only Bachmann can be used')
                end
            end
            
        end
        
        function obj = create_empty_slmodel(obj, wfcfw)

            repo = wfcfw.folders.repo;
            modelname = obj.slmodel_name;
            foldername = obj.slmodel_folder;
            
            create_SLmodel(modelname,foldername, repo);
            add_folder(obj, repo.SimulinkModels);
                                           
        end
        
        function obj = add_matlabcodegen_fields(obj, wfcfw)

            repo = wfcfw.folders.repo;
            
            SourceRefPath = fullfile(repo.ReferenceCFiles, 'Originals'); % this should be the folder where you put the original bachmann source file
            SourceRefFileName = [obj.refC_name '_app.c'];
            TargetFolder = repo.ReferenceCFiles;            
            HostName = obj.host_apptag;

            % create new file (20230216 - CarloSucameli: so far the code supports only Bachmann systems)
            create_matlabcodegen_fields(SourceRefPath, SourceRefFileName, TargetFolder , HostName)
               
        end
        
        function obj = CheckFlags(obj, wfcfw)
            
           obj.Flags.C_ref_found = find_C_ref(wfcfw, obj);
           obj.Flags.C_ref_codegen_found = find_C_ref_matlabcodegen(wfcfw, obj);  % this finds the reference C file with the matlab fields necessary for code generation
           obj.Flags.can_create_sl_model = can_create_sl_model(wfcfw, obj);
           obj.Flags.isDefined = submodelIsDefined(wfcfw, obj);
           
           obj.Flags.SLmodel_Ready = flag_SLmodel_ready(wfcfw, obj);
           
           obj.Flags.PLC_codeGen_Ready = obj.Flags.C_ref_codegen_found && obj.Flags.isDefined && obj.Flags.SLmodel_Ready; % this tells me that i'm ready to compile

        end

        function obj = check_variables(obj)
            
            % check if sl variables portnumbers are not unique
            if isempty(obj.Variables)
                return
            else

                input_flag = cell2mat(cellfun(@(x) any(strcmpi(x,{'input'})), {obj.Variables.IO},'UniformOutput',false));
                
                if length(unique([obj.Variables(input_flag).PortNumber])) ~= length([obj.Variables(input_flag).PortNumber])
                    error('Error in submodel %s: input Portnumbers are not unique', obj.modelTag)
                end
                output_flag = cell2mat(cellfun(@(x) any(strcmpi(x,{'output'})), {obj.Variables.IO},'UniformOutput',false));

                if length(unique([obj.Variables(output_flag).PortNumber])) ~= length([obj.Variables(output_flag).PortNumber])
                    error('Error in submodel %s: output Portnumbers are not unique', obj.modelTag)
                end

            end



        end



        function obj = load_svi_variables(obj, fw)

            SVI_fn = fw.settings.ExcelPLCFileName;
%             InterfaceFilesFolder = fw.folders.repo.InterfaceFilesFolder;

            opts = detectImportOptions([SVI_fn],'Sheet','Submodels');
%             opts = setvartype(opts, 'parent_SubVar', 'string');  %or 'char' if you prefer
            submodels_table = readtable([SVI_fn],opts,'Sheet','Submodels' );

%             varlist = [];
            for iVar = 1 : length(submodels_table.TagName)

                varlist(iVar,1) = sm_variable(submodels_table(iVar,:), fw.host, fw.itfc);

            end

            if exist('varlist','var')
                sm_out = varlist(strcmp(submodels_table.modelTag, obj.modelTag),:);
            else
                sm_out = [];
            end


            obj.Variables = sm_out;

        end

        function obj = run_simulink_test(obj, fw)

            u = [];
            % find model input in host test time histories

            ref_x = [];
            for iElem = 1 : length(obj.Variables)

                var = obj.Variables(iElem);

                % perform some checks. If it was created, if it is an
                % input, if it was read from the host
                if ~var.Create
                    continue
                end
                    
                if ~strcmpi(var.IO,'input')
                    continue
                end

                if ~strcmpi(var.Action,'read')
                    continue
                end
                

                yh = var.parent.test_outputs.hostvar_th;
                xh = var.parent.test_outputs.hostvar_timestamp;

                if isempty(ref_x) || isequal(ref_x, xh) 
                    ref_x = xh; 
                    u(:,var.PortNumber) = yh;
                    
                elseif ~isequal(ref_x, xh)

                    u(:,var.PortNumber) = interp1(xh, yh, ref_x);

                end
                                
                host_freq{var.PortNumber} = var.parent.output_freq;
                host_app{var.PortNumber} = var.parent_App;
            end

            if ~strcmpi(host_freq{1},'fast') && length(unique(host_freq))~=1
                warning(['Reference time for submodel %s is set to %s. This could causes potential downsampling \n' ... 
                    ' for the fast entries of submodel. If this is not an issue, proceed'], obj.modelTag, host_freq{1})
                keyboard
            elseif strcmpi(host_freq{1},'fast') && length(unique(host_freq))~=1
                warning('Not all inputs of submodel %s are sampled at the same sample time. some inputs have been reinterpolated')

            end


%             if length(unique(host_freq))~=1 % CRS 20231019, at the moment, only a single output_freq must be specified for all inputs
%                 warning('Cannot run Simulink model %s, host input variables do not all have the same output_freq!!', obj.modelTag)
%                 return
%             end

            
            if length(unique(host_app))~=1 % CRS 20231019, at the moment, only a single host_app must be specified for all inputs
                warning('Cannot run Simulink model %s, host input variables do not all have the same host app!!', obj.modelTag)
                return
            end

            switch host_freq{1}

                case 'fast'
                    sample_time = fw.settings.sample_time * fw.host.(host_app{1}).FAST_FREQ_RATIO;

                case 'slow'
                    sample_time = fw.settings.sample_time * fw.host.(host_app{1}).SLOW_FREQ_RATIO  ;

                case 'ctrl'
                    sample_time = fw.settings.sample_time * fw.host.(host_app{1}).CTRL_FREQ_RATIO;
            end

            
            t = [0 : size(u,1)-1]'*sample_time;
            duration = t(end);

            if sum(isnan(u(:)))>0 || sum(isinf(u(:)))>0

                warning('Found NaN of Inf values in inputs of Simulink model %s, if you proceed, they will be substituted with zeroes.')
                keyboard

                %% CRS 20231020: if other substitutions are desired for
                % the input values of the simulink model, they should be
                % done here

                u(isnan(u)) = 0;
                u(isinf(u)) = 0;

                %
            end

            run_inputs.t = t;
            run_inputs.u = u;
            run_inputs.duration = duration;
            run_inputs.sample_time = sample_time;

            obj.load_model_params(fw.folders.repo, fw.settings, run_inputs);
          
            % now we run the simulink model
            sim(obj.slmodel_name);


            % we cycle again to extract the outputs and assign them
            for iElem = 1 : length(obj.Variables)

                var = obj.Variables(iElem);

                % perform some checks. If it was created, if it is an
                % input, if it was read from the host
                if ~var.Create
                    continue
                end
                    
                if ~strcmpi(var.IO,'output')
                    continue
                end

                if ~strcmpi(var.Action,'write')
                    continue
                end


                sl_out = squeeze(yout{var.PortNumber}.Values.Data);
                sl_out_t = squeeze(tout);

                var.parent.test_outputs.sl_th = sl_out;
                var.parent.test_outputs.sl_time_stamp = seconds(sl_out_t) + var.parent.test_outputs.hostvar_timestamp(1);
                var.parent.test_outputs.sl_model_tagname = obj.modelTag;

%                 check_out{var.PortNumber} = var.parent.test_outputs.hostvar_th;

            end




        end

    
 
    end
end