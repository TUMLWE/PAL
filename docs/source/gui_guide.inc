To start a new session, open the App Designer file “main.mlapp” and run it. A new GUI will open. There are two main tabs that are devoted to two specific operations of the framework: the “Develop/Deploy” and the “test” tab. 


.. _user_guide_gui_dd:

Develop/Deploy
^^^^^^^^^^^^^^^^^^^

Through the Develop/Deploy tab it is possible to define the main applications for each layer. A sketch of the GUI for the example project "met_mast_reader" is shown in :numref:`paldd`

.. figure:: images/paldd_1.png
   :width: 1000
   :name: paldd

   Develop/Deploy Tab

And explained in the following table.

.. csv-table::  GUI - overview Develop/Deploy tab
   :file: gui_guide_dd.csv
   :widths: 30, 30, 40
   :header-rows: 1


New applications can be generated for each layer through the respective buttons. 
Application panels are described for each layer.

SUBMODEL
"""""""""
Aften generating a new SUBMODEL, several options and flags will be visible

.. csv-table::  GUI - SUBMODEL panel
   :file: sm_gui.csv
   :widths: 30, 70
   :header-rows: 1

It is important to remark that if an user wants to use a Simulink model developed externally, it is better to copy-paste its content into a newly created one, thus keeping the Simulink model settings set through the button "Create Simulink Model". This will create, beside an empty Simulink model, an initialization .m file that can be used to initialize specific model parameters.

ITFC
"""""""""
.. csv-table::  GUI - ITFC panel
   :file: ITFC_gui.csv
   :widths: 30, 70
   :header-rows: 1

HOST
"""""""""
Same as SUBMODEL panel.

.. _user_guide_gui_test:

Test
^^^^^^^^^^^^^^^^^^^

After generating the PLC applications it it possible to run the framework on the PLC hardware. The host application will generate .txt outputs, which can be used to verify the correctness of the data transmission between the different applications. The Test tab provides a tool to read HOST outputs and to compare the results between PLC and Simulink, among other functions. It is important to notice that the applications should be loaded on the PLC in sequential order from lower to higher layer, i.e. ITFC, HOST, SUBMODEL. For this reason a residual delay will exist between the test ITFC data and the HOST data, which is of course not physical but purely introduced by this delay. 

A sketch of the GUI for the example project "met_mast_reader" is shown in :numref:`paltest`


.. figure:: images/paltest.png
   :width: 1000
   :name: paltest

   Test tab


.. csv-table::  GUI - overview Test tab
   :file: gui_guide_test.csv
   :widths: 30, 30, 40
   :header-rows: 1

Before running the Simulink models, unphysical data should be trimmed by using the Brush function, together with the "Brush Data/ sync ITFC app" (right-click on the figure). The user should familiarize with the testing functionality by following the **examples**

