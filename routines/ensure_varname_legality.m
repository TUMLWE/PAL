function vn = ensure_varname_legality(VarName)
% this function makes sure that the name of the variables are legal in the
% plc, for example removing leading illegal characters

temp = strtrim(VarName);
    
while ~isempty(regexp(temp(1),'[^a-zA-Z]'))
    temp(1) = [];
end

if ~isempty(temp)
    vn = temp;
else
    error('Illegal variable name for %s', VarName)
end

end
