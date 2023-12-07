function create_empty_project_input(root, project_folder, input_fn)

prj_path = fullfile(root, project_folder);

mkdir(prj_path)
mkdir(fullfile(prj_path, 'ReferenceCFiles'))
mkdir(fullfile(prj_path, 'ReferenceCFiles','Originals'))
mkdir(fullfile(prj_path, 'SimulinkModels'))


%% Create main inputfile
T = table ;  % create empty table 
fname = fullfile(prj_path, input_fn) ;   % here you can give path as well 
writetable(T,fname) ;   % create a excel file with empty table

e = actxserver('Excel.Application');
e.Visible = 0;
Workbook = e.Workbooks.Open(fname);

WS = Workbook.Worksheets;
s1 = get(WS, 'Item', 1);
s1.Name = 'Main Folders';
eActivesheetRange = get(s1,'Range','A1:B1');
eActivesheetRange.Value = {'KeyName', 'KeyValue'};
set(eActivesheetRange.Font, 'Bold', true)

eActivesheetRange = get(s1,'Range','A2:A6');
eActivesheetRange.Value = {
    'PLCApps'
    'InterfaceFiles'
    'ReferenceCFiles'
    'SimulinkCCode'
    'SimulinkModels'
%     'MatlabRoutines_PLCGenerators'
    };

eActivesheetRange = get(s1,'Range','B2:B6');
eActivesheetRange.Value = {
    fullfile(prj_path,'PLCApps')
    'PLC_Matlab_InterfaceRoutines'
    fullfile(prj_path,'ReferenceCFiles')
    fullfile(prj_path,'SimulinkCCodes')
    fullfile(prj_path,'SimulinkModels')
    };


s2 = Add(WS, [], s1);
s2.Name = 'Submodels';
eActivesheetRange = get(s2,'Range','A1:I1');
eActivesheetRange.Value = {
    'modelTag'
    'PLCApp_folder'
    'slmodel_folder'
    'slmodel_name'
    'slccode_folder'
    'refC_name'
    'Flag_Generate_PLC_app'
    'host_apptag'
    'Comment'    
    }';
set(eActivesheetRange.Font, 'Bold', true)



s3 = Add(WS, [], s2);
s3.Name = 'ITFC';
eActivesheetRange = get(s3,'Range','A1:G1');
eActivesheetRange.Value = {
    'appTag'
    'PLCApp_folder'
    'refC_name'
    'Flag_Generate_PLC_app'
    'Flag_Create_test_ITFC'
    'test_ITFC_filename'
    'Comment'    
    }';
set(eActivesheetRange.Font, 'Bold', true)


s4 = Add(WS, [], s3);
s4.Name = 'HOST';
eActivesheetRange = get(s4,'Range','A1:K1');
eActivesheetRange.Value = {
    'appTag'
    'PLCApp_folder'
    'refC_name'
    'Flag_Generate_PLC_app'
    'FAST_FREQ_RATIO'
    'SLOW_FREQ_RATIO'
    'CTRL_FREQ_RATIO'
    'out_filename'
    'output_path_in_PLC'
    'output_file_duration_s'
    'Comment'    
    }';
set(eActivesheetRange.Font, 'Bold', true)


s5 = Add(WS, [], s4);
s5.Name = 'Settings';
eActivesheetRange = get(s5,'Range','A1:C1');
eActivesheetRange.Value = {
    'KeyName'
    'KeyValue'
    'Comment'
    }';
set(eActivesheetRange.Font, 'Bold', true)

eActivesheetRange = get(s5,'Range','A2:A8');
eActivesheetRange.Value = {
    'ExcelPLCFileName'
%     'ExcelIOFileName'
    'FlagGenerate_ITFC'
    'FlagGenerate_PLCApps'
    'SaveModelParametersFlag'
%     'duration'
    'sample_time'
    'PLC_system'
    'DataTypes_dict_filename'
    };

eActivesheetRange = get(s5,'Range','B2:B8');
eActivesheetRange.Value = {
    fullfile(prj_path,'SVI_definition.xlsx')
    'true'
    'true'
    'true'
    '0.1'
    'Bachmann'
    'dictionary_bachmann.xlsx'
    };

invoke(Workbook,'Save')
invoke(e,'Quit')

%% Create SVI_Definition.xlsx

T = table ;  % create empty table 
fname = fullfile(prj_path, 'SVI_Definition.xlsx') ;   % here you can give path as well 
writetable(T,fname) ;   % create a excel file with empty table

e = actxserver('Excel.Application');
e.Visible = 1;
Workbook = e.Workbooks.Open(fname);

WS = Workbook.Worksheets;
s1 = get(WS, 'Item', 1);
s1.Name = 'ITFC';
eActivesheetRange = get(s1,'Range','A1:H1');
eActivesheetRange.Value = {
    'InputNumber'
    'TagName'
    'VarName'
    'AppName'
    'VarType'
    'VarSize'
    'Access'
    'Create'}';
set(eActivesheetRange.Font, 'Bold', true)

s2 = Add(WS, [], s1);
s2.Name = 'HOST';
eActivesheetRange = get(s2,'Range','A1:P1');
eActivesheetRange.Value = {
    'TagName'
    'VarName'
    'AppName'
    'VarType'
    'VarSize'
    'Create'
    'parent_App'
    'parent_TagName'
    'parent_SubVar'
    'Action'
    'Flag_Assign_InitialValue'
    'Initial_Value'
    'Units'
    'Print_output'
    'output_freq'
    'Comments'
    }';
set(eActivesheetRange.Font, 'Bold', true)

s3 = Add(WS, [], s2);
s3.Name = 'Submodels';
eActivesheetRange = get(s3,'Range','A1:K1');
eActivesheetRange.Value = {
    'modelTag'
    'TagName'
    'VarName'
    'IO'
    'PortNumber'
    'PortName'
    'Create'
    'VarType'
    'parent_App'
    'parent_TagName'
    'Action'
%     'Flag_Assign_InitialValue'
%     'Initial_Value'
    }';
set(eActivesheetRange.Font, 'Bold', true)

invoke(Workbook,'Save')
invoke(e,'Quit')


end