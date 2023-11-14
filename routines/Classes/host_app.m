classdef host_app < handle

    properties

        appTag {mustBeText} = strings
        PLCApp_folder {mustBeText} = strings
        refC_name {mustBeText} = strings
        Flag_Generate_PLC_app {mustBeNumericOrLogical} = false
        Comment {mustBeText} = strings
        FAST_FREQ_RATIO {mustBeNumeric} = 1
        SLOW_FREQ_RATIO {mustBeNumeric} = 10
        CTRL_FREQ_RATIO {mustBeNumeric} = 10
        out_filename {mustBeText} = strings
        output_path_in_PLC {mustBeText} = strings
        output_file_duration_s {mustBeNumeric} = 600
        Variables
        Flags

    end

    methods

        function obj = host_app(inputs)

            for i = 1 : length(inputs.Properties.VariableNames)
                vn = inputs.Properties.VariableNames{i};
                value = inputs.(vn);

                if strcmp(vn,'appTag') || strcmp(vn,'PLCApp_folder') || strcmp(vn,'refC_name') || ...
                        strcmp(vn,'Comment') || strcmp(vn,'out_filename') || strcmp(vn,'output_path_in_PLC')
                    value = recastToType(vn, value,'string');
                    obj.(vn) = value;
                end

                if strcmp(vn,'Flag_Generate_PLC_app')
                    value = recastToType(vn, value,'logical');
                    obj.(vn) = value;

                end

                if strcmp(vn,'FAST_FREQ_RATIO') || strcmp(vn,'SLOW_FREQ_RATIO') || ...
                        strcmp(vn,'CTRL_FREQ_RATIO') || strcmp(vn,'output_file_duration_s')
                    value = recastToType(vn, value,'double');
                    obj.(vn) = value;

                end
               

            end

        end

        function obj = load_svi_variables(obj, fw)

            SVI_fn = fw.settings.ExcelPLCFileName;
%             InterfaceFilesFolder = fw.folders.repo.InterfaceFilesFolder;

            opts = detectImportOptions([SVI_fn],'Sheet','HOST');
            opts = setvartype(opts, 'parent_SubVar', 'string');  %or 'char' if you prefer
            host_table = readtable([SVI_fn],opts,'Sheet','HOST' );

            for iVar = 1 : length(host_table.TagName)

                varlist_host(iVar,1) = host_variable(host_table(iVar,:), fw.itfc);

            end

            if exist("varlist_host","var") % 
                host_out = varlist_host(strcmp({varlist_host.AppName}', obj.appTag));
            else
                warning('No host variables were found.')
                host_out = [];
            end

            obj.Variables = host_out;

        end

        function obj = CheckFlags(obj, wfcfw)

           obj.Flags.C_ref_found = find_C_ref(wfcfw, obj);
           obj.Flags.C_ref_codegen_found = find_C_ref_matlabcodegen(wfcfw, obj);  % this finds the reference C file with the matlab fields necessary for code generation
           obj.Flags.isDefined = itfcIsDefined(wfcfw, obj);
           obj.Flags.PLC_codeGen_Ready = obj.Flags.C_ref_codegen_found && obj.Flags.isDefined; % this tells me that i'm ready to compile

        end

        function obj = add_matlabcodegen_fields(obj, wfcfw)

            repo = wfcfw.folders.repo;
            
            SourceRefPath = fullfile(repo.ReferenceCFiles, 'Originals'); % this should be the folder where you put the original bachmann source file
            SourceRefFileName = [obj.refC_name '_app.c'];
            TargetFolder = repo.ReferenceCFiles;

            % create new file (20230216 - CarloSucameli: so far the code supports only Bachmann systems)
            if strcmp(wfcfw.settings.PLC_system,'Bachmann')
            create_matlabcodegen_fields_host(SourceRefPath, SourceRefFileName, TargetFolder)  
            else
                error('PLC system not implemented. Impossible to modify host file');
            end
            
        end

        function obj = generate_plc_code(obj, repo, settings, itfc)

            if obj.Flag_Generate_PLC_app && obj.Flags.PLC_codeGen_Ready
                
                bdclose('all');
                ReferenceCFiles = repo.ReferenceCFiles;
                PLCFolder = fullfile(repo.PLCApps, obj.PLCApp_folder);
%                 InterfaceFilesFolder = repo.InterfaceFilesFolder;
% 
%                 ExcelPLCFileName = settings.ExcelPLCFileName;
                AppName = obj.refC_name;
                 
                dict = settings.plcdict;
                if strcmp(settings.PLC_system,'Bachmann')
                PLCCodeGenerator_HOST(AppName, PLCFolder, ...
                    ReferenceCFiles, obj, dict, settings.sample_time, itfc);
                else
                    error('PLC system not found. So far, only Bachmann can be used')
                end

            end

        end

        function obj = read_outputs(obj, out_fn)

%             dtio = delimitedTextImportOptions( ...
%                 'VariableNamesLine' , 1 , ...
%                 'Delimiter',{'; '} , ...
%                 'PreserveVariableNames',true, ...
%                 'TrailingDelimitersRule', 'ignore','DataLines', 2 ...
%                 );

            fast_files = out_fn(cell2mat(cellfun(@(x) contains(x, [obj.out_filename '_FAST']), out_fn, 'UniformOutput',false)));
            slow_files = out_fn(cell2mat(cellfun(@(x) contains(x, [obj.out_filename '_SLOW']), out_fn, 'UniformOutput',false)));
            ctrl_files = out_fn(cell2mat(cellfun(@(x) contains(x, [obj.out_filename '_CTRL']), out_fn, 'UniformOutput',false)));

            sorted_fast_files = natsortfiles(fast_files');
            sorted_slow_files = natsortfiles(slow_files');
            sorted_ctrl_files = natsortfiles(ctrl_files');

            outmat_fast = table;
            for idf = 1 : length(sorted_fast_files)
%                 T = readtable(sorted_fast_files{idf}, dtio );
%                 T.TimeStamp_UTC = datetime(T.TimeStamp_UTC,"InputFormat",infmt,"Format","yyyy.MM.dd HH:mm:ss.SSS");

                T = read_outfile_host(sorted_fast_files{idf}, obj, 'fast');
                outmat_fast = [outmat_fast; T];
            end

            outmat_slow = table;
            for idf = 1 : length(sorted_slow_files)
%                 T = readtable(sorted_slow_files{idf}, dtio );
%                 T.TimeStamp_UTC = datetime(T.TimeStamp_UTC,"InputFormat",infmt,"Format","yyyy.MM.dd HH:mm:ss.SSS");

                T = read_outfile_host(sorted_slow_files{idf}, obj, 'slow');
                outmat_slow = [outmat_slow; T];
            end

            outmat_ctrl = table;
            for idf = 1 : length(sorted_ctrl_files)
%                 T = readtable(sorted_ctrl_files{idf}, dtio );
%                 T.TimeStamp_UTC = datetime(T.TimeStamp_UTC,"InputFormat",infmt,"Format","yyyy.MM.dd HH:mm:ss.SSS");
 
                T = read_outfile_host(sorted_ctrl_files{idf}, obj, 'ctrl');
                outmat_ctrl = [outmat_ctrl; T];
            end

            % now i recast the variables of the output tables
            

%             for ivar = 1 : length(obj.Variables)
%                 var = obj.Variables(ivar);
% 
%                 if ~var.Create
%                     continue
%                 end
%                 if ~var.Print_output
%                     continue
%                 end
% 
%                 label = [strtrim(var.TagName) var.Units];
% 
%                 switch var.output_freq
%                     case 'fast'
%                         outmat_fast = recast_output_table(outmat_fast , label, var);
%                     case 'slow'
%                         outmat_slow = recast_output_table(outmat_slow , label, var);
%                     case 'ctrl'
%                         outmat_ctrl = recast_output_table(outmat_ctrl , label, var);
%                     otherwise
% 
%                 end
% 
% 
%             end

            
            %% saving outputs mats

            save(obj.out_filename , 'outmat_fast')
            save(obj.out_filename , 'outmat_slow', '-append')
            save(obj.out_filename , 'outmat_ctrl', '-append')

        end


        function obj = shift_and_crop_itfcvar(obj, delay, ts_limits, parent_name)

            itfc_vars_idx = cell2mat(cellfun(@(x) strcmpi(x,parent_name) ,{obj.Variables.parent_App}, 'UniformOutput', false ));

            varlist = obj.Variables(itfc_vars_idx);

            for ivar = 1 : length(varlist)
                
                var = varlist(ivar);

                if var.Create == 0
                    continue
                end
                
                % apply specified shift and crop
                xh = var.test_outputs.hostvar_timestamp;
                yh = var.test_outputs.hostvar_th;

                idx = xh >= ts_limits(1) & xh <= ts_limits(2);
                var.test_outputs.hostvar_timestamp = xh(idx);
                var.test_outputs.hostvar_th = yh(idx);
                

                if strcmpi(var.Action,'read')
                    yi = var.test_outputs.itfc_var_th;
                    xi = var.test_outputs.itfc_var_timestamp - delay;
                    
                    idx = xi >= ts_limits(1) & xi <= ts_limits(2);
                    var.test_outputs.itfc_var_timestamp = xi(idx);
                    var.test_outputs.itfc_var_th = yi(idx);

                end

                if isfield(var.test_outputs,'sl_th')

                    yi = var.test_outputs.sl_th;
                    xi = var.test_outputs.sl_time_stamp; % here we don't subtract the delay, since it's an issue that affects ITFC only
                    
                    idx = xi >= ts_limits(1) & xi <= ts_limits(2);
                    var.test_outputs.sl_time_stamp = xi(idx);
                    var.test_outputs.sl_th = yi(idx);
                    

                end

%                 figure(); hold on
%                 plot(xh, yh)
%                 plot(xi, yi)
%                 plot(xh(idx), yh(idx))
%                 plot(xi(idx), yi(idx))
                
            end

        end


    end

end