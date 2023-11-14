function [out_set, var_info] = set_variables_sviclt_v2(var, bachmanndict, var_info, exchange_app)

out_set = [];

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

% if strcmpi(var.IO, {'input'}) % if the variable is not an output it is not important to reset it
%     return
% end

if ~strcmpi(var.Action, 'write') % if we don't have to write this variable, we exit
    return
end


InternalInterfaceVarName = var_info.InternalInterfaceVarName;

parent = var.parent;


if isempty(parent.SV) % we have an array

    flag = class_existsinlist(var_info.assign_sa_addr_parentlist_write, parent);
    if ~flag
        var_info.assign_sa_addr_parentlist_write = [var_info.assign_sa_addr_parentlist_write; parent];

        % % % % %         size_varname = [var_info.parentinfo_InternalInterfaceVarName '_size'];
        % % % % %         S1 = ['UINT32 ' size_varname ' = sizeof(' var_info.parentinfo_InternalInterfaceVarName ');' newline ...
        % % % % %             ];
        % % % % %
        % % % % %         out_size = [out_size S1];

        %         S2 = ['ret = svi_GetBlk(pSviLib_' var.parent_App ', SA_' var_info.parentinfo_InternalInterfaceVarName ...
        %             ', (UINT32*) &' var_info.parentinfo_InternalInterfaceVarName ...
        %             ',  &' size_varname ');' newline ...
        %             'if (ret != SVI_E_OK)' newline ...
        %             '   LOG_W(0, "SviClt_Read", "Could not read value ' var_info.parentinfo_InternalInterfaceVarName '!");' newline];
        switch var.VarType

            case 'double'
                if var.VarSize == 1

                    S2 = ['ret = svi_SetBlk(pSviLib_' exchange_app.(var.parent_App).refC_name ', SA_' var_info.parentinfo_InternalInterfaceVarName ...
                        ', (UINT32*) &' var_info.parentinfo_InternalInterfaceVarName ', sizeof(' var_info.parentinfo_InternalInterfaceVarName ') );' newline ...
                        'if (ret != SVI_E_OK)' newline ...
                        '     LOG_W(0, "SviClt_Write", "Could not read value ' var_info.parentinfo_InternalInterfaceVarName '!");' newline ...
                        newline];
                else
                    error('writing not yet implemented for this data type')
                end

            case {'uint16' 'logical'}

                if var.VarSize == 1
                    S2 = ['ret = svi_SetVal(pSviLib_' exchange_app.(var.parent_App).refC_name ', SA_' var_info.parentinfo_InternalInterfaceVarName ...
                        ', *(UINT32*) &' var_info.parentinfo_InternalInterfaceVarName ' );' newline ...
                        'if (ret != SVI_E_OK)' newline ...
                        '     LOG_W(0, "SviClt_Write", "Could not read value ' var_info.parentinfo_InternalInterfaceVarName '!");' newline ...
                        newline];

                else
                    error('writing not yet implemented for this data type')
                end

            otherwise
                error('writing not yet implemented for this data type')
        end



        out_set = [out_set S2];

    end

else  % we have a struct


%%%%%%%%%%%%% CRS 20231016 - Added possibility of writing into structures
       switch var.VarType

            case 'double'
                if var.VarSize == 1

                    S2 = ['ret = svi_SetBlk(pSviLib_' exchange_app.(var.parent_App).refC_name ', SA_' InternalInterfaceVarName ...
                        ', (UINT32*) &' InternalInterfaceVarName ', sizeof(' InternalInterfaceVarName ') );' newline ...
                        'if (ret != SVI_E_OK)' newline ...
                        '     LOG_W(0, "SviClt_Write", "Could not read value ' InternalInterfaceVarName '!");' newline ...
                        newline];
                else
                    error('writing not yet implemented for this data size')
                end

            case 'uint16'

                if var.VarSize == 1
                    S2 = ['ret = svi_SetVal(pSviLib_' exchange_app.(var.parent_App).refC_name ', SA_' InternalInterfaceVarName ...
                        ', *(UINT32*) &' InternalInterfaceVarName ' );' newline ...
                        'if (ret != SVI_E_OK)' newline ...
                        '     LOG_W(0, "SviClt_Write", "Could not read value ' InternalInterfaceVarName '!");' newline ...
                        newline];

                else
                    error('writing not yet implemented for this data type')
                end

            otherwise
                error('writing not yet implemented for this data type')
        end



        out_set = [out_set S2];
%%%%%%%%%%%%%%%%

end






end
