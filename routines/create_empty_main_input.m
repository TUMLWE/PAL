function create_empty_main_input(filename)
% this routine creates an empty excel input file

% write Main folders
data = {'VariableName' 'FolderName'};
writecell(data,filename,'Sheet','Main folders')

data = {'BachmannMainFolder' 	        'BachmannApp'               
'InterfaceFilesFolder'	        'PLC_Matlab_InterfaceRoutines'
'ReferenceCFiles'            'ReferenceCFiles'
'SimulinkCCodeMainFolder'	    'SimulinkCCodes'
'SimulinkModelsMainFolder'	    'SimulinkModels'
'MatlabRoutines_PLCGenerators'	'MatlabRoutines_PLCGenerators'};
writecell(data,filename,'Sheet','Main folders','Range','A2')

% write Submodels

data = {'modeltag' 'bachmannApp' 'slmodel' 'slmodelName' 'slccode'...
     'refCfiles' 'Flag_Generate_PLC_app' 'host_app' 'Comment'};
writecell(data,filename,'Sheet','Submodels')

% write settings
data = {'Variable' 'Value' 'Comment'};
writecell(data,filename,'Sheet','Settings')

data = {'Variable' 'Value' 'Comment'};
writecell(data,filename,'Sheet','Settings')


end