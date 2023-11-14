function out = assign_output_sl_model_v2(ModelName, var, bachmanndict, info, LimitLengthInputChar)

    
out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end

if ~strcmp(var.IO, 'output') % if the variable is not an output it is not important to reset it
    return
end


InternalInterfaceVarName = info.InternalInterfaceVarName;

if length(var.PortName)<LimitLengthInputChar
%     LimitLengthInputChar = length(InternalInterfaceVarName);
    LimitLengthInputChar = length(var.PortName); % 20231013 CRS: check whether this would work in any situation
end

if strcmp(var.VarType, 'STRING')
warning('Variable Type to be included in App not yet implemented (string)')
keyboard
end

out = [InternalInterfaceVarName ' =  ' ModelName '_Y.' var.PortName(1:LimitLengthInputChar) ';' newline]; % The limit to 31 characters comes from the way simulink generates the c model

end