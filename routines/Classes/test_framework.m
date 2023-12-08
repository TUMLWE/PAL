classdef test_framework < handle
    
    properties
        wfcfw
        hostdata
        host
    end
    
    
    methods

        function obj = test_framework()
            
                        
        end

        function outlist = get_outputs_list(obj, host, fw)

            outlist = [];
            for ivar = 1 : length(host.Variables)

                var = host.Variables(ivar);

                if ~isempty(var.test_outputs)
                    outlist = [outlist; {var.TagName}];
                end

            end

        end


        function outlist = load_host_data(obj, filename, host, fw)

            obj.hostdata = load(filename);

            % perform pairing host/itfc variables
            outlist = obj.check_app_data_compatibility(fw.host.(host), fw.settings.sample_time);

        end

        function outlist = check_app_data_compatibility(obj, ha, sample_time)

            flag_var_found = false(length(ha.Variables),1);
                
            hostdata = obj.hostdata;
            outlist = {};
            for iElem = 1 : length(flag_var_found)
            
                var = ha.Variables(iElem);

                if var.Create == 0
                    flag_var_found(iElem) = true;
                    continue
                end

                freq_var = var.output_freq;

                switch freq_var

                    case 'fast'

                        listout = hostdata.outmat_fast;

                    case 'slow'

                        listout = hostdata.outmat_slow;

                    case 'ctrl'

                        listout = hostdata.outmat_ctrl;

                end

                Exist_Column = find(strcmp([var.TagName var.Units],listout.Properties.VariableNames));

                if isempty(Exist_Column) || length(Exist_Column) >1

                    warning('Variable "%s" not found in output file. Please proceed with care', [var.TagName var.Units])
                    keyboard
                
                else

                    flag_var_found(iElem) = true;

                    var.test_outputs.hostvar_th = listout.([var.TagName var.Units]);
                    var.test_outputs.hostvar_timestamp = listout.TimeStamp_UTC;
                    
                    outlist = [outlist ; {var.TagName}];

                    if ~isempty(var.parent_SubVar) && strcmpi(var.Action, 'read')

                        var.test_outputs.itfc_var_th = var.find_parent_th();
                        nElem = size(var.test_outputs.itfc_var_th,1);
                        var.test_outputs.itfc_var_timestamp = seconds(([0 : nElem-1])*sample_time) + var.test_outputs.hostvar_timestamp(1);
                    
                    end

                   
                end


            end

            if sum(flag_var_found) ~= length(flag_var_found)
                warning('Could not find all host variables in output file!')
            end


        end
        
    end

end

