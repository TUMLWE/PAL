function out = read_outfile_host(fn, happ, file_type)

vars_to_consider = cell2mat(cellfun(@(x) x == 1, {happ.Variables.Create} ,'UniformOutput',false))';
vars_type = cell2mat(cellfun(@(x) strcmpi(x, file_type), {happ.Variables.output_freq} ,'UniformOutput',false))';


varlabellist = cellfun(@(x,y) [x y ], {happ.Variables(vars_to_consider & vars_type).TagName}, {happ.Variables(vars_to_consider & vars_type).Units} ,'UniformOutput',false)';
varlist = happ.Variables(vars_to_consider & vars_type);

fid = fopen(fn,'r');
if fid==-1
    error('Cannot open file %s', fn)
end

% the first line is header
header_txt = fgetl(fid);
header = strtrim(strsplit(strtrim(header_txt),';'));

if isempty(header{end})
header = header(1:end-1);
end

entry_found = false(length(varlabellist), 1); % this is used to check that all necessary variables are present in the output file

header_string = {'%s'};
varTypes = {'string'};
for ih = 2 : length(header) % the first is the timestamp
    labelfile = header{ih};


    idx_found = find(strcmp(varlabellist, labelfile));
    if length(idx_found)~=1
            warning('Found variable %s in output file, but this could not be paired to any variable in host app %s in output files. Please continue at your own risk.', labelfile, happ.appTag)
            keyboard
    else
       entry_found(strcmp(varlabellist, labelfile)) = true; 
    end

    var = varlist(idx_found);

    type = var.VarType;
    switch type

        case 'char'
            header_string = [header_string; '%s'];
            varTypes = [varTypes; 'string']; 
            
        case {'double' 'single'}
            header_string = [header_string; '%f'];
            varTypes = [varTypes; type]; 

        case {'int16' 'uint16' 'logical'}
            header_string = [header_string; '%d'];
            varTypes = [varTypes; type]; 
            
        otherwise
            warning('Data type for header not yet implemented');
            keyboard
    end


end

if sum(entry_found) == length(entry_found)
    fprintf('All headers of file "%s" could be linked to HOST application\n', fn)
else
    warning('Not all headers of file "%s" could be linked to HOST application! Please proceed with caution.\n', fn)
    keyboard
end

out = table('Size',[0 length(header)],'VariableTypes',varTypes,'VariableNames',header);

lineCount = 0;
while ~feof(fid)
    lineCount = lineCount+1;

    line = fgetl(fid);

    raw_values = strtrim(strsplit(line,';'))';
    raw_values = raw_values(1:end-1); % this removes the trailing whitespace

    try % if there is a reading error then we skip the line
        values = cellfun(@(x,y) textscan(x,y,'whitespace', ''), raw_values, header_string,'UniformOutput',false);
        values = cellfun(@(x)  x{1}, values,'UniformOutput',false);
        
        out(end+1,:) = values';

    catch ME

        checkempty = cell2mat(cellfun(@(x) isempty(x), raw_values,'UniformOutput', false));
        if sum(checkempty)~=0
            iEntry = find(checkempty);
            warning('Found empty entry in file "%s", \n at line %d (%s). Line skipped. \n', fn, lineCount, header{iEntry})
            fprintf("%s\n", line)
%             keyboard

        end

    end

end

infmt = "yyyy.MM.dd_HH:mm:ss:SSS";
out.TimeStamp_UTC = datetime(out.TimeStamp_UTC,"InputFormat",infmt,"Format","yyyy.MM.dd HH:mm:ss.SSS");



fclose(fid);

end
