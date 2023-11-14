function obj = define_submodels_contextmenu(app, cm_ref, obj, var, Enable)

cm = uicontextmenu(app.UIFigure); % since we can't copy the object, we need to redefine it
obj.ContextMenu = cm;

for iMenu = 1 : length(cm_ref.Children)

    ctc = cm_ref.Children(iMenu);
    mc =  uimenu(obj.ContextMenu);

    mc.Text = ctc.Text;

    if strcmp(mc.Text, 'Add Matlab Fields') % here i can add all the different callbacks for the different contextmenu options

        fn_handle = {@app.add_matlab_fields_callback, var};
        mc.MenuSelectedFcn = fn_handle;
        mc.Enable = Enable;

    end

end

end