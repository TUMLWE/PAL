classdef wfc_framework < handle
    properties

        folders
        settings
        sub_models
        itfc
        host
        test
    end
    %
    methods

        function obj = create_new(obj, input_file)

%             addpath('utilities')

            obj.settings.input_file = input_file;
            % define main folders
            table = readtable(input_file,'Sheet','Main Folders');

            if length(table.VariableName)~=length(table.FolderName)
                error('Error reading "%s". VariableName and FolderName have different lengths', input_file)
            end

            for iL = 1 : length(table.VariableName)

                repo.(table.VariableName{iL}) = table.FolderName{iL};

            end

            obj.folders.repo = repo;


            opts = detectImportOptions(input_file,'Sheet','Settings');
            opts = setvartype(opts,'char'); 
            opts.DataRange = 'A2';

            % define settings
            table = readtable(input_file,opts,'Sheet','Settings');
            if length(table.Variable)~=length(table.Value)
                error('Error reading "Settings" sheet in "%s". Variable and Value have different lengths', input_file)
            end

            for iL = 1 : length(table.Variable)
                vn = table.Variable{iL};
                value = table.Value{iL};

                if strcmp(vn,'duration') || strcmp(vn,'sample_time')
                    value = recastToType(vn, value,'double');
                    obj.settings.(vn) = value;
                    continue
                end

                if strcmp(vn, 'FlagGenerate_PLCApps') || strcmp(vn, 'SaveModelParametersFlag') || ...
                        strcmp(vn, 'FlagGenerate_ITFC')
                    value = recastToType(vn, value,'logical');
                    obj.settings.(vn) = value;
                end

                if strcmp(vn, 'ExcelPLCFileName') || strcmp(vn, 'ExcelIOFileName') || ...
                        strcmp(vn, 'ExcelPLCFileName') || strcmp(vn, 'ExcelIOFileName') || ...
                        strcmp(vn, 'PLC_system') || strcmp(vn, 'DataTypes_dict_filename')
                    value = recastToType(vn, value,'string');
                    obj.settings.(vn) = value;
                end

            end

            if ~isempty(obj.settings.DataTypes_dict_filename)
                obj.load_data_types_dict();
            else
                warning('Data types dictionary not specified in main input file!')
            end

            


            %% Define ITFC apps

            table = readtable(input_file,'Sheet','ITFC');
            if ~isequal(length(table.appTag),length(table.PLCApp_folder),length(table.refC_name) ,length(table.Flag_Generate_PLC_app), ...
                   length(table.Flag_Create_test_ITFC), length(table.test_ITFC_filename) )

                error('Error reading "%s". ITFC entries have different lengths', input_file)
            end

            for iL = 1 : length(table.appTag)

                ifc = itfc_app(table(iL,:));
                ifc.load_svi_variables(obj);
                ifc.assign_test_ITFC();
                ifc.checkVariables(obj.settings.plcdict);
                obj.itfc.(ifc.appTag) = ifc;

            end
            %% Define HOST apps 

            table = readtable(input_file,'Sheet','HOST');
            if ~isequal(length(table.appTag),length(table.PLCApp_folder),length(table.refC_name) ,...
                    length(table.Flag_Generate_PLC_app) )

                error('Error reading "%s". HOST entries have different lengths', input_file)
            end

            for iL = 1 : length(table.appTag)

                ha = host_app(table(iL,:));
                ha.load_svi_variables(obj);
%                 ha.checkVariables(obj.settings.plcdict);
                obj.host.(ha.appTag) = ha;

            end

            %% Define sub-app folders
            table = readtable(input_file,'Sheet','Submodels');
            if ~isequal(length(table.modelTag),length(table.PLCApp_folder),length(table.slmodel_folder),length(table.slmodel_name) , ...
                    length(table.slccode_folder),length(table.refC_name) , length(table.Flag_Generate_PLC_app), ...
                    length(table.host_apptag))

                error('Error reading "%s". Submodels entries have different lengths', input_file)
            end

            for iL = 1 : length(table.modelTag)

                sm = submodel;
                sm.create_new(table(iL,:));
                sm.load_svi_variables(obj);
                sm.check_variables();

                obj.sub_models.(sm.modelTag) = sm;

            end

            % create modbus   %% CRS: I disabled modbus for the time being
            
%             obj.modbus_evaluate()

            obj.test = test_framework();

        end

        function obj = add_folders(obj)

            % Add Folders to be used

            addpath(obj.folders.repo.InterfaceFiles)
%             addpath(obj.folders.repo.MatlabRoutines_PLCGenerators)

            % add subfolders
            if isempty(obj.sub_models)
                return
            end
            fn = fieldnames(obj.sub_models);
            for isl = 1 : length(fn)

                sl_main_folder = obj.folders.repo.SimulinkModels;
                sm = obj.sub_models.(fn{isl});
                sm.add_folder(sl_main_folder);

            end

        end

        function obj = load_submodels(obj)

            fn = fieldnames(obj.sub_models);
            for isl = 1 : length(fn)

                sm = obj.sub_models.(fn{isl});

                sm.load_model_params(obj.folders.repo, obj.settings);
                %                     sm.generate_plc_code(obj.folders.repo, obj.settings);

            end

        end

        function obj = generate_plccode_sl(obj)
            % this function generates the PLC code for all the selected sub models

            obj.check();
            Total = sum(obj.settings.Flags_Generate_sm);

            wb = waitbar(0, ['Generating PLC code: ']);

            fn = fieldnames(obj.sub_models);

            incr = 0;
            if obj.settings.FlagGenerate_PLCApps
                for isl = 1 : length(fn)

                    sm = obj.sub_models.(fn{isl});


                    if sm.Flags.SLmodel_Ready


                        sm.load_model_params(obj.folders.repo, obj.settings, []);

                        if   sm.Flag_Generate_PLC_app && sm.Flags.PLC_codeGen_Ready
                            waitbar(incr/Total, wb, ['Generating PLC code: ' sm.modelTag]);

                            sm.generate_plc_code(obj.folders.repo, obj.settings, obj.host);
                            incr = incr + 1;
                            waitbar(incr/Total, wb, ['Generating PLC code: ' sm.modelTag]);
                        end

                    else
                        warning(['Cannot process ' sm.modeltag '. SimuLink model not fully defined.'])
                    end

                end
            end
            delete(wb)
        end

        function obj = generate_plccode_itfc(obj)

            obj.check();
            Total = sum(obj.settings.Flags_Generate_itfc);

            wb = waitbar(0, ['Generating PLC code: ']);

            if obj.settings.FlagGenerate_PLCApps

                fn = fieldnames(obj.itfc);

                if isempty(obj.itfc) % if no itfc exists, return
                    return
                end

                incr = 0;
                for isl = 1 : length(fn)

                    sm = obj.itfc.(fn{isl});

                        if  sm.Flag_Generate_PLC_app && sm.Flags.PLC_codeGen_Ready
                            waitbar(incr/Total, wb, ['Generating PLC code: ' sm.appTag]);

                            sm.generate_plc_code(obj.folders.repo, obj.settings);
                            incr = incr + 1;
                            waitbar(incr/Total, wb, ['Generating PLC code: ' sm.appTag]);
                        end  

                end
            end

            delete(wb)

        end

            function obj = generate_plccode_host(obj)

            obj.check();
            Total = sum(obj.settings.Flags_Generate_itfc);

            wb = waitbar(0, ['Generating PLC code: ']);

            if obj.settings.FlagGenerate_PLCApps

                fn = fieldnames(obj.host);

                if isempty(obj.host) % if no host exists, return
                    return
                end

                incr = 0;
                for isl = 1 : length(fn)

                    sm = obj.host.(fn{isl});

                        if  sm.Flag_Generate_PLC_app && sm.Flags.PLC_codeGen_Ready
                            waitbar(incr/Total, wb, ['Generating PLC code: ' sm.appTag]);

                            sm.generate_plc_code(obj.folders.repo, obj.settings, obj.itfc);
                            incr = incr + 1;
                            waitbar(incr/Total, wb, ['Generating PLC code: ' sm.appTag]);
                        end  

                end
            end
            delete(wb)

        end

        function obj = check(obj)

            % submodels
            fn = fieldnames(obj.sub_models);
            Flags_Generate_sm = false(length(fn),1);

            for isl = 1 : length(fn)
                Flags_Generate_sm(isl,1) = obj.sub_models.(fn{isl}).Flags.PLC_codeGen_Ready && obj.sub_models.(fn{isl}).Flag_Generate_PLC_app ;
            end

            obj.settings.Flags_Generate_sm = Flags_Generate_sm;

            % interface
            
            if ~isempty(obj.itfc)
                fn = fieldnames(obj.itfc);
                Flags_Generate_itfc = false(length(fn),1);

                for iitfc = 1 : length(fn)
                    Flags_Generate_itfc(iitfc,1) = obj.itfc.(fn{iitfc}).Flags.PLC_codeGen_Ready && obj.itfc.(fn{iitfc}).Flag_Generate_PLC_app ;
                end

                obj.settings.Flags_Generate_itfc = Flags_Generate_itfc;
            else
                obj.settings.Flags_Generate_itfc = [];
            end
            % host
            if ~isempty(obj.host)
                fn = fieldnames(obj.host);
                Flags_Generate_host = false(length(fn),1);

                for ihost = 1 : length(fn)
                    Flags_Generate_host(ihost,1) = obj.host.(fn{ihost}).Flags.PLC_codeGen_Ready && obj.host.(fn{ihost}).Flag_Generate_PLC_app ;
                end

                obj.settings.Flags_Generate_host = Flags_Generate_host;
            else
                obj.settings.Flags_Generate_host = [];
            end
        end

        function obj = save_to_file(obj, entriesSM, entriesITFC, entriesHOST)

            % write submodels
            if isempty(obj.sub_models)
                fn = [];
            else
                fn = fieldnames(obj.sub_models);
            end
            newRow = cell(length(fn), length(entriesSM));
            tab = array2table(newRow, 'VariableNames',entriesSM);

            for ism = 1 : length(fn)
                for ientry = 1 : length(entriesSM)
                    cc = obj.sub_models.(fn{ism}).(entriesSM{ientry});
                    if strcmp(cc,string)
                        cc = [];
                    end
                    tab.(entriesSM{ientry}){ism} =  cc;
                end
            end

            writetable(tab, obj.settings.input_file,'Sheet','Submodels')

            % write itfc

            if isempty(obj.itfc)
                fn = [];
            else
                fn = fieldnames(obj.itfc);
            end
            newRow = cell(length(fn), length(entriesITFC));
            tab = array2table(newRow, 'VariableNames',entriesITFC);

            for ism = 1 : length(fn)
                for ientry = 1 : length(entriesITFC)
                    cc = obj.itfc.(fn{ism}).(entriesITFC{ientry});
                    if strcmp(cc,string)
                        cc = [];
                    end
                    tab.(entriesITFC{ientry}){ism} =  cc;
                end
            end

            writetable(tab, obj.settings.input_file,'Sheet','ITFC')

            % write host

            if isempty(obj.host)
                fn = [];
            else
                fn = fieldnames(obj.host);
            end
            newRow = cell(length(fn), length(entriesHOST));
            tab = array2table(newRow, 'VariableNames',entriesHOST);

            for ism = 1 : length(fn)
                for ientry = 1 : length(entriesHOST)
                    cc = obj.host.(fn{ism}).(entriesHOST{ientry});
                    if strcmp(cc,string)
                        cc = [];
                    end
                    tab.(entriesHOST{ientry}){ism} =  cc;
                end
            end

            writetable(tab, obj.settings.input_file,'Sheet','HOST')

        end

        function obj = load_data_types_dict(obj)
            fold = obj.folders.repo.InterfaceFiles;
            fn = obj.settings.DataTypes_dict_filename;

            dict = plc_dictionary(fold, fn);
            obj.settings.plcdict = dict;
        end

        function obj = modbus_evaluate(obj)

              
            % Here i gather all variables that need to be converted into a
            % modbus list

                sm = fieldnames(obj.itfc); 
                itfcvar = [];
             if ~isempty(sm)
                 for ientry = 1 : length(sm)

                     itfcvar = [itfcvar; obj.itfc.(sm{ientry}).Variables];

                 end
             end

             sm = fieldnames(obj.host); 
             hostvar = [];
             if ~isempty(sm)
                 for ientry = 1 : length(sm)

                     hostvar = [hostvar; obj.host.(sm{ientry}).Variables];

                 end
             end

            %% 20230502 CRS: I still need to add the submodels
%             mcd = modbus_com_dict(itfcvar, hostvar, obj.settings.plcdict);

%             obj.settings.modbusdict = mcd;

        end

        function obj = read_test_outputs(obj)
        % This function allows reading the HOST outputs copied from the PLC

            [file,path] = uigetfile('*.dat' ,'MultiSelect','on');
                 
            if strcmpi(class(file),'double')
                return
            end



            output_files = fullfile(path, file);
            
            % i want to read the host files
            happlist = fieldnames(obj.host);
            for iha = 1 : length(happlist)
                ha = obj.host.(happlist{iha});
                ha.read_outputs(output_files);
            end

        end

    end
end



