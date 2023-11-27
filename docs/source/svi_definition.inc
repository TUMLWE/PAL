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