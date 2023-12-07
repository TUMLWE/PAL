function out = itfc_initialize_time_histories(var, bachmanndict, info)
out = [];

% VarName = var.VarName;

if ~var.Create % if the variable should not be created
    return
end

if strcmpi(var.Access, 'write') % if variable is to be written, we don't need to initialize and define it
    return
end

InternalInterfaceVarName = info.InternalInterfaceVarName;

switch var.VarType

    case 'struct'

%         StructName = info.StructName;
        nTimeInst = size(var.SV{1}.time_hist,1);
%         String = ['struct ' StructName ' ' InternalInterfaceVarName '_TH[' num2str(nTimeInst) '];' newline ];
        String = [];
        for iSubfield = 1 : length(var.SV)
            subvar = var.SV{iSubfield};

            if ~subvar.Create % if the variable should not be created
                continue
            end

            time_hist = subvar.time_hist;

            for iTime = 0 : nTimeInst-1
                switch subvar.VarType

                    case 'char'
                        String = [String 'strcpy(' InternalInterfaceVarName '_TH[' num2str(iTime) '].' subvar.VarName ', "' time_hist(iTime+1,:) '");' newline ];

                    case {'double', 'int16', 'logical', 'single' 'uint16'}
                        
                        for issv = 0 : length(subvar.VarSize)-1
                        String = [String  InternalInterfaceVarName '_TH[' num2str(iTime) '].' subvar.VarName '[' num2str(issv) ']=' num2str(time_hist(iTime + 1, issv+1)) ';' newline];
                        end

                    otherwise

                        fprintf('Subfield Type not yet implemented \n')
                        keyboard

                end
            end

        end  
        out = [out String newline  ];


    case 'double'

        numberoflements = var.VarSize;
        PLCType = bachmanndict.mat_to_plc_type(var.VarType);


        if strcmp(var.Access,'READ')
            VN_TH = [InternalInterfaceVarName '_TH'];
            info.VN_TH = VN_TH;
            %             VN_Data = zeros(length(NonEmptyIndices),nTimeInst) ;
            VN_Data = var.time_hist;
            nTimeInst = size(VN_Data,1);
            String = [];


            for iTime = 0 : nTimeInst - 1
                %                 iTime
                for iElem = 0 : numberoflements - 1
                    String = [String VN_TH '[' num2str(iElem) '][' num2str(iTime) ']=' num2str(VN_Data(iTime+1 ,iElem+1)) ';' newline];
                end
            end
        out = [out String newline  ];

        end



    otherwise
        warning('Data type not yet implemented, please check.')
        keyboard


end


end
