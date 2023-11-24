User Guide
===========

.. contents::

General Framework Architecture
------------------------------

The framework adopts a structured three-layer approach. Each layer consists of specific applications, each designed to serve a distinct purpose:

**ITFC (Interface):**
This is the lowest-level application responsible for tasks such as sensor reading and providing input to actuators, effectively connecting to the physical layer. Ideally, ITFC apps can be developed by the owner of the hardware. Every project must include at least one ITFC app, though the use of multiple ITFC apps is permitted. ITFC applications share variables through a shared variable interface (SVI), and these can be read from and written to by a HOST app.

**HOST:**
This application operates in the middle layer and is responsible for tasks like data reading and writing to and from ITFC applications. HOST also manages variables crucial for experiment execution, monitoring, and communication between SUBMODEL applications. It generates output files in text format. Please note that as of October 2023, each project supports only one HOST application. Data sharing is performed through a shared variable interface (SVI), allowing read and write access by SUBMODEL apps.

**SUBMODEL:**
This is the top layer, comprising the primary application models based on Simulink models. Simulink inputs and outputs are connected to HOST SVIs. Any number of SUBMODELS can be implemented within a project.

One sampling frequency must be specified for the execution of the whole framework. As of Release 1.0, different frequency execution for different applications is not supported. An exemplary sketch of the framework data flow is shown in figure [INSERT FIGURE].

Empty C Source projects should be generated for each PLC application through Bachmann SolutionCenter. These source codes are then automatically modified by the framework to incorporate all the necessary variable interconnections required by the different applications.

The framework is equipped with a Graphic User Interface (GUI) that simplifies the development process and which is divided into Develop/Deploy (Section XX) and Test (Section XX).



Project Definition
-----------------------

When creating a new project through the GUI (Section XX), two excel files are created by default: “inputfile.xlsx” and “SVI_definition.xlsx”. Those files define each project and are used to define the application layers described above, as well as the data exchange between each application.

Inputfile.xlsx
^^^^^^^^^^^^^^^^^^^^

This file contains general information for the project, such as folder definitions,  settings and layers applications.
The main tabs are described below.

**Main Folders** 

This tab defines the location of subfolders necessary for each project. This sheet is automatically generated upon creation of a new project. A description of each table entry is provided here:


.. csv-table:: Inputfile - Main Folders sheet
   :file: inputfile_description_mainfolders.csv
   :widths: 30, 30, 40
   :header-rows: 1

**Settings** 

This tab defines some main settings of the project execution and compilation. Important parameters, such as sample time execution and PLC System must be specified here, as well as the location of the "SVI_Definition.xlsx file"

.. csv-table:: Inputfile - Settings sheet
   :file: inputfile_settings.csv
   :widths: 30, 30, 40
   :header-rows: 1

**ITFC** 

One row for each ITFC application. Each can take as input a test interface in the form of .mat file. Multiple ITFC Apps are supported. 

.. csv-table:: Inputfile - ITFC sheet
   :file: inputfile_itfc.csv
   :widths: 30, 30, 40
   :header-rows: 1

**HOST** 

One row for each HOST application. IMPORTANT: only one HOST application can be used for each project. 

.. csv-table:: Inputfile - HOST sheet
   :file: inputfile_host.csv
   :widths: 30, 30, 40
   :header-rows: 1

**SUBMODELS** 

One row for each Simulink submodel. IMPORTANT: each submodel can have a single Simulink model associated. There is no limit to the number of SUBMODELS for each project. 

.. csv-table::  Inputfile - SUBMODELS sheet
   :file: inputfile_submodel.csv
   :widths: 30, 30, 40
   :header-rows: 1


SVI_Definition.xlsx
^^^^^^^^^^^^^^^^^^^^

This excel file defined the data flow between the different layers of each project. Variables for each application must be specified, as well as data trasmission to other applications. 

Generally speaking, each application hosts a "shared variable interface" (SVI), which can be read and written by other applications. There are three sheets in the SVI_Definition file, one for each layer of the framework (ITFC, HOST, Submodel). Allowed variables data types are common for each layer and can be found **here**. Exceptions exist for ITFC variables, as specified below.

ITFC
"""""""""

To promote separation between hardware-specific and research-specific operations, this layer should be ideally developed by the hardware owner. This layer is used uniquely for testing purposes, to ensure that the remaining framework components (mainly the simulink submodels) operate as expected. For this reason it is fundamental that the SVI of the ITFC applications exactly replicates the one encountered during experimentation. 

ITFC applications are "inert", in the sense that they do not actively perform any reading or writing operation on any other application's SVI. HOST applications can read and write information to ITFC SVI's. 

**ITFC Variables** 

Beside the already mentioned **data type** , ITFC variables in the form of structures can also be implemented. In this case all subfields will share the same "Access" property. Numerical arrays are not yet supported for subfields, with the exception of string arrays. A description of all the table elements is provided below.

.. csv-table::  SVI_Definition - ITFC sheet
   :file: svi_definition_itfc.csv
   :widths: 30, 30, 40
   :header-rows: 1



HOST
"""""""""

HOST variables can perform READ/WRITE actions to a ITFC variable, while they cannot perform actions on SUBMODEL.

**HOST Variables** 

HOST Variables can be created for many purposes, such as:
   (1) Read variable from ITFC to be read by SUBMODEL
   (2) Read variable from ITFC for monitoring purposes
   (3) Create static variables to be read by SUBMODEL, useful for consant definition
   (4) Receive output from SUBMODEL and write it to ITFC
   (5) Receive output from SUBMODEL for monitoring purposes
   (6) Receive AppStatus variable from SUBMODEL, to check operation of application

Beside string variables, all HOST variables **MUST** be a single numerical value, i.e, arrays are not allowed.
It is important that, in case of variables exchanged with an ITFC app, the "Action" of a HOST Variable matches the "Access" of a ITFC Variable. By default, HOST applications generate three txt outputs at three different sampling time: "fast", "slow", "ctrl". For each HOST variable, this can be controlled by the field "output_freq".

.. csv-table::  SVI_Definition - HOST sheet
   :file: svi_definition_host.csv
   :widths: 30, 30, 40
   :header-rows: 1

SUBMODEL
"""""""""

SUBMODEL variables can perform READ/WRITE actions to a HOST variable, while they cannot perform actions on SUBMODEL. For each Simulink model, it is reccommended to include **all** model inputs and outputs as SUBMODEL variables (with the correct port numbering). For each SUBMODEL application, a further **AppStatus** status variable should be included, which is used to inform the HOST application about the execution of the SUBMODEL.

**SUBMODEL Variables** 

Also SUBMODEL variables must be single numerical values, i.e, arrays are not allowed. The "IO" type field must match the type of port of Simulink model, except for "status" variables. Generally, each SUBMODEL will contain nI + nO + 1 variables, where nI/nO indicates the number of input/output ports of a Simulink model. 

.. csv-table::  SVI_Definition - SUBMODELS sheet
   :file: svi_definition_submodels.csv
   :widths: 30, 30, 40
   :header-rows: 1

Below in :numref:`sketch_data_trasmission`, an exemplary sketch that illustrates the data transmission of two ITFC variables (a structure and an array), which are exchanged with a Simulink model

.. figure:: images/org_chart.png
   :width: 1000
   :name: sketch_data_trasmission

   Example of data transmission between ITFC/HOST/SUBMODEL layers



Graphic User Interface
-----------------------

To start a new session, open the App Designer file “main.mlapp” and run it. A new GUI will open. There are two main tabs that are devoted to two specific operations of the framework: the “Develop/Deploy” and the “test” tab. 

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
   :widths: 30, 7
   :header-rows: 1

It is important to remark that if an user wants to use a Simulink model developed externally, it is better to copy-paste its content into a newly created one, thus keeping the Simulink model settings set through the button "Create Simulink Model". This will create, beside an empty Simulink model, an initialization .m file that can be used to initialize specific model parameters.

ITFC
"""""""""




Similarly. Create a new interface by clicking the add interface button. Similarly several columns will be shown: 
Apptag: It is the application name
Generate PLC: same as above
PLC ref found: same as above
PLCgen-Ready: same as above
Create empty ITFC: This allows to generate an empty interface that can be used for testing 
Create random ITFC: these allows generating are random interface can be used for testing data transmission 
Load ITFC column this can be used to load a previously generated interface file 
Details: same as above 

Same story for the Host application
Are there buttons are the following:
Refresh: Which can be used to reload the sub panels 
Generate PLC: This is used to generate the PLC applications of all the apps of which the generate PLC check mark is thicked. 
Save: These saved the actual final configuration onto the input file Excel. 

This tab can be used to develop new project by specify new submodels the, interface and host and deployed through the generate PLC button.
