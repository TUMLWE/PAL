function [out, info] = SVI_var_definition(var,bachmanndict, info)


out = strings;


%     idx = idxVarToInclude(iVar);
VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

InternalInterfaceVarName = info.InternalInterfaceVarName;

switch var.VarType


    case 'struct'

        error('host and submodel variables cannot be of struct type');
       
    case {'double' 'int16' 'logical' 'uint16' 'single'}

        if var.Flag_Assign_InitialValue
            init_val = var.Initial_Value;       
        else
            init_val = 0;
        end

        PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        out = ['MLOCAL ' PLCType ' ' InternalInterfaceVarName '=' num2str(init_val) ';' newline] ;

    case 'char'

        if var.Flag_Assign_InitialValue
            error('initial value for char variable not yet implemented. Please check %s', var.TagName);
        end

        PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        outvar = find_var_from_fieldname(var.parent.SV, 'TagName', var.parent_SubVar );
        if isempty(outvar)
            error(['cannot define host variable %s. this happens because so far, char variables can only' ...
                ' be read from an interface. If this is how the variable is defined, ' ... 
                'make sure that the parent variable is correctly defined'], var.TagName);

        end

        sz = outvar.VarSize;
        out = ['MLOCAL ' PLCType ' ' InternalInterfaceVarName '[' num2str(sz) '];' newline] ;

   
    otherwise

        fprintf('Variable %s type is unknown\n', VarName)
        keyboard

end


end



