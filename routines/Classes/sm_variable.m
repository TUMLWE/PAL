classdef sm_variable < handle

    properties
        modelTag {mustBeText} = strings
        TagName {mustBeText} = strings
        VarName {mustBeText} = strings
        IO {mustBeText} = strings
        PortName {mustBeText} = strings
        PortNumber {mustBeNumeric}
        VarType{mustBeText} = strings
        Create = false
        parent_App {mustBeText} = strings
        parent_TagName {mustBeText} = strings
        parent = []
        Action {mustBeText} = strings
        VarSize = 1 % 20230504 CRS: the code so far supports only model variables of size 1 for submodels, thus no arrays
        Flag_Assign_InitialValue = false
        Initial_Value
        parent_SubVar = 1; % 20230504 CRS: the code so far supports only reading from hosts without subvar

    end

    methods

        function obj = sm_variable(input_table, host, itfc )

            obj.addEntry(input_table);

            % check association with itfc/host variable, if necessary


            if ~isempty(obj.parent_App)

                appnames = [fieldnames(itfc); fieldnames(host)];
                idxa = find(strcmp(appnames, obj.parent_App));

                if length(idxa) ~= 1
                    error('submodel %s, variable %s required info about app %s, but it could not be found or it is found multiple times',obj.modelTag, obj.TagName, obj.parent_App)
                end

                if sum(strcmpi(fieldnames(itfc), obj.parent_App)) > 0
                    parent_app = itfc.(obj.parent_App);                
                elseif sum(strcmpi(fieldnames(host), obj.parent_App)) > 0
                    parent_app = host.(obj.parent_App);
                else
                    error('submodel %s, variable %s required info about app %s, but it could not be associated',obj.modelTag, obj.TagName, obj.parent_App)
                end

                parent = parent_app.Variables( cell2mat(cellfun(@(x) strcmp(x,obj.parent_TagName), {parent_app.Variables.TagName}, 'UniformOutput',false)));

                if isempty(parent)
                    error('Could not connect variable %s of submodel app %s to its parent: could not find variable %s', ...
                        obj.TagName, obj.modelTag, obj.parent_TagName )
                elseif length(parent)>1
                    error('Could not connect variable %s of submodel app %s to its parent: multiple definition of variable %s', ...
                        obj.TagName, obj.modelTag, obj.parent_TagName )
                end

                obj.parent = parent;

            end


        end

        function obj = addEntry(obj, input_table)

            errorFlag = false;
            for i = 1 : length(input_table.Properties.VariableNames)
                vn = input_table.Properties.VariableNames{i};
                value = input_table.(vn);

                switch vn

                    case {'modelTag' 'TagName', 'VarName', 'IO', 'PortName', 'VarType', ...
                            'parent_App'	'parent_TagName', 'Action'}
                        value = recastToType(vn, value,'string');
                        obj.(vn) = value;

                    case {'Create', 'Flag_Assign_InitialValue'}

                        if isnan(value)
                            errorFlag = true;
                            errormsg = ['field "' vn '" cannot be NaN'];
                            continue
                        end

                        value = recastToType(vn, value,'logical');
                        obj.(vn) = value;

                    case {'PortNumber', 'Initial_Value'}
%                         if isnan(value) && strcmpi('PortNumber',vn)
%                             errorFlag = true;
%                             errormsg = ['field "' vn '" cannot be NaN'];
%                             continue
%                         end

                        value = recastToType(vn, value,'double');
                        obj.(vn) = value;

                        %                     case {}
                        %                         if all(isstrprop(value,"digit"))
                        %                             value = recastToType(vn, value,'double');
                        %                         else
                        %                             value = recastToType(vn, value,'string');
                        %                         end
                        %                         obj.(vn) = value;

                    otherwise
                        warning('Table field %s not implemented in sm_variable class', vn)

                end


            end


            if strcmpi(obj.TagName,strings)
                disp(input_table)
                error('Error when reading the above variable. TagName undefined.')
            end

            if errorFlag
                error('Error when reading variable "%s": %s.', obj.TagName, errormsg)
            end

        end



    end

end

