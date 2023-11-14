function [head_labels, g, itemfn] = initialize_table(app, TableType, wfcfw)


switch TableType

    case 'ITFC'

        if isempty(wfcfw.itfc)
            itemfn = [];
        else
            itemfn = fieldnames(wfcfw.itfc);
        end

        parent = app.InterfacePanel;
        ph = app.Data.Settings.itfc_UIPanelRows_height;
        head_labels = app.Data.Settings.itfc_header_labels;

    case 'submodels'

            if isempty(wfcfw.sub_models)
                itemfn = [];
            else
                itemfn = fieldnames(wfcfw.sub_models);            
            end            


        parent = app.SubmodelsPanel;
        ph = app.Data.Settings.UIPanelRows_height;
        head_labels = app.Data.Settings.header_labels;

    case 'host'

        if isempty(wfcfw.host)
            itemfn = [];
        else
            itemfn = fieldnames(wfcfw.host);
        end

        parent = app.HOSTPanel;
        ph = app.Data.Settings.host_UIPanelRows_height;
        head_labels = app.Data.Settings.host_header_labels;


end

g = uigridlayout(parent,'Scrollable','on');
g.ColumnWidth = repmat({10},[1, length(head_labels)]); % initial value
g.RowHeight = num2cell(repmat(ph,[1,length(itemfn)+1]));
g.Padding = [0 0 0 0];
g.ColumnSpacing = 0;
g.RowSpacing = 0;

createHeader(app, g, head_labels);



end