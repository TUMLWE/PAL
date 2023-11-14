function [fast_fields, slow_fields, ctrl_fields] = label_names_output_fields(var, fast_fields, slow_fields, ctrl_fields)


if ~var.Create % if the variable should not be created
    return
end

if ~var.Print_output % if the variable should not be printed to output
    return
end

if isnan(var.Units)
    warning('units of measure of variable %s is a NaN. Please correct it.', var.TagName)
    keyboard
end



switch var.output_freq

    case 'fast'
        fast_fields = [fast_fields strtrim(var.TagName) var.Units '; '];    
    case 'slow'
        slow_fields = [slow_fields strtrim(var.TagName) var.Units '; '];    

    case 'ctrl'
        ctrl_fields = [ctrl_fields strtrim(var.TagName) var.Units '; '];    

    otherwise

        error('output_freq of variable %s can only be fast, slow or ctrl.', var.TagName);

end



end
