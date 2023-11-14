function out_txt = get_itfc_sa_addres_var(ReadInputVarFromSVI, InputPortNumber, ...
    Name_Inports, VarNamePLC, HostATSVT, AppName, HostVNISA, HostConnect, AppTSVT, ...
    HostVarNameSVI, Connect, OutputAppTSVT, OutputPortNum, Name_Outports, ...
    OutputPortname, SVIVarName)

warning('WARNING: ADJUST THIS FILE!!!')

Field7Text = [newline];
CountNOI = 0; %Count number of input, to check whether all the inputs of the simulink model have been connected. Not necessary in principle but dangerous 

for iVar = 1 : length(ReadInputVarFromSVI)
    
    if ~ReadInputVarFromSVI(iVar)
        continue;
    end
    
    if InputPortNumber{iVar} == find(strcmp(Name_Inports, VarNamePLC{iVar}) )
        CountNOI = CountNOI + 1;

        % That's just a check
    else
        fprintf('error: Simulink Input Numbering does not match Excel Output Definition\n');
        keyboard
    end
        
    MatchApp = zeros(length(HostATSVT),1);
    MatchApp2 = zeros(length(HostATSVT),1);
    
    for iL = 1 : length(HostATSVT)
        
        if isnan(HostATSVT{iL})
            continue
        end
        
        MatchApp(iL) = contains(HostATSVT{iL}, AppName);
    end
    
    for iL = 1 : length(HostVNISA)
        
        if isnan(HostVNISA{iL})
            continue
        end
        
        MatchApp2(iL) = contains(HostVNISA{iL}, VarNamePLC{iVar});
        
    end
    
    
    BBBB = find(MatchApp & MatchApp2);
    if length(BBBB)~=1
        fprintf('error: Simulink input %s not properly defined in ENOHOST excel\n', VarNamePLC{iVar});
        keyboard
    end
    
    C1 = strtrim(strsplit(HostATSVT{BBBB},','));
    C2 = strtrim(strsplit(HostVNISA{BBBB},','));
    
    BBBB2 = find(strcmp(C1,AppName));
    
    if ~(length(BBBB2)==1 && strcmp(C2{BBBB2},VarNamePLC{iVar}) && HostConnect{BBBB})
        fprintf('error: Simulink input %s not properly defined in ENOHOST excel, maybe input is not connected\n', VarNamePLC{iVar});
        keyboard
    end
        
    String = ['if (svi_GetAddr(pSviLib_' AppTSVT{iVar} ', "' HostVarNameSVI{BBBB} '", &SA_' ...
        VarNamePLC{iVar} ', &SviFormat) != SVI_E_OK)' newline ...
        '{' newline ...
        '    LOG_W(0, Func, "Could not get address of value ' HostVarNameSVI{BBBB} '!");' newline ...
        '    return (ERROR);' newline ...
        '}' newline ...
        ];
    
    Field7Text = [Field7Text String];
    
end

if ~isempty(Name_Inports{1})
if CountNOI ~= length(Name_Inports)
warning('Warning: Number of simulink input connected is NOT EQUAL to total number of simulink input. Check for consistency and proceed with care.')
keyboard
end
end

for iVar = 1 : length(Connect)
    
    if ~Connect(iVar)
        continue;
    end
    if strcmp(OutputAppTSVT{iVar},'TUMITFCT')
       continue 
    end
    
    if OutputPortNum{iVar} == find(strcmp(Name_Outports, OutputPortname{iVar}) )
        % That's just a check
    else
        fprintf('error: Simulink Output Numbering does not match Excel Output Definition\n');
        keyboard
    end
        
    MatchApp = zeros(length(HostVarNameSVI),1);

    for iL = 1 : length(HostVarNameSVI)
        
        if isnan(HostVarNameSVI{iL})
            continue
        end
        
        if isnan(SVIVarName{iVar})
        fprintf('error: Simulink output %s is connected, but no SVI Variable Name has been specified \n', OutputPortname{iVar});
        keyboard            
        end
        
        MatchApp(iL) = strcmp(HostVarNameSVI{iL}, SVIVarName{iVar});
    end
        
    BBBB = find(MatchApp);
    if length(BBBB)~=1
        fprintf('error: Simulink input %s not found in ENOHOST excel. Check that SVI variable %s is UNIQUELY present\n',  SVIVarName{iVar});
        keyboard
    end
            
    String = ['if (svi_GetAddr(pSviLib_' OutputAppTSVT{iVar} ', "' HostVarNameSVI{BBBB} '", &SA_' ...
        OutputPortname{iVar} ', &SviFormat) != SVI_E_OK)' newline ...
        '{' newline ...
        '    LOG_W(0, Func, "Could not get address of value ' HostVarNameSVI{BBBB} '!");' newline ...
        '    return (ERROR);' newline ...
        '}' newline ...
        ];
    
    Field7Text = [Field7Text String];
    
end

out_txt = Field7Text;


end