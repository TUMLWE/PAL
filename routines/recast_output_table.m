function out = recast_output_table(out, label, var)


idx_found = find(strcmp(out.Properties.VariableNames, label));
if length(idx_found)~=1
    warning('Cannot find the variable %s of app %s in output files, please continue at your own risk.', var.TagName, var.AppName)
    keyboard
end


type = var.VarType;
switch type

    case 'char'

    case {'double' 'int16' 'uint16'}

        if class(out.(label))

        end
        cell2mat(out.(label))

        out.(label) = cell2mat(cellfun(@(x) str2num(x),out.(label), 'UniformOutput',false ));


    otherwise
        warning('Data type not implemented, please add it');


end


end
