function [out_size, out_get, var_info] = assign_var_from_sa_address_var(var, bachmanndict, var_info, exchange_app)

out_size = [];
out_get = [];

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

if ~strcmpi(var.Action,'read') % if this variable does not have to be read we should exit
    return
end

InternalInterfaceVarName = var_info.InternalInterfaceVarName;

parent = var.parent;


if isempty(parent.SV) % we have an array

    flag = class_existsinlist(var_info.assign_sa_addr_parentlist, parent);
    if ~flag
        var_info.assign_sa_addr_parentlist = [var_info.assign_sa_addr_parentlist; parent];

    size_varname = [var_info.parentinfo_InternalInterfaceVarName '_size'];
    S1 = ['UINT32 ' size_varname ' = sizeof(' var_info.parentinfo_InternalInterfaceVarName ');' newline ...
        ];

    out_size = [out_size S1];
    
    S2 = ['ret = svi_GetBlk(pSviLib_' exchange_app.(var.parent_App).refC_name ', SA_' var_info.parentinfo_InternalInterfaceVarName ...
        ', (UINT32*) &' var_info.parentinfo_InternalInterfaceVarName ...
        ',  &' size_varname ');' newline ...
        'if (ret != SVI_E_OK)' newline ...
        '   LOG_W(0, "SviClt_Read", "Could not read value ' var_info.parentinfo_InternalInterfaceVarName '!");' newline];

    out_get = [out_get S2];

    end

else  % we have a struct


    size_varname = [InternalInterfaceVarName '_size'];
    S1 = ['UINT32 ' size_varname ' = sizeof(' InternalInterfaceVarName ');' newline ...
        ];

    out_size = [out_size S1];
    
    S2 = ['ret = svi_GetBlk(pSviLib_' exchange_app.(var.parent_App).refC_name, ', SA_' InternalInterfaceVarName ', (UINT32*) &' InternalInterfaceVarName ...
        ',  &' size_varname ');' newline ...
        'if (ret != SVI_E_OK)' newline ...
        '   LOG_W(0, "SviClt_Read", "Could not read value ' InternalInterfaceVarName '!");' newline];

    out_get = [out_get S2];

end



end

% iN = 1;
% S1 = [newline];
% S2 = [newline]; 
% for iVar = 1 : length(SFTI)
% 
%   idx = strcmp(VarNameENOHOST, SFTI{iVar}); 
%   idx2 = strcmp(VarNamePLCGeneral,VarNamePLC{idx});
% 
%    
%     S1 = [S1 'UINT32 SIZE' num2str(iN)  ' = sizeof(' VarNameENOHOST{idx} ');' newline ...
%         ];
%     
%     S2 = [S2 'ret = svi_GetBlk(pSviLib_' Application{idx2}, ', SA_' SFTI{iVar} ', (UINT32*) &' VarNameENOHOST{idx} ...
%         ',  &SIZE' num2str(iN) ');' newline ...
%         'if (ret != SVI_E_OK)' newline ...
%         '   LOG_W(0, "SviClt_Read", "Could not read value ' SFTI{iVar} '!");' newline];
%  
% %     Field8Text = [Field8Text String];
%     
%  iN = iN + 1; 
% end
% 
% S3 = [newline];
% S4 = [newline];
% idx = find(ArrayPLC);
% for iVar = 1 : sum(ArrayPLC)
%    
%   temp = VarNamePLCGeneral{idx(iVar)}; 
%   temp(regexp(temp,'[.]'))=[];
%   
%   S3 = [S3 'UINT32 SIZE' num2str(iN)  ' = sizeof(' temp ');' newline ...       
%         ];
% 
%   S4 = [S4  'ret = svi_GetBlk(pSviLib_' Application{idx(iVar)}, ', SA_' temp ', (UINT32*) &' temp ...
%         ',  &SIZE' num2str(iN) ');' newline ...
%         'if (ret != SVI_E_OK)' newline ...
%         '   LOG_W(0, "SviClt_Read", "Could not read value ' temp '!");' newline];
% %      Field8Text = [Field8Text String];
%      
%   iN = iN + 1; 
% 
% end