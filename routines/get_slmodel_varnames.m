function varnames = get_slmodel_varnames(mdlWks)
% this function returns the variables of a simulink model.
% mdlWks: model workspace of the simulink model




vars = evalin(mdlWks, 'whos');
varnames = cell(size(vars));
for i=1:length(vars)
    varnames{i} = vars(i).name;
end

end
