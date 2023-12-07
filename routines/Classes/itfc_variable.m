classdef itfc_variable < handle

    properties
        TagName {mustBeText} = strings
        VarName {mustBeText} = strings
        AppName {mustBeText} = strings
        VarSize {mustBeNumeric} = []
        VarType {mustBeText} = strings
        Access {mustBeText} = strings
        Create = false
        SV = {}
        time_hist
        modbus_test = false
    end

    methods

        function obj = itfc_variable(input_table)

            if length(input_table.InputNumber) == 1 % if the table is only 1 elem length, it is not a struct

                switch input_table.VarType{1}

                    case 'struct'

                        error('Variable %s not properly specified: data type is a string, but could not find any associated subfield.', input_table.VarType{1});

                    otherwise

                        obj.addEntry(input_table);

                end


            else % assignment is done through recursion

                main_idx = strcmp(input_table.VarType,'struct');
                if sum(main_idx)~=1
                    error('Multiple entries defined for InputNumber %s, but the variable is not a struct', num2str(input_table.InputNumber));
                end

                for iElem = 1 : length(input_table.TagName)

                    if strcmp(input_table.VarType{iElem},'struct')

                        obj.addEntry(input_table(iElem,:)); % if it is a main variable

                    else

                        sv = itfc_variable(input_table(iElem,:));
                        obj.SV{end+1,1} = sv;

                    end

                end

            end



        end

        function obj = addEntry(obj, input_table)

            for i = 1 : length(input_table.Properties.VariableNames)
                vn = input_table.Properties.VariableNames{i};
                value = input_table.(vn);

                if strcmp(vn,'TagName') || strcmp(vn,'VarName') || strcmp(vn,'AppName') || ...
                        strcmp(vn,'VarType') || strcmp(vn,'Access')
                    value = recastToType(vn, value,'string');
                    obj.(vn) = value;
                end

                if strcmp(vn,'VarSize')
                    value = recastToType(vn, value,'double');
                    obj.(vn) = value;
                end
                if strcmp(vn,'Create') || strcmp(vn,'modbus_test')
                    value = recastToType(vn, value,'logical');
                    obj.(vn) = value;
                end

            end

            %% check that at least VarName, AppName and VarType aren't empty
            if strcmp(obj.VarName,strings)
                error('Error when reading ITFC variable "%s": no VarName found',obj.TagName);
            end

%             if strcmp(obj.AppName,strings)
%                 error('Error when reading ITFC variable "%s": no AppName found',obj.TagName);
%             end

            if strcmp(obj.VarType,strings)
                error('Error when reading ITFC variable "%s": no VarType found',obj.TagName);
            end
        end

        function [obj, out] = initialize_empty(obj, nTimeInst)

            out = [];
            type = obj.VarType;

            switch type

                case 'struct'

                    for isv = 1 : length(obj.SV)

                        subvar = obj.SV{isv};
                        if ~subvar.Create % if the variable should not be created
                            continue
                        end

                        sv_type = subvar.VarType;
                        sv_size = subvar.VarSize;

                        switch sv_type

                            case 'char'
                                baseChar(1:sv_size) = 'a';
                                temp = repmat(baseChar, [nTimeInst 1]) ;
                                out.(subvar.TagName) = recastToType(subvar.TagName, temp, sv_type);

                            case {'double', 'int16' }

                                temp = zeros(nTimeInst, sv_size);
                                out.(subvar.TagName) = recastToType(subvar.TagName, temp, sv_type);

                            case 'logical'
                                temp = false(nTimeInst, sv_size);
                                out.(subvar.TagName) = temp;

                            otherwise

                                error('Data type not yet implemented in struct');

                        end
                        obj.SV{isv}.time_hist = out.(subvar.TagName);

                    end

                case 'double'

                    size = obj.VarSize;
                    temp = zeros(nTimeInst, size);

                    out = recastToType(obj.TagName, temp, type);
                    obj.time_hist = out;

                otherwise

                    error('Data type not yet implemented');

            end

        end

        function [obj, out] = initialize_random(obj, nTimeInst)

            out = [];
            type = obj.VarType;

            if strcmpi(obj.Access, 'READ') 

            switch type

                case 'struct'

                    for isv = 1 : length(obj.SV)

                        subvar = obj.SV{isv};
                        if ~subvar.Create % if the variable should not be created
                            continue
                        end

                        sv_type = subvar.VarType;
                        sv_size = subvar.VarSize;

                        switch sv_type

                            case 'char'

                                s=['A':'Z', 'a':'z','0':'9'];
                                temp=s(randi(numel(s),nTimeInst,sv_size-1)); % the -1 is needed because you need also to store the terminator character
                                out.(subvar.TagName) = recastToType(subvar.TagName, temp, sv_type);

                            case 'single'

                                temp = rand(nTimeInst, sv_size);
                                out.(subvar.TagName) = recastToType(subvar.TagName, temp, sv_type);


                            case 'double'

                                temp = rand(nTimeInst, sv_size);
                                out.(subvar.TagName) = recastToType(subvar.TagName, temp, sv_type);

                            case 'int16'

                                temp = randi(9999,nTimeInst, sv_size);
                                out.(subvar.TagName) = recastToType(subvar.TagName, temp, sv_type);


                            case 'logical'
                                temp = logical(randi(2,nTimeInst, sv_size)-1);
                                out.(subvar.TagName) = temp;

                            otherwise

                                error('Data type not yet implemented in struct');

                        end
                        obj.SV{isv}.time_hist = out.(subvar.TagName);

                    end

                case 'double'

                    size = obj.VarSize;
                    temp = rand(nTimeInst, size);

                    out = recastToType(obj.TagName, temp, type);
                    obj.time_hist = out;

                otherwise

                    error('Data type not yet implemented');

            end

            elseif strcmpi(obj.Access, 'WRITE')

                [obj, out] = obj.initialize_empty(nTimeInst);

            end


        end

    end
end