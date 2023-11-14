function [out, info] = itfc_define_declare_svi_inputs(var, bachmanndict, flag_hostrequest)

out = strings;


%     idx = idxVarToInclude(iVar);
VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

temp = VarName;
temp(regexp(temp,'[._]'))=[];

if flag_hostrequest
InternalInterfaceVarName = [var.AppName '_' temp];
else
InternalInterfaceVarName = temp;
end
info.InternalInterfaceVarName = InternalInterfaceVarName;

switch var.VarType

    case 'struct'

        % create structure type

        StructName = ['struct_' InternalInterfaceVarName];
        String = ['struct ' StructName '{' newline ];
        info.StructName = StructName;

        for iSubfield = 1 : length(var.SV)
            subvar = var.SV{iSubfield};

            if ~subvar.Create % if the variable should not be created
                continue
            end


            subvarname = subvar.VarName;
            mattype = subvar.VarType;
            PLCType = bachmanndict.mat_to_plc_type(mattype);

            switch PLCType
                case {'REAL64', 'REAL32', 'CHAR', 'SINT16', 'BOOL8' 'UINT16'}
                    String = [String '    ' PLCType ' ' subvarname '[' num2str(subvar.VarSize) '];' newline];

                otherwise
                    fprintf('Subfield Type not yet implemented \n')
                    keyboard

            end

        end
        String = [String '}' InternalInterfaceVarName ';' newline];
        out = String;
        
    case {'double' 'uint16'}

        %             if length(InterfaceSubfield)~= ArraySizeFromPLCVar
        %                fprintf('Size of variable %s does not match with interface dimension, check excel and variable size\n', VarName)
        %                keyboard
        %             end
        %
        %             if length(unique(cell2mat(InterfaceSubfield)))~= ArraySizeFromPLCVar
        %                fprintf('Index repetition is present for variable %s in InterfaceLinker sheet of excel\n', VarName)
        %                keyboard
        %             end
        %
        %             if length(strcmp(InterfaceSubfieldType,'REAL32'))~= ArraySizeFromPLCVar
        %                fprintf('subvariables for variable %s have different types, while they should not. check InterfaceLinker sheet of excel\n', VarName)
        %                keyboard
        %             end

        numberoflements = var.VarSize;
        PLCType = bachmanndict.mat_to_plc_type(var.VarType);

        out = [PLCType ' ' InternalInterfaceVarName '[' num2str(numberoflements) '];' newline] ;

    otherwise

        fprintf('Variable %s type is unknown\n', VarName)
        keyboard

end


end
