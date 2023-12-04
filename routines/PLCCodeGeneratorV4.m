function PLCCodeGeneratorV4(ModelName, AppName, DestinationFolder, BachmannFolder, ...
    ReferenceCFiles, input_paramFile, sm, bachmanndict, host)

%%  
CurrFolder = pwd;

load_system(ModelName);

rtwbuild(ModelName);   % Build the model, generate the C code


MainFolder = fullfile(DestinationFolder , 'matlab', [ModelName '_ert_rtw']);
FileList = [dir([MainFolder '/*.h']); dir([MainFolder '/*.c'])];

SCF = [DestinationFolder '/SourceCode'];
mkdir(SCF);
mkdir(BachmannFolder)

for iL = 1 : length(FileList)
    
    copyfile([FileList(iL).folder '\' FileList(iL).name], SCF)
    if ~strcmp(FileList(iL).name,'ert_main.c')
        copyfile([FileList(iL).folder '\' FileList(iL).name], BachmannFolder)
    end
    
end

%% Generate Main File  
% load(input_paramFile)
sim(ModelName);

h_Inports  = find_system(ModelName,'FindAll','On','SearchDepth',1,'BlockType','Inport');
h_Outports = find_system(ModelName,'FindAll','On','SearchDepth',1,'BlockType','Outport');

Name_Inports  = get(h_Inports,'Name');
Name_Outports  = get(h_Outports,'Name');

if ~iscell(Name_Inports)   %if it is not cell, transform into a cell
    Name_Inports = {Name_Inports};
end
if ~iscell(Name_Outports)   %if it is not cell, transform into a cell
    Name_Outports = {Name_Outports};
end

maximum_length_sl_entry = 31; % the generated code truncates longer variable names

LimitLengthInputChar = cell2mat(cellfun(@(x) min([length(x),31]), Name_Inports, 'UniformOutput', false));
B = cellfun(@(x,y) x(1:y), Name_Inports, num2cell(LimitLengthInputChar), 'UniformOutput', false );

if ~isempty(Name_Inports{1})
if length(unique(B)) < length(Name_Inports)
error('Error: Simulink model input/output name are limited to 31 characters for c++ generation purpose, and some inputs of the model have the same first 31 characters, please check Simulink inputs'); 
end
end

LimitLengthOutputChar = cell2mat(cellfun(@(x) min([length(x),31]), Name_Outports, 'UniformOutput', false));
B = cellfun(@(x,y) x(1:y), Name_Outports, num2cell(LimitLengthOutputChar), 'UniformOutput', false );

if length(unique(B)) < length(Name_Outports)
error('Error: Simulink model input/output name are limited to 31 characters for c++ generation purpose, and some outputs of the model have the same first 31 characters, please check Simulink outputs'); 
end

%% Output modification (the simulink code must have run in order to generate the correct output, yout must be in the workspace) 

OrigAppFile = [ReferenceCFiles '\' [AppName] '_app.c'];
NewAppFile = [ReferenceCFiles '\NEW_' [AppName] '_app.c'];
copyfile(OrigAppFile, NewAppFile)
TXTSource = fileread(NewAppFile);

if ~exist('yout')
   error('ERROR: variable yout not found. Please run the Simulink model before generating the Bachmann Code') 
end

% // MATLABCODEGEN: OpenField #IncludeHeader
% // MATLABCODEGEN: CloseField #IncludeHeader

default_headername = [ModelName '.h'];
% ensure that header exists
if sum(find(strcmp({FileList.name}, default_headername))) ~= 1
    warning('Could not find default header file for SL model C code, please check and make sure that it is the intended behaviour')
    keyboard
end

string = ['#include "' default_headername '"'];

OpeningString = ['// MATLABCODEGEN: OpenField #IncludeHeader'];
ClosingString = ['// MATLABCODEGEN: CloseField #IncludeHeader'];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;

TXTSource = [TXTSource(1:k1) string TXTSource(k2:end) ];


% // MATLABCODEGEN: OpenField #Include rt_OneStep
% // MATLABCODEGEN: CloseField #Include rt_OneStep
OpeningString = ['// MATLABCODEGEN: OpenField #Include rt_OneStep'];
ClosingString = ['// MATLABCODEGEN: CloseField #Include rt_OneStep'];

string = ['void rt_OneStep(void);' newline ...
'void rt_OneStep(void)' newline ...
'{' newline ...
'    /* Step the model */' newline ...
'    ' ModelName '_step();' newline ...
'}' newline ...
];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;

TXTSource = [TXTSource(1:k1) string TXTSource(k2:end) ];

% // MATLABCODEGEN: OpenField #terminate model
% // MATLABCODEGEN: CloseField #terminate model
OpeningString = ['// MATLABCODEGEN: OpenField #terminate model'];
ClosingString = ['// MATLABCODEGEN: CloseField #terminate model'];
string = ['    /* Terminate model */' newline ...
'    ' ModelName '_terminate();' newline ...
];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;

TXTSource = [TXTSource(1:k1) string TXTSource(k2:end) ];

% // MATLABCODEGEN: OpenField #initialize model
% // MATLABCODEGEN: CloseField #initialize model
OpeningString = ['// MATLABCODEGEN: OpenField #initialize model'];
ClosingString = ['// MATLABCODEGEN: CloseField #initialize model'];
string = ['    ' ModelName '_initialize();' newline ...
];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;

TXTSource = [TXTSource(1:k1) string TXTSource(k2:end) ];



% Get address of AppStatus from HOST SVI

status_var = sm.Variables(find(strcmp({sm.Variables.IO}, 'status')));

if length(status_var)~=1
    error('Each submodel should have one ""Status"" variable defined!')
end
if isempty(status_var.parent)
    error('Could not find submodel %s status app in its host!', AppName)
end

% TXTSource = write_AppStatus(TXTSource, status_var, bachmanndict);
var_info = [];
var_info.assign_sa_addr_parentlist_write = [];
var_info.get_sa_addr_parentlist = [];
var_info.InternalInterfaceVarName = 'AppStatus';
var_info.parentinfo_InternalInterfaceVarName = 'AppStatus';
[AppStatus_write, foo] = set_variables_sviclt_v2(status_var, bachmanndict, var_info, host);
OpeningString = [AppStatus_write];
ClosingString = [];
txtAfter = ['// MATLABCODEGEN: CloseField assign variable in SviClt_Write'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

% Add App Status 
svi_appstatus = itfc_svivar_coupling(status_var, bachmanndict, var_info);
OpeningString = [svi_appstatus];
ClosingString = [];
txtAfter = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,0);

%
PLCType = bachmanndict.mat_to_plc_type(status_var.VarType);
OpeningString = ['MLOCAL ' PLCType ' AppStatus = 0;'];
ClosingString = [];
txtAfter = ['// MATLABCODEGEN: CloseField Output Variable Definition'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);


[AppStatus_addr, foo] = get_parent_sa_addres_var_v2(status_var, bachmanndict, var_info, host);
OpeningString = [AppStatus_addr];
ClosingString = [];
txtAfter = ['// MATLABCODEGEN: CloseField Get ITF SA Address'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);




%
if ~isempty(sm.Variables)

% i select only the variables i want to create
Variables = sm.Variables([sm.Variables.Create]);


% Field1
% OpeningString = ['// MATLABCODEGEN: OpenField SVI Server Definition'];
% ClosingString = ['// MATLABCODEGEN: CloseField SVI Server Definition'];

parent_App = {Variables.parent_App};
ee = cell2mat(cellfun(@(x) ~isempty(x), parent_App, 'Uniformoutput', false));
SviLibToAdd = unique(parent_App( ee) );
SviLibToAdd = cellfun(@(x) host.(x).refC_name, SviLibToAdd, 'Uniformoutput', false); % 20231013 CRS: added to always use the referenceC file as app name 

Field1Text = svi_server_definition_v2(SviLibToAdd);

OpeningString = ['// MATLABCODEGEN: OpenField SVI Server Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Server Definition'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field1Text TXTSource(k2:end)];

% Field6
% 		// MATLABCODEGEN: OpenField Get Specified  App Module
% 		// MATLABCODEGEN: CloseField Get Specified App Module
Field6Text = get_SVI_app_modules(SviLibToAdd);


inp = sm.Variables(strcmp({sm.Variables.IO},'input'));
inp_name = {inp.PortName};
check_input = zeros(size(Name_Inports));
for ii = 1 : length(inp_name)
    idx_in = find(strcmp(Name_Inports, inp_name{ii}) );
    if length(idx_in)==1
        check_input(idx_in) = 1;
    else
        warning('PortName %s of variable %s is not an input of Simulink model %s', inp_name{ii}, inp(ii).TagName, inp(ii).modeltag )
    end

    if length(inp_name{ii}) > 30
        warning('Variable Name %s has more than 31 characters. C++ generation of code limits field name length. This may cause problems. It is suggested to reduce the length of the inputs name', inp_name{ii})
    end
    

end

if sum(check_input) ~= length(Name_Inports)
    if isempty(Name_Inports{1})

    else

    warning('Not all simulink inputs were found in the Submodel definition of %s!', inp(ii).modeltag);
    keyboard
    end
end

% initialize empty fields
Field2Text = [newline];
Field3Text = [newline];
Field4Text = [newline];
Field41Text = [newline];
Field5Text = [newline];
Field31Text = [newline];
Field32Text = [newline];
Field7Text = [newline];
Field8Text = [newline];
Field9Text = [newline];
Field16Text = [newline];
Field17Text = [newline];

var_info = [];
var_info.list_parentvar = [];
var_info.get_sa_addr_parentlist = [];
var_info.assign_sa_addr_parentlist = [];
var_info.assign_sa_addr_parentlist_write = [];

for iVar = 1 : length(Variables)

    var = Variables(iVar);



    if ~var.Create % if the variable should not be created
        continue
    end

    if strcmp(var.IO,'status')
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

    % Field41   
    % // MATLABCODEGEN: OpenField Terminate Variables
    % // MATLABCODEGEN: CloseField Terminate Variables
    f41_txt = terminate_variables_v2(var, var_info);

    %Field 5
    % // MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI
    % // MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI
    f5_txt = assign_parent_var_to_localvar(var, bachmanndict, var_info);

    %Field 31
    % // MATLABCODEGEN: OpenField3 Simulink Model Input Assignment Definition
    % // MATLABCODEGEN: CloseField3 Simulink Model Input Assignment Definition
    f31_txt = assign_input_sl_model_v2(ModelName, var, bachmanndict, var_info, maximum_length_sl_entry);

    %Field 32
    % // MATLABCODEGEN: OpenField Output Variable Assignment
    % // MATLABCODEGEN: CloseField Output Variable Assignment
    f32_txt = assign_output_sl_model_v2(ModelName, var, bachmanndict, var_info, maximum_length_sl_entry);

    %     Field7
    %     // MATLABCODEGEN: OpenField Get ITF SA Address
    %     // MATLABCODEGEN: CloseField Get ITF SA Address
    [f7_txt, var_info] = get_parent_sa_addres_var_v2(var, bachmanndict, var_info, host);

    % Field 8
    %     // MATLABCODEGEN: OpenField assign variable from SA_Address
    %     // MATLABCODEGEN: CloseField assign variable from SA_Address
    % Field 9
    %     // MATLABCODEGEN: OpenField Size of Array definition
    %     // MATLABCODEGEN: CloseField Size of Array definition

    [f9_txt, f8_txt, var_info] = assign_var_from_sa_address_var(var, bachmanndict, var_info, host);

    % Field 16
    % // MATLABCODEGEN: OpenField assign variable in SviClt_Write
    % // MATLABCODEGEN: CloseField assign variable in SviClt_Write
    [f16_txt, var_info] = set_variables_sviclt_v2(var, bachmanndict, var_info, host);

    % Field 17
    % // MATLABCODEGEN: OpenField Assign ITF output Variables to HOST SVI
    % // MATLABCODEGEN: CloseField Assign ITF output Variables to HOST SVI
    f17_txt = assign_localvar_to_parent_var(var, bachmanndict, var_info);



    % final assignments
    Field2Text = [Field2Text f2_txt];
    Field3Text = [Field3Text f3_txt];
    Field4Text = [Field4Text f4_txt];
    Field41Text = [Field41Text f41_txt];
    Field5Text = [Field5Text f5_txt];
    Field31Text = [Field31Text f31_txt];
    Field32Text = [Field32Text f32_txt];
    Field7Text = [Field7Text f7_txt];
    Field8Text = [Field8Text f8_txt];
    Field9Text = [Field9Text f9_txt];
    Field16Text = [Field16Text f16_txt];
    Field17Text = [Field17Text f17_txt];

end


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
k2 = strfind(TXTSource, ClosingString) - 1;
TXTSource = [TXTSource(1:k1) Field4Text TXTSource(k2:end) ];

% Field41
% // MATLABCODEGEN: OpenField Terminate Variables
% // MATLABCODEGEN: CloseField Terminate Variables
OpeningString = ['// MATLABCODEGEN: OpenField Terminate Variables'];
ClosingString = ['// MATLABCODEGEN: CloseField Terminate Variables'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;
TXTSource = [TXTSource(1:k1) Field41Text TXTSource(k2:end) ];

%Field 5 
% // MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI
% // MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI
OpeningString = ['// MATLABCODEGEN: OpenField Assign ITF Variables to HOST SVI'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign ITF Variables to HOST SVI'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field5Text TXTSource(k2:end) ];

% Field31
% // MATLABCODEGEN: OpenField3 Simulink Model Input Assignment Definition
% // MATLABCODEGEN: CloseField3 Simulink Model Input Assignment Definition
OpeningString = ['// MATLABCODEGEN: OpenField3 Simulink Model Input Assignment Definition'];
ClosingString = ['// MATLABCODEGEN: CloseField3 Simulink Model Input Assignment Definition'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;
TXTSource = [TXTSource(1:k1) Field31Text TXTSource(k2:end) ];

% Field 32
% // MATLABCODEGEN: OpenField Output Variable Assignment
% // MATLABCODEGEN: CloseField Output Variable Assignment
OpeningString = ['// MATLABCODEGEN: OpenField Output Variable Assignment'];
ClosingString = ['// MATLABCODEGEN: CloseField Output Variable Assignment'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString) - 1;
TXTSource = [TXTSource(1:k1) Field32Text TXTSource(k2:end) ];

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


end

% Write final _app_file
fid = fopen(NewAppFile,'w');
fprintf(fid, '%s', TXTSource);
fclose(fid);

mkdir(BachmannFolder)

copyfile(NewAppFile, [BachmannFolder '\' AppName '_app.c'])
%

end