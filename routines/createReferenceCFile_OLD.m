%% Create reference C file

SourceRefPath = 'D:\Work\PhD\Codes\GitLab\wfc-framework\ReferenceCFiles\Originals'; % this should be the folder where you put the original bachmann source file
SourceRefFileName = 'delete_app.c';  % name of the original reference file

TargetFolder = 'D:\Work\PhD\Codes\GitLab\wfc-framework\ReferenceCFiles'; % target folder for the modified reference file

HostName = 'ENOHOST'; % name of the host app


%% create new file (20230216 - CarloSucameli: so far the code supports only Bachmann systems)
create_matlabcodegen_fields(SourceRefPath, SourceRefFileName, TargetFolder , HostName)


