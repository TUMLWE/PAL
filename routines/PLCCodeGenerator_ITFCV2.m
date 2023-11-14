function PLCCodeGenerator_ITFCV2(AppName, BachmannFolder, ...
    ReferenceCFiles, itfc, bachmanndict)

% Generate ITFC Application

%%
% Create copy of the original source in the reference file folder
OrigAppFile = [ReferenceCFiles '\' [AppName] '_app.c'];
NewAppFile = [ReferenceCFiles '\NEW_' [AppName] '_app.c'];
copyfile(OrigAppFile, NewAppFile)
TXTSource = fileread(NewAppFile);


% initialize empty fields
Field1Text = [newline];
Field2Text = [newline];
Field3Text = [newline];
Field4Text = [newline 'int iC;' newline];
Field5Text = [newline];
% Field99Text = [newline];


% idxVarToInclude = find(PLCVarAvailable==1 & strcmp(PLCVarApplication,'TUMITFCT'));

for iVar = 1 : length(itfc.Variables)

    var = itfc.Variables(iVar);

    if ~var.Create % if the variable should not be created
        continue
    end


    % OpeningString = ['// MATLABCODEGEN: OpenField define and declare SVI inputs'];
    % ClosingString = ['// MATLABCODEGEN: CloseField define and declare SVI inputs'];
    [f1_txt, info] = itfc_define_declare_svi_inputs(var, bachmanndict,0);

    % OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
    % ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
    f2_txt = itfc_svivar_coupling(var, bachmanndict, info);

    % OpeningString = ['// MATLABCODEGEN: OpenField define variable Time Histories'];
    % ClosingString = ['// MATLABCODEGEN: CloseField define variable Time Histories'];
    [f3_txt, info] = itfc_define_time_histories(var, bachmanndict, info);

    % OpeningString = ['// MATLABCODEGEN: OpenField Initializing Variables Time History'];
    % ClosingString = ['// MATLABCODEGEN: CloseField Initializing Variables Time History'];
    f5_txt = itfc_initialize_time_histories(var, bachmanndict, info);

    % OpeningString = ['// MATLABCODEGEN: OpenField Assign TimeStep'];
    % ClosingString = ['// MATLABCODEGEN: CloseField Assign TimeStep'];
    f4_txt = itfc_assign_timestep(var, bachmanndict, info);

    Field1Text = [Field1Text f1_txt];
    Field2Text = [Field2Text f2_txt];
    Field3Text = [Field3Text f3_txt];
    Field4Text = [Field4Text f4_txt];
    Field5Text = [Field5Text f5_txt];
%     Field99Text = [Field99Text newline];

end

% Field1 
OpeningString = ['// MATLABCODEGEN: OpenField define and declare SVI inputs'];
ClosingString = ['// MATLABCODEGEN: CloseField define and declare SVI inputs'];
k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;

TXTSource = [TXTSource(1:k1) Field1Text TXTSource(k2:end)];

%Field 2
OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;

TXTSource = [TXTSource(1:k1) Field2Text TXTSource(k2:end) ];

%Field3
OpeningString = ['// MATLABCODEGEN: OpenField define variable Time Histories'];
ClosingString = ['// MATLABCODEGEN: CloseField define variable Time Histories'];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field3Text TXTSource(k2:end) ];

% %Field5
OpeningString = ['// MATLABCODEGEN: OpenField Initializing Variables Time History'];
ClosingString = ['// MATLABCODEGEN: CloseField Initializing Variables Time History'];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field5Text TXTSource(k2:end) ];

%Field4
OpeningString = ['// MATLABCODEGEN: OpenField Assign TimeStep'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign TimeStep'];

k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
k2 = strfind(TXTSource, ClosingString)-1;
TXTSource = [TXTSource(1:k1) Field4Text TXTSource(k2:end) ];
% 
% %Field99
% OpeningString = ['// MATLABCODEGEN: OpenField Initialize Output Variable'];
% ClosingString = ['// MATLABCODEGEN: CloseField Initialize Output Variable'];
% 
% k1 = strfind(TXTSource, OpeningString) + length(OpeningString);
% k2 = strfind(TXTSource, ClosingString)-1;
% TXTSource = [TXTSource(1:k1) Field99Text TXTSource(k2:end) ];

% Write final _app_file
fid = fopen(NewAppFile,'w');
fprintf(fid, '%s', TXTSource);
fclose(fid);

mkdir(BachmannFolder)

copyfile(NewAppFile, [BachmannFolder '\' AppName '_app.c'])

%%
end
