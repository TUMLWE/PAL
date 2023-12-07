function resizeTableColumn(app, item, parent, iCol)
% this function was created because the fit property of
% UIgridlayout didn't work properly, and could not resize cells
% according to the string size

if strcmp(class(item),'matlab.ui.control.EditField')
    kk = 1.2;

    f =figure('Visible','off');
    cc = uicontrol(f,'Visible','off','style','text');
    cc.FontUnits = "pixels";
    cc.FontName = item.FontName;
    cc.FontWeight = item.FontWeight;
    cc.FontSize = item.FontSize;
    cc.FontAngle = item.FontAngle;
    cc.String = item.Value;
    Ext = get(cc, 'Extent');
    
    if Ext(3)*kk > parent.ColumnWidth{iCol}
        parent.ColumnWidth{iCol} = Ext(3)*kk;
    else
    end
  
    
elseif strcmp(class(item),'matlab.ui.control.CheckBox')
    
else
    
end


end
