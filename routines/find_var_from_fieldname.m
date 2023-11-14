function outvar = find_var_from_fieldname(list, fieldname, fieldval)
   outvar = {}; 

for ivar = 1 : length(list)
    var = list{ivar};

    if strcmp(var.(fieldname),fieldval)
        outvar = var;
    end

end
    

end
