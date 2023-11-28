function init_WF_status(params_filename)%this is the initialization file for the WF_status Simulink model, here you can specify constants and variables necessary for running and compiling your model 

load(params_filename)
% Variable definition


WF_status_param.MaxWS = 10; 

WF_status_param.MaxWD = 260;
WF_status_param.MinWD = 180;

WF_status_param.PowProd_State = 1; % Controller state for which the turbine is in power production

% Sect 2: assignments. Here all the defined variables that are useful for running the model are assigned to the parameter file
save(params_filename,'WF_status_param','-append')  

end 
