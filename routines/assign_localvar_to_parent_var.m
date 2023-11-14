function out = assign_localvar_to_parent_var(var, bachmanndict, info)

    
out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end

if strcmp(class(var),'sm_variable') && strcmpi(var.IO,'input')
    return
end

if strcmpi(var.Action,'read')
    return
end

InternalInterfaceVarName = info.InternalInterfaceVarName;

parent = var.parent;

if isempty(parent.SV) % we have an array

    switch var.VarType

        case {'double' 'logical'}

            str = [info.parentinfo_InternalInterfaceVarName  '[' num2str(var.parent_SubVar-1)  '] = ' InternalInterfaceVarName  ';' newline];

            out = [out str];

        otherwise

            warning('Data type not yet implemented, please check.')
            keyboard

    end



else
    
%             warning('Data type not yet implemented, please check.')
%             keyboard

    % CRS 
    % so far, only array variables need to be linked to a parent array,
    % since structs are directly read from the interface

end


end