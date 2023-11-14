function Flag = find_C_ref(wfcfw, sm)

    Flag = false;

    Cfolder = wfcfw.folders.repo.ReferenceCFiles;
    
    if sm.refC_name == string
       return 
    end
    
    
    fn = fullfile(Cfolder, 'Originals', [sm.refC_name '_app.c']);

    if exist(fn) == 2
    
        Flag = true;
           
    else
        
    end

end