function [fast, slow, ctrl] = print_out_to_file(var, info)

fast = [];
slow = [];
ctrl = [];

if ~var.Create % if the variable should not be created
    return
end

if ~var.Print_output % if the variable should not be printed to output
    return
end


switch var.VarType 

    case 'char'
        String = ['fprintf(fp, "%s; ", ' info.InternalInterfaceVarName ');  ' newline];
 
    case {'double', 'single', 'int16', 'logical', 'uint16'}

        switch var.VarType 

            case 'double'
                printchar = '%llf';
            case 'single'
                printchar = '%f';
            case 'int16'
                printchar = '%d';
            case 'logical'
                printchar = '%d';
            case 'uint16'
                printchar = '%d';

            otherwise 
            warning('Data type not yet implemented, please check.')
            keyboard

        end

    String = [newline ...
        'if (' info.InternalInterfaceVarName '==' info.InternalInterfaceVarName ')' newline ...
    '{' newline ...
    '     fprintf(fp, "' printchar '; ", ' info.InternalInterfaceVarName ');' newline ...
    '}' newline ...
    'else' newline ...
    '{' newline ...
    '     fprintf(fp, "%s; ", "NaN");' newline ...
    '}' newline  ...
    ]; 

    otherwise
            warning('Data type not yet implemented, please check.')
            keyboard
end



switch var.output_freq

    case 'fast'
        fast = String;

    case 'slow'
        slow = String;

    case 'ctrl'
        ctrl = String;

end