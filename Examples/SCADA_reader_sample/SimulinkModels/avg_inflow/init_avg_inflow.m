function init_avg_inflow(params_filename)%this is the initialization file for the avg_inflow Simulink model, here you can specify constants and variables necessary for running and compiling your model 

load(params_filename)
% Variable definition


StartupTime = 6; %seconds,  Before this time, the application will flag error state
MovAvTI = [6]; % Moving Average In Seconds for Inflow quantities
MovAvErrorTime = [6]; % Moving Average to check for errors in inflow quantities

ParInflow_TUM.MovAvTI = round(MovAvTI*1/sample_time);
ParInflow_TUM.StartupTime = round(StartupTime*1/sample_time);
ParInflow_TUM.MovAvErrorCheck = round(MovAvErrorTime*1/sample_time);

% Sect 2: assignments. Here all the defined variables that are useful for running the model are assigned to the parameter file
save(params_filename,'ParInflow_TUM','-append')

end 
