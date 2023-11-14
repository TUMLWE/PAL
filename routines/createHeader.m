function createHeader(app, parent, header_labels)

% header_labels = app.Data.Settings.header_labels;
for ih = 1 : length(header_labels)
    
    
    edt = uieditfield(parent);
    edt.Layout.Row = 1;
    edt.Layout.Column = ih;
    edt.Editable = false;
    edt.FontWeight = 'bold';
    edt.BackgroundColor = [0.9 0.9 0.9];
    edt.Value = header_labels{ih};
    resizeTableColumn(app, edt, parent, ih)
    
end

end