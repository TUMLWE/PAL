function out = recastToType(vn, value, input_type)
 
% classes_vec = {'double'; 'logical'; 'char'; 'cell'};
% input_vec = {'string'; 'logical'; 'double'}

if strcmp(input_type,'string') && isa(value,'double')
            if isnan(value)
                out = string;
            elseif isempty(value)
                out = string;                
            else
            warning(['Cannot assign ' vn '. Input not properly specified'])
            end
elseif strcmp(input_type,'string') && isa(value,'logical') 
            warning(['Cannot assign ' vn '. Input not properly specified']) 
elseif strcmp(input_type,'string') && isa(value,'char') 
            out = value;
elseif strcmp(input_type,'string') && isa(value,'string') 
            out = value;
elseif strcmp(input_type,'string') && isa(value,'cell') 
            out = recastToType(vn, value{1}, input_type);
            
elseif strcmp(input_type,'logical') && isa(value,'double') 
    
                    if value~=0 && value~=1
                        warning(['Cannot assign ' vn '. Input not properly specified. Please correct it.'])
                    else
                        out = logical(value);
                    end
                    
elseif strcmp(input_type,'logical') && isa(value,'logical') 
            out = value;
elseif strcmp(input_type,'logical') && isa(value,'char')
            out = recastToType(vn,  str2num(value), input_type);    
    
elseif strcmp(input_type,'logical') && isa(value,'cell')
                out = recastToType(vn, value{1}, input_type);
                
elseif strcmp(input_type,'double') && isa(value,'double')
                out = value;

elseif strcmp(input_type,'double') && isa(value,'logical')
                out = double(value);

elseif strcmp(input_type,'double') && isa(value,'char')
                if isempty(value)
                   warning(['Cannot assign ' vn '. Input not properly specified'])
                else
                    out = str2num(value);
                end

elseif strcmp(input_type,'double') && isa(value,'string')
                if isempty(value)
                   warning(['Cannot assign ' vn '. Input not properly specified'])
                else
                    out = str2num(value);
                end

elseif strcmp(input_type,'char') && isa(value,'char')
                out = value;

elseif strcmp(input_type,'int16') && isa(value,'double')
                out = cast(value,'int16');

elseif iscell(value) && ~isempty(value{1})
        out = recastToType(vn,  value{1}, input_type); 

elseif strcmp(input_type,'single') && isa(value,'double')
        out = cast(value,'single');


else
   keyboard 

end
    


            
        
end