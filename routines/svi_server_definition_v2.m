function [out] = svi_server_definition_v2(SviLibToAdd)

out = newline;
String = strings;

for iVar = 1 : length(SviLibToAdd)

    String = ['MLOCAL SINT32  (**pSviLib_' SviLibToAdd{iVar} ')  ()= NULL;' newline];
    out = [out String];

end


end