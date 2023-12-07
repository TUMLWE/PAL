To start a new session, launch the *App Designer* file “main.mlapp” and run it. A new GUI
will open, featuring two primary tabs which focus on distinct operations within the framework: the
:ref:`Develop/Deploy<user_guide_gui_dd>` and the :ref:`Test<user_guide_gui_test>` tab.

.. _user_guide_gui_dd:

Develop/Deploy
^^^^^^^^^^^^^^^^^^^

The *Develop/Deploy* tab allows the definition of the applications for each layer. A sample
of the GUI for the sample project "met_mast_reader" is illustrated in :numref:`paldd`.

.. figure:: images/paldd_1.png
   :width: 1000
   :name: paldd

   Develop/Deploy Tab

The following table provides a description of the Develop/Deploy tab.

.. csv-table::  GUI - description of Develop/Deploy tab
   :file: gui_guide_dd.csv
   :widths: 30, 30, 40
   :header-rows: 1

New applications can be generated for each layer through dedicated buttons. Specific application panels
are detailed for each layer.

SUBMODEL
"""""""""

Upon generating a new *SUBMODEL*, various options and flags become visible.

.. csv-table::  GUI - SUBMODEL panel
   :file: sm_gui.csv
   :widths: 30, 70
   :header-rows: 1

If a user intends to use an externally developed *Simulink* model, it is advisable to copy-paste its content into a newly created one. This ensures the retention of the *Simulink* model settings defined via the "Create Simulink Model" button. This action generates, alongside an empty *Simulink* model, an
initialization .m file, which can be used for configuring specific model parameters.

ITFC
"""""""""
.. csv-table::  GUI - ITFC panel
   :file: ITFC_gui.csv
   :widths: 30, 70
   :header-rows: 1

HOST
"""""""""
The definion of a *HOST* is equivalent to that of a *SUBMODEL* panel.

.. _user_guide_gui_test:

Test
^^^^^^^^^^^^^^^^^^^

After the generation of PLC applications, the framework can be executed on the PLC hardware. The *HOST*
application generates ".txt" outputs, which can be used through the *Test* tab to verify the correctness of the data transmission between the different applications. The tab provides tools for reading *HOST* outputs and comparing results between PLC and *Simulink*, among other functions. It is important to mention that, since applications must be loaded onto the PLC sequentially, from lower to higher layers (*ITFC*, *HOST*, and then *SUBMODEL*), a residual delay exists between
test *ITFC* data and *HOST* data, which is of course not physical but purely introduced by this delay. 

An example of the GUI for the example project "met_mast_reader" is shown in :numref:`paltest`.

.. figure:: images/paltest.png
   :width: 800
   :name: paltest

   Test tab

.. csv-table::  GUI - overview Test tab
   :file: gui_guide_test.csv
   :widths: 30, 30, 40
   :header-rows: 1

Before running *Simulink* models, unphysical data should be trimmed using the Brush function, along with the "Brush Data/sync ITFC app" (right-click on the figure). Users should familiarize themselves with the testing functionality by following the :ref:`examples provided<user_guide_example1>`.



