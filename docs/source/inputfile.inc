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

ITFC
"""""""""

One row for each ITFC application. Each can take as input a test interface in the form of .mat file. Multiple ITFC Apps are supported. 

.. csv-table:: Inputfile - ITFC sheet
   :file: inputfile_itfc.csv
   :widths: 30, 30, 40
   :header-rows: 1

HOST
"""""""""

One row for each HOST application. IMPORTANT: only one HOST application can be used for each project. 

.. csv-table:: Inputfile - HOST sheet
   :file: inputfile_host.csv
   :widths: 30, 30, 40
   :header-rows: 1

SUBMODELS
"""""""""

One row for each Simulink submodel. IMPORTANT: each submodel can have a single Simulink model associated. There is no limit to the number of SUBMODELS for each project. 

.. csv-table::  Inputfile - SUBMODELS sheet
   :file: inputfile_submodel.csv
   :widths: 30, 30, 40
   :header-rows: 1