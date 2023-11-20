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
The main tabs are described in the following:

.. csv-table:: Tab: Main Folders
   :file: inputfile_description_mainfolders.csv
   :widths: 30, 30, 40
   :header-rows: 1

.. csv-table:: Tab: Settings
   :file: inputfile_settings.csv
   :widths: 30, 30, 40
   :header-rows: 1
