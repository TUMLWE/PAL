function out = itfc_svivar_coupling(var, bachmanndict, info)


out = strings;

VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

% temp = VarName;
% temp(regexp(temp,'[._]'))=[];
% InternalInterfaceVarName = temp;
InternalInterfaceVarName = info.InternalInterfaceVarName;

switch var.VarType

    case 'struct'

        StructName = info.StructName;

%         numberoflements = var.VarSize;
%         PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        out = ['{"' VarName '", SVI_F_INOUT | SVI_F_BLK | SVI_F_MIXED, sizeof(struct ' StructName '), (UINT32 *) &', InternalInterfaceVarName   ', 0, NULL, NULL}'];

        out = [out newline ',' newline];
        
        for iSubfield = 1 : length(var.SV)
            subvar = var.SV{iSubfield};

            if ~subvar.Create % if the variable should not be created
                continue
            end

            subvarname = subvar.VarName;
            mattype = subvar.VarType;
            PLCType = bachmanndict.mat_to_plc_type(mattype);
            PLCsize = bachmanndict.size_of_mat_type(mattype);
            numberoflements = subvar.VarSize;


            switch PLCType

                case {'CHAR'}

                    String2 = ['{"' VarName '.' subvarname '", SVI_F_INOUT | SVI_F_STRING, sizeof(' PLCType '[' num2str(numberoflements)  ']), (UINT32 *) &', InternalInterfaceVarName '.' subvarname ', 0, NULL, NULL}' newline ',' newline];
                
                case {'SINT16', 'BOOL8' 'UINT16'}
                    String2 = ['{"' VarName '.' subvarname '", SVI_F_INOUT | SVI_F_' PLCType ' , sizeof(' PLCType '[' num2str(numberoflements)  ']), (UINT32 *) &', InternalInterfaceVarName '.' subvarname ', 0, NULL, NULL}' newline ',' newline];

                case {'REAL64', 'REAL32'}
                    String2 = ['{"' VarName '.' subvarname '", SVI_F_INOUT | SVI_F_BLK | SVI_F_' PLCType ' , sizeof(' PLCType '[' num2str(numberoflements)  ']), (UINT32 *) &', InternalInterfaceVarName '.' subvarname ', 0, NULL, NULL}' newline ',' newline];

                otherwise
                    fprintf('Subfield Type not yet implemented \n')
                    keyboard

            end
            out = [out String2];

        end

    case {'double'}

        numberoflements = var.VarSize;
        PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        if numberoflements==1
%             String = ['{"' VarName '", SVI_F_INOUT | SVI_F_' PLCType ', sizeof(' PLCType '), (UINT32 *) &', InternalInterfaceVarName ', 0, NULL, NULL}'];
            String = ['{"' VarName '", SVI_F_INOUT | SVI_F_BLK | SVI_F_' PLCType ', sizeof(' PLCType '[' num2str(numberoflements) ']), (UINT32 *) &', InternalInterfaceVarName ', 0, NULL, NULL}'];

        elseif numberoflements>1
            String = ['{"' VarName '", SVI_F_INOUT | SVI_F_BLK | SVI_F_' PLCType ', sizeof(' PLCType '[' num2str(numberoflements) ']), (UINT32 *) &', InternalInterfaceVarName ', 0, NULL, NULL}'];

        else
            warning('Please check size of variable, error detected.')
            keyboard
        end

        String = [String ',' newline];
        out = String;

    case {'int16' 'logical' 'uint16' 'single'}

        numberoflements = var.VarSize;
        PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        if numberoflements==1
%             String = ['{"' VarName '", SVI_F_INOUT | SVI_F_' PLCType ', sizeof(' PLCType '), (UINT32 *) &', InternalInterfaceVarName ', 0, NULL, NULL}'];
            String = ['{"' VarName '", SVI_F_INOUT | SVI_F_' PLCType ', sizeof(' PLCType '[' num2str(numberoflements) ']), (UINT32 *) &', InternalInterfaceVarName ', 0, NULL, NULL}'];

        elseif numberoflements>1
            String = ['{"' VarName '", SVI_F_INOUT | SVI_F_BLK | SVI_F_' PLCType ', sizeof(' PLCType '[' num2str(numberoflements) ']), (UINT32 *) &', InternalInterfaceVarName ', 0, NULL, NULL}'];

        else
            warning('Please check size of variable, error detected.')
            keyboard
        end

        String = [String ',' newline];
        out = String;

    case 'char'

        numberoflements = var.VarSize;
        PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        String = ['{"' VarName '", SVI_F_INOUT | SVI_F_STRING, sizeof(' PLCType '[' ...
        num2str(numberoflements) ']), (UINT32 *) &' InternalInterfaceVarName ', 0, NULL, NULL},' newline ...
        ];
        out = String;

    otherwise
            warning('Data type not yet implemented, please check.')
            keyboard


end

