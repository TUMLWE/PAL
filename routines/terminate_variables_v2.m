function out = terminate_variables_v2(var, info)


out = [];


%     idx = idxVarToInclude(iVar);
VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if ~strcmp(var.IO, 'output') % if the variable is not an output it is not important to reset it
    return
end


InternalInterfaceVarName = info.InternalInterfaceVarName;

switch var.VarType


    case 'struct'

        error('host and submodel variables cannot be of struct type');
       
    case {'double' 'int16' 'logical' 'uint16'}

        if var.Flag_Assign_InitialValue
            init_val = var.Initial_Value;       
        else
            init_val = 0;
        end

        out = [InternalInterfaceVarName '=' num2str(init_val) ';' newline] ;

    case 'char'

            error('termination value for char variable not yet implemented. Please check %s', var.TagName);

    otherwise

        fprintf('Variable %s type is unknown\n', VarName)
        keyboard

end

end

