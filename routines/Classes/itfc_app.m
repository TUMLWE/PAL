classdef itfc_app < handle
    
    properties
        
        appTag {mustBeText} = strings
        PLCApp_folder {mustBeText} = strings
        refC_name {mustBeText} = strings
        Flag_Generate_PLC_app {mustBeNumericOrLogical} = false
        Flag_Create_test_ITFC {mustBeNumericOrLogical} = false
        test_ITFC_filename {mustBeText} = strings
        test_ITFC 
        Comment {mustBeText} = strings
        Variables
        Flags
       
    end
    
    methods

        function obj = itfc_app(inputs)

            for i = 1 : length(inputs.Properties.VariableNames)
                vn = inputs.Properties.VariableNames{i};
                value = inputs.(vn);

                if strcmp(vn,'appTag') || strcmp(vn,'PLCApp_folder') || strcmp(vn,'refC_name') || ...
                        strcmp(vn,'test_ITFC_filename') || strcmp(vn,'Comment')
                    value = recastToType(vn, value,'string');
                    obj.(vn) = value;                    
                end 

                if strcmp(vn,'Flag_Generate_PLC_app') || strcmp(vn,'Flag_Create_test_ITFC')
                    value = recastToType(vn, value,'logical');
                    obj.(vn) = value;                    
                    
                end


            end

            if obj.Flag_Create_test_ITFC

                itfc_found = 0;
                if exist(obj.test_ITFC_filename) == 2 
                    itfc_found = 1;
                elseif exist([obj.test_ITFC_filename '.mat']) == 2
                    itfc_found = 1;
                    obj.test_ITFC_filename = [obj.test_ITFC_filename '.mat'];
                end

                if itfc_found

                    load(obj.test_ITFC_filename);
                    obj.test_ITFC = test_ITFC;
                else
                    error('ITFC_file not found. Please check filepath.')
                end

            end

        end

        function obj = load_itfc_file(obj)
            
            if ~obj.Flag_Create_test_ITFC
                warning('Cannot load ITFC file because Flag_Create_test_ITFC is set to false, please change it.')                
            end

            
            if exist(obj.test_ITFC_filename) == 2
                    load(obj.test_ITFC_filename);
                    obj.test_ITFC = test_ITFC;
                    obj.assign_test_ITFC()
             else
                    error('ITFC_file not found. Please check filepath.')
            end
        end

        function obj = assign_test_ITFC(obj)

            if ~isempty(obj.test_ITFC)

                for ivar = 1 : length(obj.Variables)
                    var = obj.Variables(ivar);
                    if ~isfield(obj.test_ITFC,var.TagName)
                        error('Variable %s is not present in loaded ITFC', var.TagName)
                    end
                    var.time_hist = obj.test_ITFC.(var.TagName);

                    switch var.VarType

                        case 'struct'

                            for isv = 1 : length(var.SV)
                                subvar = var.SV{isv};
                                subvar.time_hist = var.time_hist.(subvar.TagName);
                            end

                        otherwise
                            
                            obj.Variables(ivar) = var;

                    end

                end

            end

        end

        function obj = load_svi_variables(obj, fw)

            SVI_fn = fw.settings.ExcelPLCFileName;
%             InterfaceFilesFolder = fw.folders.repo.InterfaceFilesFolder;

            itfc_table = readtable(SVI_fn,'Sheet','ITFC');

            IN_vector = unique(itfc_table.InputNumber);
            for iVar = 1 : length(IN_vector)

                
                row_vars = itfc_table.InputNumber == IN_vector(iVar);

                varlist_itfc(iVar,1) = itfc_variable(itfc_table(row_vars,:));

            end

            if exist("varlist_itfc","var") % 
                itfc_out = varlist_itfc(strcmp({varlist_itfc.AppName}', obj.appTag));
            else
                itfc_out = [];
            end


            obj.Variables = itfc_out;

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
%             HostName = obj.host_app;

            % create new file (20230216 - CarloSucameli: so far the code supports only Bachmann systems)
            create_matlabcodegen_fields_itfc(SourceRefPath, SourceRefFileName, TargetFolder)  
        end

        function obj = generate_plc_code(obj, repo, settings)

            if obj.Flag_Generate_PLC_app && obj.Flags.PLC_codeGen_Ready
                
                bdclose('all');
                ReferenceCFiles = repo.ReferenceCFiles;
                PLCFolder = fullfile(repo.PLCApps, obj.PLCApp_folder);
%                 InterfaceFilesFolder = repo.InterfaceFilesFolder;
% 
%                 ExcelPLCFileName = settings.ExcelPLCFileName;
                AppName = obj.refC_name;
                 
                dict = settings.plcdict;
                PLCCodeGenerator_ITFCV2(AppName, PLCFolder, ...
                    ReferenceCFiles, obj, dict);
                
            end

        end

        function obj = checkVariables(obj, dict)

            for iv = 1 : length(obj.Variables)

                var = obj.Variables(iv);

                if strcmp(var.VarType,'struct')

                    SV = var.SV;

                    if ~isempty(SV) % here i check that the size of the struct is equal to the sum of the subvariables
                       sizes =  cell2mat(cellfun(@(x) x.VarSize, SV, UniformOutput=false));
                       typesize = cell2mat(cellfun(@(x) dict.size_of_mat_type(x.VarType), SV, UniformOutput=false));
                       Create = cell2mat(cellfun(@(x) x.Create, SV, UniformOutput=false));

                       totsum = sum(sizes.*typesize.*Create);
                       if totsum~= var.VarSize
                            error('ITFC App "%s" - variable "%s" is a struct of size %s Bytes, but its subvariables sum up to %s. Please check the excel file.', ...
                               obj.appTag, var.TagName, num2str(var.VarSize), num2str(totsum));
                       end

                    else

                    end


                else
                    if ~isempty(var.SV)
                        error('ITFC App %s - variable %s is defined as a %s, but subfields were found', ...
                             var.VarType)
                    end
                end

            end


        end

        function obj = create_empty_itfc(obj)
            test_ITFC = [];
            nTimeInst = 100; % when i initialize an empty on, i set it with 1000 elements
            for iVar = 1 : length(obj.Variables)

                var = obj.Variables(iVar); 
                if ~var.Create % if the variable should not be created
                    continue
                end
                [var, out] = var.initialize_empty(nTimeInst);
                test_ITFC.(var.TagName) = out;
            end

            outfn = obj.test_ITFC_filename;
            save(outfn,"test_ITFC")

            obj.test_ITFC = test_ITFC;
        end

        function obj = create_random_itfc(obj, sample_time)


%             prompt = cellfun(@(x) ['Enter ' x ':'], smvn, 'UniformOutput',false);
            dlgtitle = 'ITFC';

            dims = [1];
%             definput = cellfun(@(x) num2str(sm.(x)), smvn, 'UniformOutput',false);
            answer = inputdlg('Specify duration [s]',dlgtitle,dims,{'100'});

            if isempty(answer)
                return
            end

            test_ITFC = [];
            nTimeInst = str2num(answer{1})*1/sample_time; 
            for iVar = 1 : length(obj.Variables)

                var = obj.Variables(iVar); 
                if ~var.Create % if the variable should not be created
                    continue
                end
                [var, out] = var.initialize_random(nTimeInst);
                test_ITFC.(var.TagName) = out;
            end

            outfn = obj.test_ITFC_filename;
            save(outfn,"test_ITFC")

            obj.test_ITFC = test_ITFC;


        end

    end
end