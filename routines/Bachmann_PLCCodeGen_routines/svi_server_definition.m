function [out_txt, SviLibToAdd] = svi_server_definition(ReadInputVarFromSVI, AppTSVT, OutputAppTSVT, Connect)


% k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
% k2 = strfind(TXTSource, ClosingString)-1;

if ReadInputVarFromSVI ~= 0
    if isempty(AppTSVT(ReadInputVarFromSVI)) || isempty(OutputAppTSVT(Connect))

        fprintf('Warning: SubApp is not specified for some SVI Elements to be connected \n');
        keyboard
    end
end

SviLibToAdd = unique([AppTSVT(ReadInputVarFromSVI); OutputAppTSVT(Connect)]);


% String = TXTSource(k1:k2);

Field1Text = newline;
for iVar = 1 : length(SviLibToAdd)

    String = ['MLOCAL SINT32  (**pSviLib_' SviLibToAdd{iVar} ')  ()= NULL;' newline];
    Field1Text = [Field1Text String];

end

out_txt = Field1Text;

end
