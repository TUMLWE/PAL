This file holds essential project information like folders, settings, and application layers definition. Its main tabs are detailed below.

**Main Folders** 

In this section, users will find the necessary subfolder structure for each project, which is automatically generated upon initiating a new project. A detailed description of each table entry is provided:

.. csv-table:: Inputfile - Main Folders sheet
   :file: inputfile_description_mainfolders.csv
   :widths: 30, 30, 40
   :header-rows: 1

**Settings** 

The "Settings" tab manages important aspects of project execution and compilation. Users must specify parameters such as sample time execution and PLC System settings. Additionally, the location of the "SVI_Definition.xlsx file" needs to be specified:

.. csv-table:: Inputfile - Settings sheet
   :file: inputfile_settings.csv
   :widths: 30, 30, 40
   :header-rows: 1

ITFC
"""""""""

In this tab, a row is dedicated to each *ITFC* application. Each application can receive a test interface in the form of a .mat file as input. The system supports the inclusion of multiple *ITFC* applications:

.. csv-table:: Inputfile - ITFC sheet
   :file: inputfile_itfc.csv
   :widths: 30, 30, 40
   :header-rows: 1

.. _user_guide_inputfile_host: 

HOST 
"""""""""

In this tab, a row is dedicated to each *HOST* application. IMPORTANT: as of this version, the system restricts the usage to only one *HOST* application per project. 

.. csv-table:: Inputfile - HOST sheet
   :file: inputfile_host.csv
   :widths: 30, 30, 40
   :header-rows: 1

SUBMODELS
"""""""""

One row for each Simulink submodel. IMPORTANT: each *SUBMODEL* can have a single Simulink model associated. There is no limit to the number of *SUBMODEL* apps for each project. 

In thi tab, one row is dedicated to each *SUBMODEL*. Notably, as of this version, each submodel must have a single *Simulink* model associated. There is no specific limit to the number of *SUBMODEL* apps permitted for each project:

.. csv-table::  Inputfile - SUBMODELS sheet
   :file: inputfile_submodel.csv
   :widths: 30, 30, 40
   :header-rows: 1