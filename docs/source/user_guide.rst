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



New Project Definition
-----------------------

When creating a new project through the GUI (Section XX), two excel files are created by default: “inputfile.xlsx” and “SVI_definition.xlsx”. Those files define each project and are used to define the application layers described above, as well as the data exchange between each application.

Inputfile.xlsx
~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This excel file defined the data flow between the different layers of each project. Variables for each application must be specified, as well as data trasmission to other applications. 

Generally speaking, each application hosts a "shared variable interface" (SVI), which can be read and written by other applications. There are three sheets in the SVI_Definition file, one for each layer of the framework (ITFC, HOST, Submodel). Variables for each layer are treated differently, as specified below.

**ITFC** 

To promote separation between hardware-specific and research-specific operations, this layer should be ideally developed by the hardware owner. This layer is used uniquely for testing purposes, to ensure that the remaining framework components (mainly the simulink submodels) operate as expected. For this reason it is fundamental that the SVI of the ITFC applications exactly replicates the one encountered during experimentation. 

ITFC applications are "inert", in the sense that they do not actively perform any reading or writing operation on any other application's SVI. HOST applications can read and write information to ITFC SVI's. 

.. csv-table::  SVI_Definition - ITFC sheet
   :file: svi_definition_itfc.csv
   :widths: 30, 30, 40
   :header-rows: 1

A complete list of the supported data types is provided **here**


.. csv-table::  SVI_Definition - HOST sheet
   :file: svi_definition_host.csv
   :widths: 30, 30, 40
   :header-rows: 1

.. csv-table::  SVI_Definition - SUBMODELS sheet
   :file: svi_definition_submodels.csv
   :widths: 30, 30, 40
   :header-rows: 1






