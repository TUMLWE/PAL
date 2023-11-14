classdef submodel_Flags < handle
    
    properties
        SLmodel_Ready {mustBeNumericOrLogical} = false
        PLC_codeGen_Ready {mustBeNumericOrLogical} = false
        C_ref_found {mustBeNumericOrLogical} = false
        C_ref_codegen_found {mustBeNumericOrLogical} = false
        can_create_sl_model {mustBeNumericOrLogical} = false
        isDefined {mustBeNumericOrLogical} = false

    end
    
    methods
        
        function obj = submodel_Flags(obj)
            
        end
        
    end
end