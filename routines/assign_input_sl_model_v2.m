function out = assign_input_sl_model_v2(ModelName, var, bachmanndict, info, LimitLengthInputChar)

    
out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if isempty(var.parent_App) % only variables that need connection to parent should be evaluated
    return
end

if ~strcmp(var.IO, 'input') % if the variable is not an output it is not important to reset it
    return
end


InternalInterfaceVarName = info.InternalInterfaceVarName;

if length(var.PortName)<LimitLengthInputChar
%     LimitLengthInputChar = length(InternalInterfaceVarName);
    LimitLengthInputChar = length(var.PortName); % 20231013 CRS: check whether this would work in any situation
end


out = [ModelName '_U.' var.PortName(1:LimitLengthInputChar) ' =  ' InternalInterfaceVarName ';' newline]; % The limit to 31 characters comes from the way simulink generates the c model



end


% Field3Text = [newline newline];
% 
% CountNOI = 0; %Count number of input, to check whether all the inputs of the simulink model have been connected. Not necessary in principle but dangerous
% for iVar = 1 : length(ReadInputVarFromSVI)
% 
%     if ~ReadInputVarFromSVI(iVar)
%         continue;
%     end
% 
%     if InputPortNumber{iVar} == find(strcmp(Name_Inports, VarNamePLC{iVar}) )
%         % That's just a check
%         CountNOI = CountNOI + 1;
%         if length(VarNamePLC{iVar}) > 30
%             warning('Warning: Variable Name %s has more than 31 characters. C++ generation of code limits field name length. This may cause problems. It is suggested to reduce the length of the inputs name', VarNamePLC{iVar})
%         end
% 
% 
%     else
%         fprintf('error: Simulink Input Numbering does not match Excel Output Definition\n');
%         keyboard
%     end
% 
%     if strcmp(InputType{iVar},'REAL32')
%         String = [ModelName '_U.' VarNamePLC{iVar}(1:LimitLengthInputChar(iVar)) ' = (' InputType{iVar} ')' VarNamePLC{iVar} ';' newline]; % The limit to 31 characters comes from the way simulink generates the c model
%         Field3Text = [Field3Text String];
% 
%     elseif strcmp(InputType{iVar},'BOOL8')
%         String = [ModelName '_U.' VarNamePLC{iVar}(1:LimitLengthInputChar(iVar)) ' = (REAL32)' VarNamePLC{iVar} ';' newline]; % The limit to 31 characters comes from the way simulink generates the c model
% 
%         Field3Text = [Field3Text String];
%         warning(['Automatically recasted variable ' VarNamePLC{iVar} ' from BOOL8 into REAL32, make sure Simulink input is REAL32'])
%     else
%         error('Error: Field3 type conversion not supported')
%     end
% 
% end
% 
% if ~isempty(Name_Inports{1})
%     if CountNOI ~= length(Name_Inports)
%         warning('Warning: Number of simulink input connected is NOT EQUAL to total number of simulink input. Check for consistency and proceed with care.')
%         keyboard
%     end
% end
% 
% out_txt = Field3Text;


