function out_txt = set_variables_sviclt(Connect, OutputAppTSVT, OutputPortname)

warning("WARNING: ADJUST THIS FUNCTION")

Field97Text = [newline newline];
String = [];

for iVar = 1 : length(Connect)

    if ~Connect(iVar)
        continue;
    end
    if strcmp(OutputAppTSVT{iVar},'TUMITFCT')
       continue 
    end

    String = ['ret = svi_SetVal(pSviLib_' OutputAppTSVT{iVar} ', SA_' OutputPortname{iVar} ', *(UINT32*) &' OutputPortname{iVar} ' );' newline ...
        'if (ret != SVI_E_OK)' newline ...
        '     LOG_W(0, "SviClt_Write", "Could not read value ' OutputPortname{iVar} '!");' newline ...
        newline];

%         ret = svi_SetVal(pSviLib_ENOHOST , SA_output_WS_ms,  *(UINT32*) &output_WS);
%     if (ret != SVI_E_OK) LOG_W(0, "SviClt_Write", "Could not write value output_WS!");

    
  Field97Text = [Field97Text String];
       
end


out_txt = Field97Text;

end