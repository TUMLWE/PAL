function out_txt = initialize_var_and_sa(ReadInputVarFromSVI, VarNamePLC, InputType, Connect, OutputPortname, OutputType)


string_warning = ['Variable Type to be included in App not yet implemented'];

exchanged_Varname_List = [VarNamePLC(ReadInputVarFromSVI); OutputPortname(Connect)]; 
exchanged_Vartype_List = [ InputType(ReadInputVarFromSVI); OutputType(Connect)];

Field2Text = [newline newline];

for iVar = 1 : length(exchanged_Varname_List)

    varname = exchanged_Varname_List{iVar};
    vartype = exchanged_Vartype_List{iVar};
    
    % define SVIAddress
    String = ['MLOCAL SVI_ADDR SA_' varname ';' newline ...
                ];

    % define and initialize exchanged variables
    
    switch vartype 
        case 'STRING'

            warning(string_warning)
            keyboard

        case 'REAL32'

            String = [String 'MLOCAL REAL32 ' varname ' = 0.0f;' newline];

        case 'BOOL8'

            String = [String 'MLOCAL BOOL8 ' varname ' = 0;' newline];

        case 'SINT16'

            warning(string_warning)
            keyboard

        otherwise
            
            warning(string_warning)
            keyboard
            
    end

        Field2Text = [Field2Text String];

end

out_txt = Field2Text;
end

