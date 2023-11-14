classdef host_variable < handle

    properties
        TagName {mustBeText} = strings
        VarName {mustBeText} = strings
        AppName {mustBeText} = strings
        %         VarSize {mustBeNumeric} = []
        VarType {mustBeText} = strings
        %         Access {mustBeText} = strings
        VarSize {mustBeNumeric}
        Create = false
        parent_App {mustBeText} = strings
        parent_TagName {mustBeText} = strings
        parent_SubVar
%         Connect_to_sm = false
        Action {mustBeText} = strings
%         sm_AppName {mustBeText} = strings
%         sm_VarName {mustBeText} = strings
        Flag_Assign_InitialValue = false
        Initial_Value
        Units {mustBeText} = strings
        Print_output = false
        output_freq {mustBeText} = strings
        Comments {mustBeText} = strings
        parent = []
        modbus_test = false
        time_hist
        SV = [];
        test_outputs
    end

    methods

        function obj = host_variable(input_table, itfc)

            obj.addEntry(input_table);

            % check association with itfc variable, if necessary
            if ~isempty(obj.parent_App)

                if isempty(itfc.(obj.parent_App))
                    error('Could not connect variable %s of HOST app %s to its parent: could not find parent_App %s', ...
                      obj.TagName, obj.AppName, obj.parent_App)                    
                end

                itfc_app = itfc.(obj.parent_App);

                itfc_var = itfc_app.Variables( cell2mat(cellfun(@(x) strcmp(x,obj.parent_TagName), {itfc_app.Variables.TagName}, 'UniformOutput',false)));

                 if isempty(itfc_var) 
                    error('Could not connect variable %s of HOST app %s to its ITFC: could not find variable %s', ...
                      obj.TagName, obj.AppName, obj.parent_TagName )
                 elseif length(itfc_var)>1
                    error('Could not connect variable %s of HOST app %s to its ITFC: multiple definition of variable %s', ...
                      obj.TagName, obj.AppName, obj.parent_TagName )

                 end


                 switch class(obj.parent_SubVar)

                     case 'double'

                         obj.VarSize = 1;

                        if strcmpi(obj.VarType,'inherit')
                            obj.VarType = itfc_var.VarType;
                        end

                     case 'string'
                         
                        outsubvar = find_var_from_fieldname(itfc_var.SV, 'TagName', obj.parent_SubVar );
                        obj.VarSize = outsubvar.VarSize;
                         
                        if strcmpi(obj.VarType,'inherit')
                            obj.VarType = outsubvar.VarType;
                        end

                     otherwise

                        error(['Problem when connecting variable %s of HOST app %s to its ITFC: subvariable should be read from parent %s' ...
                        ', but data class of variable parent_SubVar could not be resolved.'], ...
                            obj.TagName, obj.AppName, obj.parent_TagName )                       

                 end
                
                 obj.parent = itfc_var;



                 % find VarSize
                 if obj.VarSize == -1
                    warning(['Problem when connecting variable %s of HOST app %s to its ITFC: subvariable should be read from parent %s' ...
                        ', but size of variable is not inherited. This can lead to issues if the two sizes differ'], ...
                      obj.TagName, obj.AppName, obj.parent_TagName )
                 end
                 
            end


            %% There must be a part of connection between submodel variables and host, but possibly i can do it in the submodels
%             if ~isempty(obj.Connect_to_sm)
% 
%                 if ~isempty(obj.sm_AppName)
%                     error('Could not connect variable %s of HOST app %s to its submodel: could not find sm_AppName.', ...
%                         obj.TagName, obj.AppName )
%                 end
%                 if ~isempty(obj.sm_VarName)
%                     error('Could not connect variable %s of HOST app %s to its submodel variable: could not find sm_VarName.', ...
%                         obj.TagName, obj.AppName )
%                 end
% 
%                 obj
% 
%             else
% 
%             end

        end

        function obj = addEntry(obj, input_table)

            for i = 1 : length(input_table.Properties.VariableNames)
                vn = input_table.Properties.VariableNames{i};
                value = input_table.(vn);

                switch vn

                    case {'TagName', 'VarName', 'AppName', 'VarType','parent_App','parent_TagName',...
                            'Units', 'Comments', 'output_freq', 'sm_AppName', 'sm_VarName', 'Action'}
                    value = recastToType(vn, value,'string');
                    obj.(vn) = value;

                    case {'Create', 'Connect_to_sm', 'Print_output','Flag_Assign_InitialValue' 'modbus_test'}
                    value = recastToType(vn, value,'logical');
                    obj.(vn) = value;

                    case {'Initial_Value', 'VarSize'}
                    value = recastToType(vn, value,'double');
                    obj.(vn) = value;

                    case {'parent_SubVar'}
                        if all(isstrprop(value,"digit"))
                            value = recastToType(vn, value,'double');                            
                        else 
                            value = recastToType(vn, value,'string');                            
                        end
                    obj.(vn) = value;

                    otherwise
                    warning('Table field %s not implemented in host_variable class', vn)

                end

            end
            %% check that at least VarName, AppName and VarType aren't empty
            if strcmp(obj.VarName,strings)
                error('Error when reading HOST variable "%s": no VarName found',obj.TagName);
            end

            if strcmp(obj.VarType,strings)
                error('Error when reading ITFC variable "%s": no VarType found',obj.TagName);
            end




        end

        function out_th = find_parent_th(obj)

            pp = obj.parent;

            if strcmpi(class(obj.parent_SubVar),'string') % if it is a struct


             out_th = pp.time_hist.(obj.parent_SubVar);
            

            elseif strcmpi(class(obj.parent_SubVar),'double') % if it is an array

             out_th = pp.time_hist(:, obj.parent_SubVar);

            end

       
        end

        function delay_samples = find_delay_test(obj, sample_time)

            yh = obj.test_outputs.hostvar_th;
%             xh = obj.test_outputs.hostvar_timestamp;

            xh = 1 : length(yh);

            if ~strcmpi(obj.Action,'read')
                delay_samples = 0;
                return
            end

            yi = obj.test_outputs.itfc_var_th;

            nElem = size(yi,1);
            xi = 1:nElem;
            
%             xi = [seconds([0 : nElem-1]*sample_time) + xh(1)]';

            % identification of good chunk
            delay_samples = finddelay(yh - mean(yh), yi - mean(yi)) + 1;
            

% %             xi_v2 = xi - seconds(delay_samples*sample_time);
%             xi_v2 = xi - delay_samples;
% 
%             xi_begin = find(xi_v2 > xh(1), 1, 'first');
% 
%             figure(); hold on
%             plot(xh, yh)
%             plot(xi_v2, yi)
% 
%             xi_v3 = xi_v2(xi_begin:end);
%             yi_v3 = yi(xi_begin:end);



            %
%             sample_windowing = 100;
%             d2 = []
%             for iElem = 1 : length(xi_v3) - sample_windowing
% 
%                
%                 yi_chunk = yi_v3(iElem:iElem + sample_windowing); 
%                 yh_chunk = yh(iElem:iElem + sample_windowing); 
%     
%                 d2(iElem) = finddelay(yh_chunk - mean(yh_chunk), yi_chunk - mean(yi_chunk)) + 1;
% 
%                 figure()
%                 hold on
%                 plot(xh, yh)
% %                 plot(xi_v3 - seconds(d2(iElem)*sample_time), yi_v3)
%                 plot(xi_v3 - d2(iElem) , yi_v3)
%                 xline(xi_v3(iElem))
%                 xline(xi_v3(iElem + sample_windowing))
% 
%                 pause
%                 close all
% 
% 
% %                 if find(yi_v3(iElem) == yh)
% % 
% % 
% %                 end
% 
%             end


            

        end

    end

end
