function create_matlabcodegen_fields_itfc(SourceRefPath, SourceRefFileName, TargetFolder)
% This routine modifies a _app.c bachmann file, adding the necessary lines
% for the SVI interconnection and internal variables definition.


[is, ie] = regexp(SourceRefFileName, '.*_app.c');

if isempty(is)
   
    error('The selected source file name does not end with "_app.c". Please correct it.');
    
end

% AppName = SourceRefFileName(1:end-6);

%%
TXTSource = fileread(fullfile(SourceRefPath, SourceRefFileName));


OpeningString = ['// MATLABCODEGEN: OpenField define and declare SVI inputs'];
ClosingString = ['// MATLABCODEGEN: CloseField define and declare SVI inputs'];
txtBefore = ['/* Global variables: miscellaneous */'];
TXTSource = addfield(1,txtBefore, OpeningString, ClosingString, TXTSource,1);

OpeningString = ['// MATLABCODEGEN: OpenField SVI Variables Coupling'];
ClosingString = ['// MATLABCODEGEN: CloseField SVI Variables Coupling'];
txtIn = ['(?<=MLOCAL SVI_GLOBVAR SviGlobVarList.*)};'];
TXTSource = addfield(3, txtIn, OpeningString, ClosingString, TXTSource,1);

% Add comma to SVIGlobVarList, in preparation to the other variables to add
OpeningString = [','];
ClosingString = [];
txtIn = ['(?!{"ModuleVersion"[^{}]*})[\s]*// MATLABCODEGEN: OpenField SVI Variables Coupling'];
TXTSource = addfield(3, txtIn, OpeningString, ClosingString, TXTSource,0);

OpeningString = ['// MATLABCODEGEN: OpenField define variable Time Histories'];
ClosingString = ['// MATLABCODEGEN: CloseField define variable Time Histories'];
txtAfter = ['// MATLABCODEGEN: CloseField define and declare SVI inputs'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

OpeningString = ['// MATLABCODEGEN: OpenField Initializing Variables Time History'];
ClosingString = ['// MATLABCODEGEN: CloseField Initializing Variables Time History'];
txtAfter = ['/* TODO: add what is necessary before cyclic operation starts */'];
TXTSource = addfield(0,txtAfter, OpeningString, ClosingString, TXTSource,1);

OpeningString = ['// MATLABCODEGEN: OpenField Assign TimeStep'];
ClosingString = ['// MATLABCODEGEN: CloseField Assign TimeStep'];
txtAfter = ['MLOCAL VOID Control_CycleStart\(VOID\)\s*{'];
TXTSource = addfield(4,txtAfter, OpeningString, ClosingString, TXTSource,1);


outfile = fullfile(TargetFolder, SourceRefFileName);
fid = fopen(outfile,'w');
fprintf(fid, '%s', TXTSource);
fclose(fid);



% % % %% I believe this isn't necessary anymore
% % % OpeningString = ['// MATLABCODEGEN: OpenField Initialize Output Variable'];
% % % ClosingString = ['// MATLABCODEGEN: CloseField Initialize Output Variable'];



end