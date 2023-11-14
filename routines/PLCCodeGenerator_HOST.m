function PLCCodeGenerator_HOST(AppName, BachmannFolder, ...
    ReferenceCFiles, host, bachmanndict, sample_time, itfc)



% Create copy of the original source in the reference file folder
OrigAppFile = [ReferenceCFiles '\' [AppName] '_app.c'];
NewAppFile = [ReferenceCFiles '\NEW_' [AppName] '_app.c'];
copyfile(OrigAppFile, NewAppFile)
TXTSource = fileread(NewAppFile);

if isempty(host.Variables)
    return
end

%% I first need to write some settings of the host
MAXSTRLENGTH = 1024;  %% DO NOT CHANGE THAT, UNLESS YOU CHANGE ALSO THE ONE IN "create_matlabcodegen_fields_host"

FAST_FREQ_RATIO = host.FAST_FREQ_RATIO;
SLOW_FREQ_RATIO = host.SLOW_FREQ_RATIO;
CTRL_FREQ_RATIO = host.CTRL_FREQ_RATIO;
output_file_duration_s = host.output_file_duration_s;
output_FileName_main = host.out_filename;
output_path = host.output_path_in_PLC;
CycleTime_ms = sample_time*1000; % in ms


% logging frequencies
OpeningString = ['// Buffer and Writer Task Settings' newline ....
'MLOCAL UINT32 FAST_FREQ_RATIO = ' num2str(FAST_FREQ_RATIO) ';       // Ratio between AppCklRate and Fast Buffer' newline ...
'MLOCAL UINT32 SLOW_FREQ_RATIO = ' num2str(SLOW_FREQ_RATIO) ';      // Ratio between AppCklRate and Slow Buffer' newline ...
'MLOCAL UINT32 CTRL_FREQ_RATIO = ' num2str(CTRL_FREQ_RATIO) ';      // Ratio between AppCklRate and Control Buffer'];
ClosingString = [];
txtAfter = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

if strcmp(output_FileName_main,string) || strcmp(output_path,string)

    error('Error in host app: no "output_FileName_main" or "output_path" was found.')

end

OpeningString = ['char output_FileName_main[' num2str(MAXSTRLENGTH) '] = "' output_FileName_main '";' newline ...
'char output_path[' num2str(MAXSTRLENGTH) '] = "' output_path '";' newline ...
'MLOCAL UINT32 newfile_time_s ='  num2str(output_file_duration_s) ';' newline ];
ClosingString = [];
txtAfter = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

% 

% substitute line of original file
OpeningString = ['    ' num2str(CycleTime_ms)];
ClosingString = [];
txtBefore = ['\/\* default task cycle time in ms \(\->Task\_CfgRead\) \*\/'];
TXTSource = addfield(5,txtBefore, OpeningString, ClosingString, TXTSource,1);

%%%%%%%%%%%%% Then i fill in the fields


% i select only the variables i want to create
Variables = host.Variables([host.Variables.Create]);

%% Field1
% find SVI servers
parent_App = {Variables.parent_App};
ee = cell2mat(cellfun(@(x) ~isempty(x), parent_App, 'Uniformoutput', false));
SviLibToAdd = unique(parent_App( ee) );

SviLibToAdd = cellfun(@(x) itfc.(x).refC_name, SviLibToAdd, 'Uniformoutput', false); % 20231013 CRS: added to always use the referenceC file as app name 

Field1Text = svi_server_definition_v2(SviLibToAdd);


%% Field6
% 		// MATLABCODEGEN: OpenField Get Specified  App Module
% 		// MATLABCODEGEN: CloseField Get Specified App Module
Field6Text = get_SVI_app_modules(SviLibToAdd);


% % % % %% Field2 (like submodels)
% % % % Field2Text = initialize_var_and_sa_v2(Variables, bachmanndict);



% initialize empty fields
Field2Text = [newline];
Field3Text = [newline];
Field4Text = [newline];
Field5Text = [newline];
Field7Text = [newline];
Field8Text = [newline];
Field9Text = [newline];
Field13Text = [newline];
Field14Text = [newline];
Field15Text = [newline];
Field16Text = [newline];
Field17Text = [newline];

var_info = [];
var_info.list_parentvar = [];
var_info.get_sa_addr_parentlist = [];
var_info.assign_sa_addr_parentlist = [];
var_info.assign_sa_addr_parentlist_write = [];

fast_fields = ['fprintf(fp, "TimeStamp_UTC; '];
slow_fields = ['fprintf(fp, "TimeStamp_UTC; '];
ctrl_fields = ['fprintf(fp, "TimeStamp_UTC; '];

for iVar = 1 : length(Variables)

    var = Variables(iVar);

    if ~var.Create % if the variable should not be created
        continue
    end

    InternalInterfaceVarName = ensure_varname_legality(var.VarName);
    var_info.InternalInterfaceVarName = InternalInterfaceVarName;


    % Field 2
    % // MATLABCODEGEN: OpenField SA Address and Variables to be read from interface
    % // MATLABCODEGEN: CloseField SA Address and Variables to be read from interface
    [f2_txt, var_info] = initialize_var_and_sa_v3(var, bachmanndict, var_info);

    % Field 3
    % // MATLABCODEGEN: OpenField SVI Variables Definition
    % // MATLABCODEGEN: CloseField SVI Variables Definition
    [f3_txt, var_info] = SVI_var_definition(var, bachmanndict, var_info);

    % Field4
    % OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
    % ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
    f4_txt = itfc_svivar_coupling(var, bachmanndict, var_info);

    %Field 5
    % // MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI
    % // MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI
    f5_txt = assign_parent_var_to_localvar(var, bachmanndict, var_info);

    %     Field7
    %     // MATLABCODEGEN: OpenField Get ITF SA Address
    %     // MATLABCODEGEN: CloseField Get ITF SA Address
    [f7_txt, var_info] = get_parent_sa_addres_var_v2(var, bachmanndict, var_info, itfc);


    % Field 8
    %     // MATLABCODEGEN: OpenField assign variable from SA_Address
    %     // MATLABCODEGEN: CloseField assign variable from SA_Address
    % Field 9
    %     // MATLABCODEGEN: OpenField Size of Array definition
    %     // MATLABCODEGEN: CloseField Size of Array definition

    [f9_txt, f8_txt, var_info] = assign_var_from_sa_address_var(var, bachmanndict, var_info, itfc);

    [fast_fields, slow_fields, ctrl_fields] = label_names_output_fields(var, fast_fields, slow_fields, ctrl_fields);


    
    % Field13
    %     OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file fast'];
    %     ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file fast'];
    % Field14
    %     OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file slow'];
    %     ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file slow'];
    % Field15
    %     OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file ctrl'];
    %     ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file ctrl'];
    [f13_txt, f14_txt, f15_txt] = print_out_to_file(var, var_info);

    % Field 16
    % // MATLABCODEGEN: OpenField assign variable in SviClt_Write
    % // MATLABCODEGEN: CloseField assign variable in SviClt_Write
    [f16_txt, var_info] = set_variables_sviclt_v2(var, bachmanndict, var_info, itfc);

    % Field 17
    % // MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI
    % // MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI
    f17_txt = assign_localvar_to_parent_var(var, bachmanndict, var_info);


    % final assignments
    Field2Text = [Field2Text f2_txt];
    Field3Text = [Field3Text f3_txt];
    Field4Text = [Field4Text f4_txt];
    Field5Text = [Field5Text f5_txt];
    Field7Text = [Field7Text f7_txt];
    Field8Text = [Field8Text f8_txt];
    Field9Text = [Field9Text f9_txt];
    Field13Text = [Field13Text f13_txt];
    Field14Text = [Field14Text f14_txt];
    Field15Text = [Field15Text f15_txt];
    Field16Text = [Field16Text f16_txt];
    Field17Text = [Field17Text f17_txt];


end

Field10Text = [fast_fields '\n");'];
Field11Text = [slow_fields '\n");'];
Field12Text = [ctrl_fields '\n");'];

% Field1  (like submodels)
OpeningString = ['// MATLABCODEGEN: OpenField SVI Server Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Server Definition'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field1Text TXTSource(k2:end)];

% Field 2
% // MATLABCODEGEN: OpenField SA Address and Variables to be read from interface
% // MATLABCODEGEN: CloseField SA Address and Variables to be read from interface
OpeningString = ['// MATLABCODEGEN: OpenField SA Address and Variables to be read from interface'];
ClosingString = ['// MATLABCODEGEN: CloseField SA Address and Variables to be read from interface'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field2Text TXTSource(k2:end) ];


% Field 3
% // MATLABCODEGEN: OpenField SVI Variables Definition
% // MATLABCODEGEN: CloseField SVI Variables Definition
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Definition'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field3Text TXTSource(k2:end) ];


% Field4 
% 	// MATLABCODEGEN: OpenField SVI Variables Coupling
% 	// MATLABCODEGEN: CloseField SVI Variables Coupling
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field4Text TXTSource(k2:end) ];


%Field 5 
% // MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field5Text TXTSource(k2:end) ];


% Field 6
% 		// MATLABCODEGEN: OpenField Get Specified  App Module
% 		// MATLABCODEGEN: CloseField Get Specified App Module
OpeningString = ['// MATLABCODEGEN: OpenField Get Specified  App Module'];
ClosingString = ['// MATLABCODEGEN: CloseField Get Specified App Module'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field6Text TXTSource(k2:end) ];


% Field7
% // MATLABCODEGEN: OpenField Get ITF SA Address
% // MATLABCODEGEN: CloseField Get ITF SA Address
OpeningString = ['// MATLABCODEGEN: OpenField Get ITF SA Address'];
ClosingString = ['// MATLABCODEGEN: CloseField Get ITF SA Address'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field7Text TXTSource(k2:end) ];


% Field 8 
%     // MATLABCODEGEN: OpenField assign variable from SA_Address
%     // MATLABCODEGEN: CloseField assign variable from SA_Address
OpeningString = ['// MATLABCODEGEN: OpenField assign variable from SA_Address'];
ClosingString = ['// MATLABCODEGEN: CloseField assign variable from SA_Address'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field8Text TXTSource(k2:end) ];

% Field 9 
%     // MATLABCODEGEN: OpenField Size of Array definition
%     // MATLABCODEGEN: CloseField Size of Array definition
OpeningString = ['// MATLABCODEGEN: OpenField Size of Array definition'];
ClosingString = ['// MATLABCODEGEN: CloseField Size of Array definition'];    
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field9Text TXTSource(k2:end) ];


% add fields to user defined functions
% Field10
OpeningString = ['// MATLABCODEGEN: OpenField  Label Names Output Fields fast'];
ClosingString = ['// MATLABCODEGEN: CloseField  Label Names Output Fields fast'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field10Text TXTSource(k2:end) ];


% Field11
OpeningString = ['// MATLABCODEGEN: OpenField  Label Names Output Fields slow'];
ClosingString = ['// MATLABCODEGEN: CloseField  Label Names Output Fields slow'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field11Text TXTSource(k2:end) ];


% Field12
OpeningString = ['// MATLABCODEGEN: OpenField  Label Names Output Fields ctrl'];
ClosingString = ['// MATLABCODEGEN: CloseField  Label Names Output Fields ctrl'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field12Text TXTSource(k2:end) ];


% Field13
OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file fast'];
ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file fast'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field13Text TXTSource(k2:end) ];


% Field14
OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file slow'];
ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file slow'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field14Text TXTSource(k2:end) ];

% Field15
OpeningString = ['// MATLABCODEGEN: OpenField  Print output to file ctrl'];
ClosingString = ['// MATLABCODEGEN: CloseField  Print output to file ctrl'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field15Text TXTSource(k2:end) ];


% Field 16
% // MATLABCODEGEN: OpenField assign variable in SviClt_Write
% // MATLABCODEGEN: CloseField assign variable in SviClt_Write
OpeningString = ['// MATLABCODEGEN: OpenField assign variable in SviClt_Write'];
ClosingString = ['// MATLABCODEGEN: CloseField assign variable in SviClt_Write'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;
TXTSource = [TXTSource(1:k1) Field16Text TXTSource(k2:end) ];

% Field 17
% // MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;
TXTSource = [TXTSource(1:k1) Field17Text TXTSource(k2:end) ];


% Write final _app_file
fid = fopen(NewAppFile,'w');
fprintf(fid, '%s', TXTSource);
fclose(fid);

mkdir(BachmannFolder)

copyfile(NewAppFile, [BachmannFolder '\' AppName '_app.c'])


end