function create_matlabcodegen_fields(SourceRefPath, SourceRefFileName, TargetFolder, HostName)
% This routine modifies a _app.c bachmann file, adding the necessary lines
% for the SVI interconnection and internal variables definition.


[is, ie] = regexp(SourceRefFileName, '.*_app.c');

if isempty(is)
   
    error('The selected source file name does not end with "_app.c". Please correct it.');
    
end

AppName = SourceRefFileName(1:end-6);

%%
TXTSource = fileread(fullfile(SourceRefPath, SourceRefFileName));


% Field1 
OpeningString = ['// MATLABCODEGEN: OpenField SVI Server Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Server Definition'];
txtBefore = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Field2
OpeningString = ['// MATLABCODEGEN: OpenField SA Address and Variables to be read from interface'];
ClosingString = ['// MATLABCODEGEN: CloseField SA Address and Variables to be read from interface'];
txtBefore = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Field 3
% // MATLABCODEGEN: OpenField SVI Variables Definition
% // MATLABCODEGEN: CloseField SVI Variables Definition
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Definition'];
txtBefore = ['\/\*\s*\* Global variables\: Settings for application task'];
TXTSource = addfield(3,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Field4
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
txtIn = ['(?<=MLOCAL SVI_GLOBVAR SviGlobVarList.*)};'];
TXTSource = addfield(3, txtIn, OpeningString, ClosingString, TXTSource,1);

% Field3
OpeningString = ['// MATLABCODEGEN: OpenField3 Simulink Model Input Assignment Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField3 Simulink Model Input Assignment Definition'];
txtBefore = ['MLOCAL VOID Control_CycleStart\(VOID\)\s*{'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Add SVI read in CycleStart
OpeningString = ['if (SviClt_Read() < 0)' newline ...
         '    LOG_W(0, "Control_CycleStart", "Could not read all SVI variables!");'];
ClosingString = [];
txtBefore = ['MLOCAL VOID Control_CycleStart\(VOID\)\s*{'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);


% Field 6
% 		// MATLABCODEGEN: OpenField Get Specified  App Module
% 		// MATLABCODEGEN: CloseField Get Specified App Module
OpeningString = ['// MATLABCODEGEN: OpenField Get Specified  App Module'];
ClosingString = ['// MATLABCODEGEN: CloseField Get Specified App Module'];
txtBefore = ['MLOCAL SINT32 SviClt_Init\(VOID\)\s*' ...
        '{[^{}]*{[^{}]*}[^{}]'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Field7
% // MATLABCODEGEN: OpenField Get ITF SA Address
% // MATLABCODEGEN: CloseField Get ITF SA Address
OpeningString = ['// MATLABCODEGEN: OpenField Get ITF SA Address'];
ClosingString = ['// MATLABCODEGEN: CloseField Get ITF SA Address'];
txtAfter = ['// MATLABCODEGEN: CloseField Get Specified App Module'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

% // MATLABCODEGEN: OpenField5  Read Variable SVI
% // MATLABCODEGEN: CloseField5 Read Variable SVI
% // MATLABCODEGEN: OpenField assign variable in SviClt_Write
% // MATLABCODEGEN: CloseField assign variable in SviClt_Write

% Add SviClt_Read()
OpeningString = ['MLOCAL SINT32 SviClt_Read()' newline ...
'{' newline ...
'    SINT32  ret;' newline ...
'    // MATLABCODEGEN: OpenField assign variable from SA_Address' newline ...
'    // MATLABCODEGEN: CloseField assign variable from SA_Address' newline ...
'    return (ret);' newline ...
'}' newline ...
'MLOCAL SINT32 SviClt_Write(VOID)' newline ...
'{' newline ...
'    SINT32  ret;' newline ...
'    // MATLABCODEGEN: OpenField assign variable in SviClt_Write' newline ...
'    // MATLABCODEGEN: CloseField assign variable in SviClt_Write' newline ...
'    return (ret);' newline ...
'}' newline ...
];
ClosingString = [];
txtBefore = ['MLOCAL SINT32 SviClt_Example\(UINT32 \* pTime_us\)\s*{'];
TXTSource = addfield(3,txtBefore, OpeningString, ClosingString, TXTSource,0);

%     // MATLABCODEGEN: OpenField Size of Array definition
%     // MATLABCODEGEN: CloseField Size of Array definition
OpeningString = ['// MATLABCODEGEN: OpenField Size of Array definition'];
ClosingString = ['// MATLABCODEGEN: CloseField Size of Array definition'];    
txtBefore = ['MLOCAL SINT32 SviClt_Read\(\)\s*' ...
        '{\s*SINT32\s*ret\;'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);



% // MATLABCODEGEN: OpenField Output Variable Definition
% // MATLABCODEGEN: CloseField Output Variable Definition
OpeningString = ['// MATLABCODEGEN: OpenField Output Variable Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField Output Variable Definition'];
txtAfter = ['MLOCAL UINT32 CycleCount = 0;'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

% Add #CC Stepper
OpeningString = ['    rt_OneStep();'];
ClosingString = [];
txtAfter = ['CycleCount++;'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,0);

% // MATLABCODEGEN: OpenField Output Variable Assignment
% // MATLABCODEGEN: CloseField Output Variable Assignment
OpeningString = ['// MATLABCODEGEN: OpenField Output Variable Assignment'];
ClosingString = ['// MATLABCODEGEN: CloseField Output Variable Assignment'];
txtAfter = ['rt_OneStep();'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);


% // MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI'];
txtAfter = ['// MATLABCODEGEN: CloseField Output Variable Assignment'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);


OpeningString = ['    SviClt_Write();'];
ClosingString = [];
txtBefore = ['MLOCAL VOID Control_CycleEnd\(TASK_PROPERTIES \* pTaskData\)\s*{'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% // MATLABCODEGEN: OpenField #initialize model
% // MATLABCODEGEN: CloseField #initialize model
OpeningString = ['// MATLABCODEGEN: OpenField #initialize model'];
ClosingString = ['// MATLABCODEGEN: CloseField #initialize model'];
txtBefore = ['MLOCAL VOID Control_CycleInit\(VOID\)\s*{'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Add AppStatus initialization
OpeningString = ['AppStatus = 1;'];
ClosingString = [];
txtAfter = ['// MATLABCODEGEN: CloseField #initialize model'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);


% // MATLABCODEGEN: OpenField #terminate model 
% // MATLABCODEGEN: CloseField #terminate model 
OpeningString = ['// MATLABCODEGEN: OpenField #terminate model'];
ClosingString = ['// MATLABCODEGEN: CloseField #terminate model'];
txtBefore = ['MLOCAL VOID Control_Main\(TASK_PROPERTIES \* pTaskData\)\s*' ...
        '{[^{}]*{[^{}]*}[^{}]'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Add AppStatus termination & SviClt_write
OpeningString = ['AppStatus = 0;'];
ClosingString = ['SviClt_Write();'];
txtAfter = ['// MATLABCODEGEN: CloseField #terminate model'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

% // MATLABCODEGEN: OpenField Terminate Variables
% // MATLABCODEGEN: CloseField Terminate Variables
OpeningString = ['// MATLABCODEGEN: OpenField Terminate Variables'];
ClosingString = ['// MATLABCODEGEN: CloseField Terminate Variables'];
txtAfter = ['// MATLABCODEGEN: CloseField #terminate model'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

%Field 5 
% // MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI'];
txtBefore = ['// MATLABCODEGEN: OpenField3 Simulink Model Input Assignment Definition'];
TXTSource = addfield(1, txtBefore, OpeningString, ClosingString, TXTSource,1);


% // MATLABCODEGEN: OpenField #IncludeHeader
% // MATLABCODEGEN: CloseField #IncludeHeader
OpeningString = ['// MATLABCODEGEN: OpenField #IncludeHeader'];
ClosingString = ['// MATLABCODEGEN: CloseField #IncludeHeader'];
txtBefore = '/* Functions: administration, to be called from outside this file */';
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);

% // MATLABCODEGEN: OpenField #Include rt_OneStep
% // MATLABCODEGEN: CloseField #Include rt_OneStep
OpeningString = ['// MATLABCODEGEN: OpenField #Include rt_OneStep'];
ClosingString = ['// MATLABCODEGEN: CloseField #Include rt_OneStep'];
txtBefore = '/* Functions: administration, to be called from outside this file */';
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);


% Add SviClt read/write
OpeningString = ['MLOCAL SINT32 SviClt_Read();'];
ClosingString = ['MLOCAL SINT32 SviClt_Write(VOID);'];
txtBefore = '/* Global variables: data structure for mconfig parameters */';
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);


% Add AppStatus SA and BOOL
OpeningString = ['MLOCAL SVI_ADDR SA_AppStatus;'];
ClosingString = [];
txtBefore = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);


% Add comma to SVIGlobVarList, in preparation to the other variables to add
OpeningString = [','];
ClosingString = [];
txtIn = ['(?!{"ModuleVersion"[^{}]*})[\s]*// MATLABCODEGEN: OpenField SVI Variables Coupling'];
TXTSource = addfield(3, txtIn, OpeningString, ClosingString, TXTSource,0);

%

%
% // MATLABCODEGEN: OpenField write output array TUMITFC
% // MATLABCODEGEN: CloseField write output array TUMITFC

%     // MATLABCODEGEN: OpenField specify size of array to be written in TUMITFC
%     // MATLABCODEGEN: CloseField specify size of array to be written in TUMITFC

outfile = fullfile(TargetFolder, SourceRefFileName);
fid = fopen(outfile,'w');
fprintf(fid, '%s', TXTSource);
fclose(fid);

end