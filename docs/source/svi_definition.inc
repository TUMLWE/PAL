This *Excel* file defines the data flow among different
layers in each project. It outlines the variables for each
application and defines their transmission to other applications.

In general, each application manages a *shared variable
interface* (SVI), which is read and written by
other applications. The **SVI_Definition** file comprises three
sheets, each dedicated to a layer of the framework (*ITFC*,
*HOST*, *Submodel*). Common variable data types are detailed in the
:ref:`appendix<user_guide_appendix_supporteddata>`.

.. _user_guide_svi_definition_itfc:

ITFC 
"""""""""

To promote the separation between hardware-specific and
research-specific operations, the *ITFC* layer should be ideally
developed by the hardware owner. Within the present framework, this layer serves the unique
purpose of testing, ensuring the proper functioning of the
framework components, primarily the *Simulink* submodels.
Thus, it is crucial that the SVI of *ITFC* applications
precisely replicates the one encountered during experimentation.


*ITFC* applications are "inert," meaning they do not actively
perform reading or writing operations on any other application's
SVI. *HOST* applications, on the other hand, can read and write
information to *ITFC* SVIs.

**ITFC Variables** 

In addition to the mentioned :ref:`data types<user_guide_appendix_supporteddata>`, *ITFC* variables
can also be implemented as structures, where all subfields share the
same "Access" property. While numerical arrays for subfields
are not yet supported (except for string arrays), a detailed
description of all table elements is provided below.

.. csv-table::  SVI_Definition - ITFC sheet
   :file: svi_definition_itfc.csv
   :widths: 30, 30, 40
   :header-rows: 1

HOST
"""""""""

An *HOST* variable can be READ/WRITE to an *ITFC* variable, but cannot be transmitted to *SUBMODEL* variables.

*HOST* Variables serve various purposes, including:
   #. Reading variables from *ITFC* and host them to be read by *SUBMODEL* apps.
   #. Read variable from ITFC for monitoring purposes
   #. Creating static variables for *SUBMODEL*, particularly useful for constant definitions
   #. Receiving output from *SUBMODEL* and writing it back to *ITFC*
   #. Receiving *SUBMODEL* output for monitoring purposes
   #. Receiving the AppStatus variable from *SUBMODEL* to assess the application's operation

Apart from string variables, *HOST* variables **MUST** consist of a single numerical value; arrays are not allowed. It is important to remind that, when exchanging variables with an *ITFC* app, the "Action" of a *HOST* Variable aligns with the "Access" of an *ITFC* Variable. By default, *HOST* applications generate three .txt outputs at different sampling times: "fast," "slow," "ctrl." For each *HOST* variable, this behavior can be controlled by the "output_freq" field.

.. csv-table::  SVI_Definition - HOST sheet
   :file: svi_definition_host.csv
   :widths: 30, 30, 40
   :header-rows: 1

SUBMODEL
"""""""""

*SUBMODEL* variables can perform READ/WRITE operations to a *HOST* variable, but they do not perform actions on other *SUBMODEL* apps. For each *Simulink* model, it is recommended to include **all** model inputs and outputs as *SUBMODEL* variables (with the correct port numbering). Additionally, for each *SUBMODEL* app, include an **AppStatus** status variable, which informs the *HOST* application about the execution of the *SUBMODEL*.

*SUBMODEL* variables must be single numerical values; arrays are not allowed. The "IO" type field must match the type of the Simulink model port, except for "status" variables. Generally, each *SUBMODEL* will contain nI + nO + 1 variables, where nI/nO indicates the number of input/output ports of a Simulink model.

.. csv-table::  SVI_Definition - SUBMODELS sheet
   :file: svi_definition_submodels.csv
   :widths: 30, 30, 40
   :header-rows: 1

Below in :numref:`sketch_data_transmission`, an illustrative sketch exemplifies the data transmission involving two *ITFC* variables (a structure and an array). These variables are exchanged with a *Simulink* model.


.. figure:: images/org_chart.png
   :width: 1000
   :name: sketch_data_trasmission

   Example of data transmission among *ITFC*, *HOST*, and *SUBMODEL* layers