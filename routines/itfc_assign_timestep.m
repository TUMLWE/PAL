function out = itfc_assign_timestep(var, bachmanndict, info)


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
        String = [];

        %         StructName = info.StructName;

        for iSubfield = 1 : length(var.SV)

            subvar = var.SV{iSubfield};

            if ~subvar.Create % if the variable should not be created
                continue
            end

            %     time_hist = subvar.time_hist;

            switch subvar.VarType

                case 'char'
                    String = [String 'strcpy(' InternalInterfaceVarName '.' subvar.VarName ',' ...
                        InternalInterfaceVarName '_TH[CycleCount].' subvar.VarName ');' newline];

                case {'double', 'int16', 'logical', 'single' 'uint16'}

                    numberoflements = subvar.VarSize;
                    for inoe = 1 : numberoflements

                        String = [String InternalInterfaceVarName '.' subvar.VarName '[' num2str(inoe-1) ']' '=' ...
                        InternalInterfaceVarName '_TH[CycleCount].' subvar.VarName '[' num2str(inoe-1) ']' ';' newline];

                    end

                otherwise

                    fprintf('Subfield Type not yet implemented \n')
                    keyboard

            end

        end

        out = [out String];

    case 'double'

        numberoflements = var.VarSize;
        Access = var.Access;
        %         PLCType = bachmanndict.mat_to_plc_type(var.VarType);
        %         Field4Text = [Field4Text String newline  ];

        if strcmp(Access, 'READ')
            VN_TH = info.VN_TH;

            String = ['for (iC = 0 ; iC < ' num2str(numberoflements) '; iC++)' newline ...
                '{' newline ...
                '    ' InternalInterfaceVarName '[iC] = ' VN_TH '[iC][CycleCount];' newline ...
                '}' ];

            out = [out String newline];


        elseif strcmp(Access, 'WRITE')

% %             String = ['for (iC = 0 ; iC < ' num2str(numberoflements) '; iC++)' newline ...
% %                 '{' newline ...
% %                 InternalInterfaceVarName '[iC] = 0.0;' newline ...
% %                 '}' ];
% % 
% %             out = [out String newline];
% % 
% %             %             Field99Text = [Field99Text String newline ];

        else
            error(['Error when writing variable "' var.TagName '" in app "' var.AppName '": data type should be either READ or WRITE'])
        end

    otherwise
        warning('Data type not yet implemented, please check.')
        keyboard


end



end




