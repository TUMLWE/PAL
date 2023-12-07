function [out, info ]= initialize_var_and_sa_v3(var, bachmanndict, info)

out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end


InternalInterfaceVarName = info.InternalInterfaceVarName;

parent = var.parent;

if isempty(parent.SV) % we have an array

    flag = class_existsinlist(info.list_parentvar, parent);
    if ~flag
        info.list_parentvar = [info.list_parentvar; parent];
        [str, parentinfo] = itfc_define_declare_svi_inputs(parent, bachmanndict, 1);
        out = [out str];

        info.parentinfo_InternalInterfaceVarName = parentinfo.InternalInterfaceVarName;
        str = ['MLOCAL SVI_ADDR SA_' parentinfo.InternalInterfaceVarName ';' newline ...
            ];
        out = [out str];

    end
else % we have a struct, then we only need to add the SVIADDR

    str = ['MLOCAL SVI_ADDR SA_' InternalInterfaceVarName ';' newline ...
        ];
    out = [out str];

end




end
