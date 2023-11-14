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






% % % % % % % % % % % % % % % % % % % % % % % try 
% % % % % % % % % % % % % % % % % % % % % % %     var.parent;
% % % % % % % % % % % % % % % % % % % % % % % catch
% % % % % % % % % % % % % % % % % % % % % % %     warning('Could not find parent for variable %s', var.TagName)
% % % % % % % % % % % % % % % % % % % % % % %     keyboard
% % % % % % % % % % % % % % % % % % % % % % % end
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % % VarName_parents = 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % exchanged_Varname_List = [VarNamePLC(ReadInputVarFromSVI); OutputPortname(Connect)]; 
% % % % % % % % % % % % % % % % % % % % % % % exchanged_Vartype_List = [ InputType(ReadInputVarFromSVI); OutputType(Connect)];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % Field2Text = [newline newline];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % for iVar = 1 : length(exchanged_Varname_List)
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % parent = var.parent;
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % VarName = var.parent_App;
% % % % % % % % % % % % % % % % % % % % % % %  var.parent_TagName;
% % % % % % % % % % % % % % % % % % % % % % %  var.parent_SubVar;
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % var.parent 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % temp = VarName;
% % % % % % % % % % % % % % % % % % % % % % % temp(regexp(temp,'[._]'))=[];
% % % % % % % % % % % % % % % % % % % % % % % InternalInterfaceVarName = temp;
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % info.InternalInterfaceVarName = InternalInterfaceVarName;
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %     varname = exchanged_Varname_List{iVar};
% % % % % % % % % % % % % % % % % % % % % % %     vartype = exchanged_Vartype_List{iVar};
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     % define SVIAddress
% % % % % % % % % % % % % % % % % % % % % % %     String = ['MLOCAL SVI_ADDR SA_' varname ';' newline ...
% % % % % % % % % % % % % % % % % % % % % % %                 ];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %     % define and initialize exchanged variables
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     switch vartype 
% % % % % % % % % % % % % % % % % % % % % % %         case 'STRING'
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %             warning(string_warning)
% % % % % % % % % % % % % % % % % % % % % % %             keyboard
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %         case 'REAL32'
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %             String = [String 'MLOCAL REAL32 ' varname ' = 0.0f;' newline];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %         case 'BOOL8'
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %             String = [String 'MLOCAL BOOL8 ' varname ' = 0;' newline];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %         case 'SINT16'
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %             warning(string_warning)
% % % % % % % % % % % % % % % % % % % % % % %             keyboard
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %         otherwise
% % % % % % % % % % % % % % % % % % % % % % %             
% % % % % % % % % % % % % % % % % % % % % % %             warning(string_warning)
% % % % % % % % % % % % % % % % % % % % % % %             keyboard
% % % % % % % % % % % % % % % % % % % % % % %             
% % % % % % % % % % % % % % % % % % % % % % %     end
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %         Field2Text = [Field2Text String];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % end
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % out_txt = Field2Text;
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % %%
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % VariablesArrayInENOHOST = {};
% % % % % % % % % % % % % % % % % % % % % % % idxArray = find(ArrayPLC);
% % % % % % % % % % % % % % % % % % % % % % % Field2Text = [newline newline];
% % % % % % % % % % % % % % % % % % % % % % % iVV = 1;
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % for iVar = 1 : nSubF
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     VariablesArrayInENOHOST{iVV} = strtrim(SFTI{iVar});
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %  IdxVar = strcmp(VarNameENOHOST, SFTI{iVar} ) ; 
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % % NElem = Length(IdxVar);   
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %    if strcmp(Type{IdxVar}, 'STRING') 
% % % % % % % % % % % % % % % % % % % % % % %             
% % % % % % % % % % % % % % % % % % % % % % % %     String = ['MLOCAL SVI_ADDR SA_' VariablesArrayInENOHOST{iVV} ';' newline ...
% % % % % % % % % % % % % % % % % % % % % % % %         'MLOCAL CHAR ' VariablesArrayInENOHOST{iVV} '[' num2str(NElem) '];' newline newline];
% % % % % % % % % % % % % % % % % % % % % % % %  
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %     String = ['MLOCAL SVI_ADDR SA_' VariablesArrayInENOHOST{iVV} ';' newline ...
% % % % % % % % % % % % % % % % % % % % % % %         ];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %    elseif strcmp(Type{IdxVar}, 'REAL32') || strcmp(Type{IdxVar}, 'SINT16') || strcmp(Type{IdxVar}, 'BOOL8')
% % % % % % % % % % % % % % % % % % % % % % %        
% % % % % % % % % % % % % % % % % % % % % % % %     String = ['MLOCAL SVI_ADDR SA_' VariablesArrayInENOHOST{iVV} ';' newline ...
% % % % % % % % % % % % % % % % % % % % % % % %         'MLOCAL ' Type{IdxVar} ' ' VariablesArrayInENOHOST{iVV} ';' newline newline];     
% % % % % % % % % % % % % % % % % % % % % % % %  
% % % % % % % % % % % % % % % % % % % % % % %         String = ['MLOCAL SVI_ADDR SA_' VariablesArrayInENOHOST{iVV} ';' newline ...
% % % % % % % % % % % % % % % % % % % % % % %          ];     
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %    else
% % % % % % % % % % % % % % % % % % % % % % %        
% % % % % % % % % % % % % % % % % % % % % % %        fprintf('Variable Type to be included in ENOHOST not yet implemented')
% % % % % % % % % % % % % % % % % % % % % % %        keyboard
% % % % % % % % % % % % % % % % % % % % % % %    end
% % % % % % % % % % % % % % % % % % % % % % %    
% % % % % % % % % % % % % % % % % % % % % % %       Field2Text = [Field2Text String];
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % %       iVV = iVV + 1;
% % % % % % % % % % % % % % % % % % % % % % %  
% % % % % % % % % % % % % % % % % % % % % % % end
% % % % % % % % % % % % % % % % % % % % % % % 
% % % % % % % % % % % % % % % % % % % % % % % idx = find(ArrayPLC);
% % % % % % % % % % % % % % % % % % % % % % % for iVar = 1 : sum(ArrayPLC)
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     temp = VarNamePLCGeneral{idx(iVar)};
% % % % % % % % % % % % % % % % % % % % % % %     temp(regexp(temp,'[.]'))=[];
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     String = ['MLOCAL SVI_ADDR SA_' temp ';' newline ...
% % % % % % % % % % % % % % % % % % % % % % %         ];
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     if strcmp(VarTypePLC{idx(iVar)}, 'REAL32_BLK') || strcmp(VarTypePLC{idx(iVar)}, 'REAL32')
% % % % % % % % % % % % % % % % % % % % % % %         String = [String 'MLOCAL REAL32 ' temp '[' num2str(LengthPLCVar{idx(iVar)}/4) '];' newline];
% % % % % % % % % % % % % % % % % % % % % % %     else
% % % % % % % % % % % % % % % % % % % % % % %         
% % % % % % % % % % % % % % % % % % % % % % %         fprintf('Variable Type to be included in ENOHOST not yet implemented')
% % % % % % % % % % % % % % % % % % % % % % %         keyboard
% % % % % % % % % % % % % % % % % % % % % % %         
% % % % % % % % % % % % % % % % % % % % % % %     end
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % %     Field2Text = [Field2Text String];
% % % % % % % % % % % % % % % % % % % % % % %     
% % % % % % % % % % % % % % % % % % % % % % % end
% % % % % % % % % % % % % % % % % % % % % % % 
end
