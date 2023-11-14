function out_txt = read_svi_inputs(ReadInputVarFromSVI, AppTSVT, VarNamePLC)


Field5Text = [newline];

for iVar = 1 : length(ReadInputVarFromSVI)

    if ~ReadInputVarFromSVI(iVar)
        continue;
    end

    String = ['ret = svi_GetVal(pSviLib_' AppTSVT{iVar} ', SA_' VarNamePLC{iVar} ', (UINT32*) &' VarNamePLC{iVar} ' );' newline ...
        'if (ret != SVI_E_OK)' newline ...
        '     LOG_W(0, "SviClt_Read", "Could not read value ' VarNamePLC{iVar} '!");' newline ...
        newline];

    Field5Text = [Field5Text String];

end

out_txt = Field5Text;

end