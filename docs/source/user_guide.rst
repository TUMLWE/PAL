User Guide
===========

.. contents::

General Framework Architecture
------------------------------

The framework adopts a structured three-layer approach. There are named *ITFC*, *HOST*, and *SUBMODEL*. Each layer consists of specific applications, each designed to serve a distinct purpose:

**ITFC:**
*Interfaces* are the lowest-level applications responsible for tasks such as sensor reading and providing input to actuators, effectively connecting to the physical layer. Ideally, *ITFC* apps can be developed by the owner of the hardware. The use of an *ITFC* layer is only necessary when data exchange towards sensors or equipment is required. Multiple *ITFC* apps are permitted. *ITFC* applications share variables through a shared variable interface (SVI), and these can be read from and written to by a *HOST* app.

**HOST:**
This application operates in the middle layer and is responsible for tasks like data reading and writing to and from *ITFC* applications. *HOST* also manages variables crucial for experiment execution, monitoring, and communication between *SUBMODEL* applications. It generates output files in text format. Please note that as of release 1.0, each project supports only one *HOST* application. Data sharing is performed through a shared variable interface (SVI), allowing read and write access by *SUBMODEL* apps.

**SUBMODEL:**
This is the top layer, comprising the primary application models based on *Simulink* models. *Simulink* inputs and outputs are connected to *HOST* SVIs. Any number of *SUBMODELS* can be implemented within a project.

One sampling frequency must be specified for the execution of the whole framework. As of release 1.0, different frequency execution for different applications is not supported. An exemplary sketch of the framework data flow is shown in :numref:`data_flow_chart`.

Empty C source projects should be generated for each PLC application through *Bachmann SolutionCenter*. These source codes are then automatically modified by the framework to incorporate all the necessary variable interconnections required by the different applications.

The framework is equipped with a :ref:`Graphic User Interface (GUI)<user_guide_GUI>` that simplifies the development process and is divided into :ref:`Develop/Deploy<user_guide_gui_dd>` and :ref:`Test<user_guide_gui_test>`.

.. figure:: images/data_flow_chart.png
   :width: 1000
   :name: data_flow_chart

   Data flow between the different layers


.. _user_guide_project_definition:

Project Definition 
-----------------------

When a new project is initiated using the :ref:`Graphical User Interface (GUI)<user_guide_GUI>`, a folder is automatically generated with the project's name. Two additional folders, **ReferenceCFiles** and **SimulinkModels**, are created to store the reference PLC source code and necessary *Simulink* models for the *SUBMODEL* apps.

Additionally, two *Excel* files, **inputfile.xlsx** and **SVI_definition.xlsx** are created along with the folders. These files are crucial for project definition, outlining the layers of the applications architecture, and showing how data is exchanged among each application. Both these files are described in the following paragraphs.


Inputfile.xlsx
^^^^^^^^^^^^^^^^^^^^

.. include:: inputfile.inc


SVI_Definition.xlsx
^^^^^^^^^^^^^^^^^^^^

.. include:: svi_definition.inc

.. _user_guide_GUI:

Graphic User Interface
-----------------------

.. include:: gui_guide.inc

Examples
-----------------------

The following subsections describe two examples derived
from wind energy applications: a *Met-Mast* data reader,
which can be used to read data from a met-mast and
perform moving averages for monitoring purposes, and a
*SCADA-data* reader.

.. include:: example_1.inc

Example 2: SCADA data reader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The present tutorial aims at developing a SCADA data reader for wind energy applications. As a prerequisite, the user is reccommended to have follower the tutorial "Met-Mast data reader", since this tutorial will focus mostly on the differences with respect to the previous case.

Create a new project, and name it "SCADA_reader".

In this tutorial,several applications will be created:

    #. 3 SUBMODELS, connected to Simulink models performing inflow averagine, wind farm monitoring, and loads postprocessing
    #. 3 ITFC, which simulate data provided by different clients
    #. An HOST, responsible for managing output data

Create SUBMODEL apps
"""""""""""""""""""""""""""

The procedure to create new SUBMODELS is the same described in the previous example (INSERT LINK). SUBMODELS naming goes as follows:

    #. Create three submodels, named "avg_inflow", "WF_status", "loads_process".
    #. Create empty Simulink models for the three SUBMODELS
    #. Create new Bachmann C projects and name them "avginflw", "wfstatus", "loadsprc". Copy into the reference C folder. Update the names it by clicking “Details” and changing “refC_name”
    #. Right-click on the “C ref found” light and select “Add Matlab Fields”
    #. Create the Simulink models. For simplicify, use the models provided under "Examples\\SCADA_reader_sample"
    #. If you inspect the Simulink models provided, you will notice that "avg_inflow" is equivalent to "avg_calc" of the previous example. "WF_status" simply check whether a turbine is producing power and whether the wind direction is within a specific range. "loads_process" calculates out-of-plane and in-plane blade root bending moments from flapwise and edgewise moments.

Create ITFC apps
"""""""""""""""""""""""""""

#. Create three ITFC apps, named "itfc_scada", "itfc_mm" and "itfc_loads".
#.  As already done for the SUBMODEL apps, create empty PLC code and paste the reference source codes. Name them "ifcscada", "ifcmm", "ifcloads"
#.   Update "Details" reference C codes.
#.   Right-click on the "C ref found" light and select "Add Matlab Fields"

Create HOST apps
"""""""""""""""""""""""""""
#. Create one HOST apps, named "host_scada"
#. As already done for the SUBMODEL apps, create empty PLC code and paste the reference source codes. Name it "hstscada"
#.   Update "Details" reference C codes.
#.   Right-click on the "C ref found" light and select "Add Matlab Fields"
#.  Specify “out_filename” field in “Details” as "host_scada" and provide the "output_path_in_PLC" (note that this may vary on your PLC). Ensure the corresponding folder is pre-created on the PLC; otherwise, the entire framework will crash on startup
#.  Save

Modify the "inputfile.xlsx"
""""""""""""""""""""""""""""""""

The steps to modify the "inputfile.xlsx" are the same described in (SECTION XX). The different tabs are reported below, for completeness.

.. csv-table::  SCADA_reader - "inputfile.xlsx" - SUBMODEL
   :file: inputfile_scadap1_sm.csv
   :header-rows: 1

.. csv-table::  SCADA_reader - "inputfile.xlsx" - ITFC
   :file: inputfile_scadap1_itfc.csv
   :header-rows: 1

.. csv-table::  SCADA_reader - "inputfile.xlsx" - HOST
   :file: inputfile_scadap1_host.csv
   :header-rows: 1

.. csv-table::  SCADA_reader - "inputfile.xlsx" - Settings
   :file: inputfile_scadap1_settings.csv
   :widths: 30, 60, 10
   :header-rows: 1

Modify the “SVI_Definition”
""""""""""""""""""""""""""""""""

Unlike the "met_mast_reader", a more complex variable mapping is necessary for the "SCADA_reader". Each layer is detailed with a dedicated section.

**ITFC**

The test ITFC applications are here designed to simulate a realistic case in which data is coming from different hardware and applications, and operating in a three wind turbines wind farm. For each ITFC application, the following variables are included:

#. itfc_scada contains 6 variables. SCADA data is provided through a structure variable, one for each turbine. Each structure includes subfields related to different quantities such as rotorspeed, pitch, power, and so on. In addition, vibrations are measured as well for each turbine and in the three directions, through double arrays
#. itfc_mm contains 1 structure. This ITFC app replicates "met_mast_ITFC" of the "met_mast_reader"
#. itfc_loads contains 1 structure, which provide data in terms of blade loads for the three wind turbines
#. two additional WRITE variables are included within the ITFC app "itfc_scada". The first is a 10-element double array used for control purposes. The second is a structure in which the in-plane and out-of-plane bending loads are written by the HOST

An overview of the ITFC variables is shown below

.. csv-table::  SCADA_reader - "SVI_Definition.xlsx" - ITFC
   :file: SVI_Definition_scada_itfc.csv
   :header-rows: 1

**HOST**

The following table presents the list of the HOST variables implemented in the "SVI_Definition.xlsx". It is possible to notice that the variables fed to "exchange_data_ctrl" has been assigned an initial value equal to 1. Beside the greater number of variables, there is not many differences with respect to the "met_mast_reader". 

.. csv-table::  SCADA_reader - "SVI_Definition.xlsx" - HOST
   :file: SVI_Definition_scada_host.csv
   :header-rows: 1

**SUBMODEL**

Similarly, an overview of the SUBMODELS variables is provided below.

.. csv-table::  SCADA_reader - "SVI_Definition.xlsx" - SUBMODEL
   :file: SVI_Definition_scada_sm.csv
   :header-rows: 1

Generate the test interface
""""""""""""""""""""""""""""""""
After modifying the “SVI_Definition.xlsx”, the project needs to be reloaded. Similarly to the "met_mast_reader", the correct size will need to be provided for the structure variables.

    #. Dummy interfaces need to be generated for the different ITFC applications. Define the "test_ITFC_filename" fields for each ITFC app, named:
        #. ./Examples/SCADA_reader/itfc_scada.mat
        #. ./Examples/SCADA_reader/itfc_mm.mat
        #. ./Examples/SCADA_reader/itfc_loads.mat
    #. Click on "Create Random ITFC"
    #. You will notice that new .mat files are generated for the three ITFC applications. Again, this will not contain realistic data. You have two options: either replace the random data with your own or use the provided dummy variables containing realistic data. By default, only the read variables will be filled with random numbers, while the others will be set to 0.
    #. Open the details of the ITFC apps and set their “Flag_Create_test_ITFC” to TRUE. Click on “Load ITFC”.

Generate the PLC code
""""""""""""""""""""""""""""""""

Save as (SECTION XX)


Testing the framework
""""""""""""""""""""""""""""""""

The testing of the project follows very closely what was done for the "met_mast_reader". A few remarks:
    #. The syncronization must be performed on all three ITFC applications
    #. SUBMODELS will be run at "FAST" frequency, which may differ from the HOST variables




Appendix
-----------------------

.. include:: appendix.inc

.. include:: types.inc