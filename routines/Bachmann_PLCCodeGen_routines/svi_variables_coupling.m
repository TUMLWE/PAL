function out_txt = svi_variables_coupling(Connect, OutputPortNum, Name_Outports, OutputPortname, OutputType, SVIVarName)
% this function defines the SVI of the application

string_warning = ['Variable Type to be included in App not yet implemented'];

Field4Text = [newline];

for iVar = 1 : length(Connect)

    if ~Connect(iVar)
        continue;
    end

    if OutputPortNum{iVar} == find(strcmp(Name_Outports, OutputPortname{iVar}) )
        % That's just a check
    else
        fprintf('error: Simulink Output Numbering does not match Excel Output Definition');
        keyboard
    end

    if strcmp(OutputType{iVar},'REAL32') || strcmp(OutputType{iVar},'SINT16') || strcmp(OutputType{iVar},'BOOL8')
        String = ['{"' SVIVarName{iVar} '", SVI_F_INOUT | SVI_F_' OutputType{iVar} ...
            ', sizeof(' OutputType{iVar} '), (UINT32 *) &' OutputPortname{iVar} ...
            ', 0, NULL, NULL},' newline ...
            ];

    elseif strcmp(OutputType{iVar},'STRING')

        %         String = ['{"' SVIVarName{VPLCList(iVar)} '", SVI_F_INOUT | SVI_F_' Type{VPLCList(iVar)} ', sizeof(char[' ...
        %     num2str(Length(VPLCList(iVar))) ']), (UINT32 *) &' VarNameENOHOST{VPLCList(iVar)} ', 0, NULL, NULL},' newline ...
        %         ];

        warning(string_warning)
        keyboard


    else
        warning(string_warning)
        keyboard

    end

    Field4Text = [Field4Text String];

end

out_txt = Field4Text;

end