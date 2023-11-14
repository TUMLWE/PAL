function [out, var_info] = get_parent_sa_addres_var_v2(var, bachmanndict, var_info, exchange_app)



out = [];

VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end

InternalInterfaceVarName = var_info.InternalInterfaceVarName;

parent = var.parent;


if isempty(parent.SV) % we have an array, so we just add the array

    flag = class_existsinlist(var_info.get_sa_addr_parentlist, parent);
    if ~flag
        var_info.get_sa_addr_parentlist = [var_info.get_sa_addr_parentlist; parent];

        String = ['if (svi_GetAddr(pSviLib_' exchange_app.(var.parent_App).refC_name ', "' parent.VarName  '", &SA_' ...
       var_info.parentinfo_InternalInterfaceVarName ', &SviFormat) != SVI_E_OK)' newline ...
       '{' newline ...
       '    LOG_W(0, Func, "Could not get address of value ' var_info.parentinfo_InternalInterfaceVarName '!");' newline ...
       '    return (ERROR);' newline ...
       '}' newline ...
        ];

        out = [out String];

    end


else  % we have a struct

      String = ['if (svi_GetAddr(pSviLib_' exchange_app.(var.parent_App).refC_name ', "' parent.VarName '.'  char(var.parent_SubVar) '", &SA_' ...
       InternalInterfaceVarName ', &SviFormat) != SVI_E_OK)' newline ...
       '{' newline ...
       '    LOG_W(0, Func, "Could not get address of value ' parent.VarName '.' char(var.parent_SubVar) '!");' newline ...
       '    return (ERROR);' newline ...
       '}' newline ...
        ];
    
    out = [out String];


end





end
