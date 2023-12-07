function out = initialize_var_and_sa_v2(Variables, bachmanndict)

out = newline;


% string_warning = ['Variable Type to be included in App not yet implemented'];

parent_App = {Variables.parent_App};
ee = cell2mat(cellfun(@(x) ~isempty(x), parent_App, 'Uniformoutput', false));

% only select variables that are read from a parent
Variables = Variables(ee);

parent_App = {Variables.parent_App};

%% arrays and structure are treated differently
list_parentvar = [];
for ivar = 1 : length(parent_App)
    var = Variables(ivar);
    if ~var.Create % if the variable should not be created
        continue
    end
   parent = var.parent;

    if isempty(parent.SV) % we have an array

        flag = class_existsinlist(list_parentvar, parent);
        if ~flag
        list_parentvar = [list_parentvar; parent];
        [str, info] = itfc_define_declare_svi_inputs(parent, bachmanndict, 1);
        out = [out str];
        
        str = ['MLOCAL SVI_ADDR SA_' info.InternalInterfaceVarName ';' newline ...
        ];
        out = [out str];

        end
    else % we have a struct, then we only need to add the SVIADDR

        str = ['MLOCAL SVI_ADDR SA_' var.VarName ';' newline ...
        ];
        out = [out str];
        
    end

end


end
