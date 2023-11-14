function create_matlabcodegen_fields_host(SourceRefPath, SourceRefFileName, TargetFolder, host_settings)
% This routine modifies a _app.c bachmann file, adding the necessary lines
% for the SVI interconnection and internal variables definition.


[is, ie] = regexp(SourceRefFileName, '.*_app.c');

if isempty(is)
   
    error('The selected source file name does not end with "_app.c". Please correct it.');
    
end

MAXSTRLENGTH = 1024;  %% DO NOT CHANGE THAT, UNLESS YOU CHANGE ALSO THE ONE IN "PLCCodeGenerator_HOST"

%%
TXTSource = fileread(fullfile(SourceRefPath, SourceRefFileName));


% Add SviClt read/write
OpeningString = ['MLOCAL SINT32 SviClt_Read();'];
ClosingString = ['MLOCAL SINT32 SviClt_Write(VOID);'];
txtBefore = '/* Global variables: data structure for mconfig parameters */';
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);

% OpeningString = ['MLOCAL SINT32 SviClt_Read();'];
% ClosingString = [];
% txtAfter = 'MLOCAL VOID SviClt_Deinit\(VOID\);';
% TXTSource = addfield(4,txtAfter, OpeningString, ClosingString, TXTSource,1);

% Field1  (like submodels)
OpeningString = ['// MATLABCODEGEN: OpenField SVI Server Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Server Definition'];
txtBefore = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);


% Field2 (like submodels)
% // MATLABCODEGEN: OpenField SA Address and Variables to be read from interface
% // MATLABCODEGEN: CloseField SA Address and Variables to be read from interface
OpeningString = ['// MATLABCODEGEN: OpenField SA Address and Variables to be read from interface'];
ClosingString = ['// MATLABCODEGEN: CloseField SA Address and Variables to be read from interface'];
txtBefore = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);

% user defined variables

OpeningString = ['MLOCAL UINT16 Recording_Start = TRUE;      // Needs to be UINT16 in order to be modbus compatible' newline ....
'MLOCAL UINT16 Recording_Start_prev = TRUE;      // Needs to be UINT16 in order to be modbus compatible' newline ...
'MLOCAL UINT32 CycleCount_new_file = 0;' newline ...
'MLOCAL SVI_ADDR Time_ms1970_SviAddr;' newline ...
'UINT64  Time_ms1970;' newline ...
'#define TRUE 1' newline ...
'#define FALSE 0' newline ...
'#define MAXSTRLENGTH ' num2str(MAXSTRLENGTH) newline ...
'MLOCAL FILE *pFileFast  = NULL;                 // File for fast data recording' newline ...
'MLOCAL FILE *pFileSlow  = NULL;                 // File for slow data recording' newline ...
'MLOCAL FILE *pFileCtrl  = NULL;                 // File for recording controller data' newline ...
'MLOCAL SINT32 write_header_fast();' newline ...
'MLOCAL SINT32 write_output_fast();' newline ...
'MLOCAL SINT32 write_header_slow();' newline ...
'MLOCAL SINT32 write_output_slow();' newline ...
'MLOCAL SINT32 write_header_ctrl();' newline ...
'MLOCAL SINT32 write_output_ctrl();' newline ...
'char output_FileName_fast[MAXSTRLENGTH];' newline ...
'char output_FileName_slow[MAXSTRLENGTH];' newline ...
'char output_FileName_ctrl[MAXSTRLENGTH];' newline ...
'char dateYMD_HMS[MAXSTRLENGTH];' newline ...
'MLOCAL time_t rawtime;' newline ...
'struct tm * timeinfo;' newline ...
'MLOCAL REAL32 CycleTime_ms;' newline ...
'MLOCAL UINT32 offset_newfile;' newline ...
'MLOCAL BOOL8 create_new_file_fast = FALSE;' newline ...
'MLOCAL BOOL8 create_new_file_slow = FALSE;' newline ...
'MLOCAL BOOL8 create_new_file_ctrl = FALSE;' newline ...
'MLOCAL CHAR Date_and_Time[24] = "2023.04.24_19:41:00:000";' newline ...
];
ClosingString = [];
txtAfter = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);



% Field 3
% // MATLABCODEGEN: OpenField SVI Variables Definition
% // MATLABCODEGEN: CloseField SVI Variables Definition
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Definition'];
txtBefore = ['\/\*\s*\* Global variables\: Settings for application task'];
TXTSource = addfield(3,txtBefore, OpeningString, ClosingString, TXTSource,1);

%
OpeningString = [',' newline ...
'    {"TimeStamp_UTC", SVI_F_OUT   | SVI_F_STRING, sizeof(Date_and_Time),  (UINT32 *) Date_and_Time},' newline ...
'     {"Recording/Flag_Record", SVI_F_INOUT   | SVI_F_UINT16, sizeof(UINT16), (UINT32*) &Recording_Start},' newline ...
];
ClosingString = [];
txtIn = ['(?<=MLOCAL SVI_GLOBVAR SviGlobVarList.*)};'];
TXTSource = addfield(3, txtIn, OpeningString, ClosingString, TXTSource,1);

% Field4 
% 	// MATLABCODEGEN: OpenField SVI Variables Coupling
% 	// MATLABCODEGEN: CloseField SVI Variables Coupling
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
txtIn = ['(?<=MLOCAL SVI_GLOBVAR SviGlobVarList.*)};'];
TXTSource = addfield(3, txtIn, OpeningString, ClosingString, TXTSource,1);

% write Control_CycleInit
OpeningString = ['    CycleTime_ms = TaskProperties_aControl.CycleTime_ms;' newline ...
    '    offset_newfile = newfile_time_s / CycleTime_ms * 1000;'];
ClosingString = [''];
txtAfter = ['MLOCAL VOID Control_CycleInit\(VOID\)\s*{'];
TXTSource = addfield(4,txtAfter, OpeningString, ClosingString, TXTSource,1);


% write Control_CycleStart
cyclestart_txt = fileread('host_CycleStart_bachmann.txt');
OpeningString = [cyclestart_txt];
ClosingString = [];
txtAfter = ['MLOCAL VOID Control_CycleStart\(VOID\)\s*{'];
TXTSource = addfield(4,txtAfter, OpeningString, ClosingString, TXTSource,1);

%Field 5 
% // MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI'];
txtBefore = ['time ( &rawtime );'];
TXTSource = addfield(1, txtBefore, OpeningString, ClosingString, TXTSource,1);

% write Control_CycleEnd
OpeningString = ['    Recording_Start_prev = Recording_Start;'];
ClosingString = [];
txtAfter = ['MLOCAL VOID Control_CycleEnd\(TASK_PROPERTIES \* pTaskData\)\s*{'];
TXTSource = addfield(4,txtAfter, OpeningString, ClosingString, TXTSource,1);

% write SviClt_Init
OpeningString = ['    if (svi_GetAddr(pSviLib, "Time/Time_ms1970", &Time_ms1970_SviAddr, &SviFormat) != SVI_E_OK)' newline ...
    '    {' newline ...
    '        LOG_W(0, Func, "Could not get address of value ''Time_ms1970''!");' newline ...
    '        return (ERROR);' newline...
    '    }' newline ...
    ];
ClosingString = [];
txtBefore = ['MLOCAL SINT32 SviClt_Init\(VOID\)\s*' ...
        '{[^{}]*{[^{}]*}[^{}]'];
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


OpeningString = ['    SviClt_Write();'];
ClosingString = [];
txtBefore = ['MLOCAL VOID Control_CycleEnd\(TASK_PROPERTIES \* pTaskData\)\s*{'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);


% % Field 8 
% %     // MATLABCODEGEN: OpenField assign variable from SA_Address
% %     // MATLABCODEGEN: CloseField assign variable from SA_Address
% OpeningString = ['// MATLABCODEGEN: OpenField assign variable from SA_Address'];
% ClosingString = ['// MATLABCODEGEN: CloseField assign variable from SA_Address'];
% txtBefore = ['MLOCAL SINT32 SviClt_Read\(\)\s*' ...
%         '{\s*SINT32\s*ret\;'];
% TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

%     // MATLABCODEGEN: OpenField Size of Array definition
%     // MATLABCODEGEN: CloseField Size of Array definition
OpeningString = ['// MATLABCODEGEN: OpenField Size of Array definition'];
ClosingString = ['// MATLABCODEGEN: CloseField Size of Array definition'];    
txtBefore = ['MLOCAL SINT32 SviClt_Read\(\)\s*' ...
        '{\s*SINT32\s*ret\;'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Adding milliseconds
OpeningString = ['UINT32 Time_ms1970_size = sizeof(Time_ms1970);' newline ...
'ret = svi_GetBlk(pSviLib, Time_ms1970_SviAddr, (UINT32*) &Time_ms1970,  &Time_ms1970_size);' newline ...
'if (ret != SVI_E_OK)' newline ...
'    LOG_W(0, "SviClt_Read", "Could not read value Time_ms1970!");'];
ClosingString = [];  
txtAfter = ['// MATLABCODEGEN: CloseField Size of Array definition'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);


% write host_functions
host_functions = fileread('host_functions_bachmann.txt');
OpeningString = [host_functions];
ClosingString = [];
TXTSource = [TXTSource newline OpeningString];

% add fields to user defined functions
% Field10
OpeningString = ['// MATLABCODEGEN: OpenField  Label Names Output Fields fast'];
ClosingString = ['// MATLABCODEGEN: CloseField  Label Names Output Fields fast'];
txtBefore = ['MLOCAL SINT32 write_header_fast\(\)\s*' ...
        '{[^{}]*{[^{}]*}[^{}]'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Field11
OpeningString = ['// MATLABCODEGEN: OpenField  Label Names Output Fields slow'];
ClosingString = ['// MATLABCODEGEN: CloseField  Label Names Output Fields slow'];
txtBefore = ['MLOCAL SINT32 write_header_slow\(\)\s*' ...
        '{[^{}]*{[^{}]*}[^{}]'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);

% Field12
OpeningString = ['// MATLABCODEGEN: OpenField  Label Names Output Fields ctrl'];
ClosingString = ['// MATLABCODEGEN: CloseField  Label Names Output Fields ctrl'];
txtBefore = ['MLOCAL SINT32 write_header_ctrl\(\)\s*' ...
        '{[^{}]*{[^{}]*}[^{}]'];
TXTSource = addfield(4,txtBefore, OpeningString, ClosingString, TXTSource,1);


% Field13
[idx1, idx2] = regexp(TXTSource, 'fprintf\(fp, "\%s; ", Date_and_Time\);');
OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file fast'];
ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file fast'];
newstring = [newline OpeningString newline ClosingString newline];
TXTSource = [TXTSource(1:idx2(1)+1) newstring TXTSource(idx2(1)+1:end)];
% Field14
[idx1, idx2] = regexp(TXTSource, 'fprintf\(fp, "\%s; ", Date_and_Time\);');
OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file slow'];
ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file slow'];
newstring = [newline OpeningString newline ClosingString newline];
TXTSource = [TXTSource(1:idx2(2)+1) newstring TXTSource(idx2(2)+1:end)];
% Field15
[idx1, idx2] = regexp(TXTSource, 'fprintf\(fp, "\%s; ", Date_and_Time\);');
OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file ctrl'];
ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file ctrl'];
newstring = [newline OpeningString newline ClosingString newline];
TXTSource = [TXTSource(1:idx2(3)+1) newstring TXTSource(idx2(3)+1:end)];

% // MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI'];
txtAfter = ['// MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);


%
outfile = fullfile(TargetFolder, SourceRefFileName);
fid = fopen(outfile,'w');
fprintf(fid, '%s', TXTSource);
fclose(fid);


end