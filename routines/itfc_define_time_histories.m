function [out, info] = itfc_define_time_histories(var, bachmanndict, info)


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

        StructName = info.StructName;
        nTimeInst = size(var.SV{1}.time_hist,1);
        String = ['struct ' StructName ' ' InternalInterfaceVarName '_TH[' num2str(nTimeInst) '];' newline ];
 
        out = [out String ];


    case 'double'

        numberoflements = var.VarSize;
        PLCType = bachmanndict.mat_to_plc_type(var.VarType);


        if strcmp(var.Access,'READ')
            VN_TH = [InternalInterfaceVarName '_TH'];
            info.VN_TH = VN_TH;
            %             VN_Data = zeros(length(NonEmptyIndices),nTimeInst) ;
            VN_Data = var.time_hist;
            nTimeInst = size(VN_Data,1);

            String = [PLCType ' ' VN_TH '[' num2str(numberoflements) '][' num2str(nTimeInst) '] = { 0 };' newline ...
                ];

        out = [out String  ];

        end



    otherwise
        warning('Data type not yet implemented, please check.')
        keyboard


end


end
