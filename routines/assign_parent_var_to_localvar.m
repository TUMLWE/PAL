function out = assign_parent_var_to_localvar(var, bachmanndict, info)

    
out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end

if strcmp(class(var),'sm_variable') && strcmpi(var.IO,'output')
    return
end

if strcmpi(var.Action,'write') % this routine should be run only for read variables
    return
end


InternalInterfaceVarName = info.InternalInterfaceVarName;

parent = var.parent;

if isempty(parent.SV) % we have an array

    switch var.VarType

        case {'double' 'uint16'}

            if isempty(var.parent_SubVar)
                error(['Error when writing "' var.VarName '" in App "' var.AppName '": parent_SubVar is empty. Please specify a valid value.'])
            end

            str = [InternalInterfaceVarName ' = ' info.parentinfo_InternalInterfaceVarName  '[' num2str(var.parent_SubVar-1) '];' newline];
            out = [out str];

        otherwise

            warning('Data type not yet implemented, please check.')
            keyboard

    end



else

    % so far, only array variables need to be linked to a parent array,
    % since structs are directly read from the interface

end


end

% % 
% %     String = [];
% %     
% %     if sum(strcmp(VarNamePLCGeneral(idxArray), VarNamePLC{VPLCList(iVar)})) % This means that the variable is treated as an array
% %         
% %     if strcmp(Type{VPLCList(iVar)},'REAL32')
% %         
% %         temp = VarNamePLC{VPLCList(iVar)};
% %         temp(regexp(temp,'[.]'))=[];
% %         String = [VarNameENOHOST{VPLCList(iVar)} ' = ' temp '[' num2str(ArraySubfield{VPLCList(iVar)}-1) '];' newline];
% %         
% %     elseif strcmp(Type{VPLCList(iVar)},'STRING')
% %         String = ['strcpy(' VarNameENOHOST{VPLCList(iVar)} ',' ArraySubfield{VPLCList(iVar)} ');' newline];
% %   
% %     else
% %        fprintf('Variable Type to be included in ENOHOST not yet implemented')
% %        keyboard
% % 
% %     end
% %    
% %     end
    
    
