classdef plc_dictionary < handle

    properties

        Types_dict

    end

    methods

        function obj = plc_dictionary(folder, main)

            table = readtable(fullfile(folder,main),'Sheet','Types');
            obj.Types_dict = table;

        end

        function sz = size_of_mat_type(obj, mattype)

            idx = find(strcmp(obj.Types_dict.matlab, mattype));

            if length(idx)~=1
                error('Error in dictionary: expected a unique %s entry, please correct it.', mattype)
            end
            
            sz = obj.Types_dict.Bytes(idx);

        end

        function plctype = mat_to_plc_type(obj, mattype)
            idx = find(strcmp(obj.Types_dict.matlab, mattype));
            plctype = obj.Types_dict.PLC{idx};
        end
                


    end
end