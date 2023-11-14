function out_txt = output_variable_assignment(ModelName, Connect, OutputPortname, OutputType)

% ArrayPLC = ReadVarFromSVI;
% idxArray = find(ArrayPLC);
Field98Text = [newline];
String = [];
for iVar = 1 : length(Connect)

    if Connect(iVar)
        OutputPortname{iVar};

        if strcmp(OutputType{iVar}, 'STRING')

            fprintf('Variable Type to be included in App not yet implemented')
            keyboard

        elseif strcmp(OutputType{iVar}, 'REAL32') || strcmp(OutputType{iVar}, 'SINT16') || strcmp(OutputType{iVar}, 'BOOL8')

            String = [OutputPortname{iVar} ' = ' ModelName '_Y.' OutputPortname{iVar} ';' newline];

        else

            fprintf('Variable Type to be included in App not yet implemented')
            keyboard
        end

        Field98Text = [Field98Text String];

        %       iVV = iVV + 1;

    end
end

out_txt = Field98Text;

end