function out_txt = get_SVI_app_modules(SviLibToAdd)

Field6Text = [newline];

for iVar = 1 : length(SviLibToAdd)

    String = ['pSviLib_' SviLibToAdd{iVar} ' = svi_GetLib("' SviLibToAdd{iVar} '");' newline ...
        'if (!pSviLib_' SviLibToAdd{iVar} ')' newline...
        '{' newline...
        '   LOG_W(0, Func, "Could not get SVI of module ' SviLibToAdd{iVar} '!");' newline ...
        '   return (ERROR);' newline ...
        '}' newline newline ...
        ];

    Field6Text = [Field6Text String];

end

out_txt = Field6Text;

end