function [out, info ]= initialize_var_and_sa_v3(var, bachmanndict, info)

out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end


% parent_App = {Variables.parent_App};
% ee = cell2mat(cellfun(@(x) ~isempty(x), parent_App, 'Uniformoutput', false));
% 
% % only select variables that are read from a parent
% Variables = Variables(ee);
% 
% parent_App = {Variables.parent_App};
% 
% %% arrays and structure are treated differently
% list_parentvar = [];
% for ivar = 1 : length(parent_App)
%     var = Variables(ivar);
%     if ~var.Create % if the variable should not be created
%         continue
%     end

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
