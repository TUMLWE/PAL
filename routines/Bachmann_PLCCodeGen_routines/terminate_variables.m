function out_txt = terminate_variables(Connect, OutputType, OutputPortname)

Field96Text = [newline newline];
String = [];

for iVar = 1 : length(Connect)

  if Connect(iVar)  
    
   if strcmp(OutputType{iVar}, 'STRING') 
                     
       fprintf('Variable Type to be included in App not yet implemented')
       keyboard

   elseif strcmp(OutputType{iVar}, 'REAL32')  
       
        String = [OutputPortname{iVar} ' = 0.0;' newline];
    
   elseif strcmp(OutputType{iVar}, 'SINT16') || strcmp(OutputType{iVar}, 'BOOL8')
       
       fprintf('Variable Type to be included in App not yet implemented')
       keyboard
       
   else
       
       fprintf('Variable Type to be included in App not yet implemented')
       keyboard
      
   end
   
      Field96Text = [Field96Text String];
 
  end
end

out_txt = Field96Text;
end