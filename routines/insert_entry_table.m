function out_obj = insert_entry_table(app, Type, iCol, g, iitem, varargin)


switch Type

    case 'tag'

        if nargin ~=6
            error('Wrong number of input arguments to tag definition')
        else
            tag = varargin{1};

        end

        edt = uieditfield(g);
        edt.Layout.Row = iitem+1;
        edt.Layout.Column = iCol;
        edt.Editable = false;
        edt.Value = tag;

        resizeTableColumn(app, edt, g, edt.Layout.Column)
        out_obj = edt;

    case 'Generate PLC'

        if nargin ~=9
            error('Wrong number of input arguments to tag definition')
        else
            Flag_Generate_PLC_app = varargin{1};
            EnableCheckBox_PLC_app = varargin{2};
            itemclass = varargin{3};
            tag =  varargin{4};
        end

        g2 = uigridlayout(g);
        g2.Layout.Row = iitem+1;
        g2.Layout.Column = iCol;

        g2.ColumnWidth = {'fit'};
        g2.RowHeight = {'fit'};

        cb = uicheckbox(g2,'ValueChangedFcn',{@app.cBoxChanged_GenPLC, tag, itemclass});
        ww_temp = cb.Position(3);
        hh_temp = cb.Position(4);
        cb.Text = '';
        cb.Layout.Row = 1;
        cb.Layout.Column = 1;
        cb.Value = Flag_Generate_PLC_app;
        cb.Enable = EnableCheckBox_PLC_app;

        % this loop is necessary because there is a latency between
        % the predefined box size and its updated value
        while cb.Position(3)==ww_temp || cb.Position(4)==hh_temp
            pause(0.1)  %App designer must be bugged, because if i don't insert this pause the positions are not updated
            ww = cb.Position(3);
            hh = cb.Position(4);
        end

        % ww = 16;
        % hh = 16;

        g2.Padding = [g.ColumnWidth{2}/2 - ww/2, 0, 0, g.RowHeight{iitem+1}/2 - hh/2];
        out_obj = cb;

    case 'C ref found'
        if nargin ~=8
            error('Wrong number of input arguments to tag definition')
        else
            Flag = varargin{1};
            itemclass = varargin{2};
            tag =  varargin{3};

        end

        lmp = uilamp(g);
        lmp.Layout.Row = iitem+1;
        lmp.Layout.Column = iCol;
        if Flag
            lmp.Color = [0 1 0];
            Enable = 'on';
        else
            lmp.Color = [1 0 0];
            Enable = 'off';
        end
        resizeTableColumn(app, lmp, g, lmp.Layout.Column)
        lmp = define_submodels_contextmenu(app, app.ContextMenu_Cref,  lmp, {tag itemclass}, Enable);
        out_obj = lmp;

    case 'PLCgen-Ready'

        if nargin ~=8
            error('Wrong number of input arguments to tag definition')
        else
            Flag = varargin{1};
            itemclass = varargin{2};
            tag =  varargin{3};

        end

        lmp = uilamp(g);
        lmp.Layout.Row = iitem+1;
        lmp.Layout.Column = iCol;
        if Flag
            lmp.Color = [0 1 0];
%             Enable = 'on';
        else
            lmp.Color = [1 0 0];
%             Enable = 'off';
        end
        resizeTableColumn(app, lmp, g, lmp.Layout.Column)
        out_obj = lmp;


    case 'Details'

        if nargin ~=7
            error('Wrong number of input arguments to tag definition')
        else
%             Flag = varargin{1};
            itemclass = varargin{1};
            tag =  varargin{2};

        end

        item = uibutton(g,'ButtonPushedFcn',@(obj, event) button_details_Callback(app,{tag itemclass}));
        item.Layout.Row = iitem+1;
        item.Layout.Column = iCol;
        item.Text = 'Open';
        item.Enable = true;
        resizeTableColumn(app, item, g, item.Layout.Column)
        out_obj = item;

    case 'Create Empty ITFC'

        if nargin ~=6
            error('Wrong number of input arguments to tag definition')
        else
            tag =  varargin{1};

        end

        item = uibutton(g,'ButtonPushedFcn',@(obj, event) button_empty_itfc_Callback(app,{tag}));
        item.Layout.Row = iitem+1;
        item.Layout.Column = iCol;
        item.Text = 'Create';
        item.Enable = true;
        resizeTableColumn(app, item, g, item.Layout.Column)
        out_obj = item;

     case 'Create Random ITFC'

        if nargin ~=6
            error('Wrong number of input arguments to tag definition')
        else
            tag =  varargin{1};

        end

        item = uibutton(g,'ButtonPushedFcn',@(obj, event) button_random_itfc_Callback(app,{tag}));
        item.Layout.Row = iitem+1;
        item.Layout.Column = iCol;
        item.Text = 'Create';
        item.Enable = true;
        resizeTableColumn(app, item, g, item.Layout.Column)
        out_obj = item;


    case 'Load ITFC'

        if nargin ~=6
            error('Wrong number of input arguments to tag definition')
        else
            tag =  varargin{1};
        end

        item = uibutton(g,'ButtonPushedFcn',@(obj, event) button_load_itfc_Callback(app,{tag}));
        item.Layout.Row = iitem+1;
        item.Layout.Column = iCol;
        item.Text = 'Load';
        item.Enable = true;
        resizeTableColumn(app, item, g, item.Layout.Column)
        out_obj = item;



end


end