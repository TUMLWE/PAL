Example 2: SCADA data reader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The second example in this tutorial focuses on developing a SCADA data reader for wind energy applications. It assumes that users have already followed the "Met-Mast Data Reader" tutorial, as this example will only highlight the differences between the two cases.

Begin by creating a new project and naming it "SCADA_reader."

In this tutorial, several applications will be created:

#. 3 *SUBMODEL* apps: These are connected to Simulink models responsible for inflow averaging, wind farm monitoring, and loads post-processing

    #. avg_inflow: A *SUBMODEL* connected to a *Simulink* model performing inflow averaging
    #. WF_status: A *SUBMODEL* monitoring wind farm status
    #. loads_process: A *SUBMODEL* for loads post-processing
#. 3 *ITFC* apps: These *ITFC* applications simulate data provided by different clients

    #. itfc_scada: Simulating data from SCADA for different turbines, including rotor speed, pitch, power, vibrations, and control variables
    #. itfc_mm: Simulating data from a meteorological mast, replicating "met_mast_ITFC" from the previous example
    #. itfc_loads: Simulating data for blade loads on three wind turbines.
#. An *HOST* app:

    #. host_scada: A HOST application coordinating the overall functionality and data output

Create SUBMODEL apps
"""""""""""""""""""""""""""

The procedure to create new *SUBMODEL* is the same described in the previous example :ref:`example (GUI)<user_guide_example1_createsubmodel>`:

    #. Create three *SUBMODEL*, named "*avg_inflow*", "*WF_status*", "*loads_process*"
    #. Create empty *Simulink* models for all the *SUBMODEL* apps
    #. Create new *Bachmann* C projects and name them "*avginflw*", "*wfstatus*", "*loadsprc*". Copy them into the reference C folder. Update the *refC_name* field using the “Details” menu
    #. Right-click on the “*C ref found*” light and select “*Add Matlab Fields*”
    #. Create the *Simulink* models. For simplicify, use the models provided under "Examples\\SCADA_reader_sample"
    #. Inspect the *Simulink* models; "*avg_inflow*" corresponds to "*avg_calc*" from the previous example. "*WF_status*" checks turbine power production and wind direction within a specified range. "*loads_process*" calculates out-of-plane and in-plane blade root bending moments from flapwise and edgewise bending moments.

Create ITFC apps
"""""""""""""""""""""""""""

#. Create three *ITFC* apps, named "*itfc_scada*", "*itfc_mm*" and "*itfc_loads*".
#. As already done for the *SUBMODEL* apps, create empty PLC code and paste the reference source codes. Name them "*ifcscada*", "*ifcmm*", "*ifcloads*"
#. Update "Details" reference C codes.
#. Right-click on the "C ref found" light and select "*Add Matlab Fields*"

Create HOST apps
"""""""""""""""""""""""""""
#. Create one *HOST* apps, named "*host_scada*"
#. As already done for the *SUBMODEL* apps, create empty PLC code and paste the reference source codes. Name it "*hstscada*"
#. Update "Details" reference C codes.
#. Right-click on the "C ref found" light and select "*Add Matlab Fields*"
#. Specify *out_filename* field in “Details” as "*host_scada*" and provide the *output_path_in_PLC* (note that this may vary on your PLC). Ensure the corresponding folder is pre-created on the PLC; otherwise, the entire framework will crash on startup
#.  Save

Modify the "inputfile.xlsx"
""""""""""""""""""""""""""""""""

The steps to modify the "inputfile.xlsx" are the same described in the previous :ref:`example<user_guide_example1_inputfile>`. The different tabs are reported below, for completeness.

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

For the "SCADA_reader," a more intricate variable mapping is required compared to the "met_mast_reader." Each layer, including *ITFC*, *HOST*, and *SUBMODEL*, is detailed in the following sections.

**ITFC**

The *ITFC* applications simulate a realistic scenario with data coming from various hardware and applications in a three-wind-turbine wind farm. Each *ITFC* application includes the following variables:

#. **itfc_scada** contains 6 variables. SCADA data is provided through a structure variable, one for each turbine. Each structure includes subfields related to different quantities such as rotorspeed, pitch, power, and so on. In addition, vibrations are measured as well for each turbine and in the three directions, through double arrays
#. **itfc_mm** contains 1 structure. This *ITFC* app is identical to "met_mast_ITFC" of the "met_mast_reader" example
#. **itfc_loads** contains 1 structure, which provides data in terms of blade loads for the three wind turbines
#. Two additional "WRITE" variables are included within the *ITFC* app "*itfc_scada*":

    #. A 10-element double array used for control purposes.
    #. A structure in which the in-plane and out-of-plane bending loads are written by the *HOST*.

An overview of the ITFC variables is shown below

.. csv-table::  SCADA_reader - "SVI_Definition.xlsx" - ITFC
   :file: SVI_Definition_scada_itfc.csv
   :header-rows: 1

**HOST**

The table below presents the list of *HOST* variables implemented in the "SVI_Definition.xlsx." The variables fed to "exchange_data_ctrl" have been assigned an initial value equal to 1. Besides the greater number of variables, the data handling is similar to that of "met_mast_reader."

.. tabularcolumns:: |p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|p{1cm}|
.. csv-table::  SCADA_reader - "SVI_Definition.xlsx" - HOST
   :file: SVI_Definition_scada_host.csv
   :widths: 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
   :header-rows: 1

**SUBMODEL**

Similarly, an overview of the SUBMODELS variables is provided below.

.. tabularcolumns:: |\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|\X{1}{10}|
.. csv-table::  SCADA_reader - "SVI_Definition.xlsx" - SUBMODEL
   :file: SVI_Definition_scada_sm.csv
   :widths: 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
   :header-rows: 1


Generate the test interface
""""""""""""""""""""""""""""""""
After modifying the “SVI_Definition.xlsx”, the project needs to be reloaded. Similarly to the "met_mast_reader", the correct size will need to be provided for the structure variables.

    #. Dummy interfaces need to be generated for the different *ITFC* applications. Define the "test_ITFC_filename" fields for each *ITFC* app, named:

        #. ./Examples/SCADA_reader/itfc_scada.mat
        #. ./Examples/SCADA_reader/itfc_mm.mat
        #. ./Examples/SCADA_reader/itfc_loads.mat
    #. Click on "Create Random ITFC"
    #. New ".mat" files are generated for the three ITFC applications. Note that this data will not contain realistic values. You have two options:

        #. Replace the random data with your own.
        #. Use the provided dummy variables (located in "*root\\PAL\\Examples\\SCADA_reader_sample*") containing realistic data. By default, only the read variables will be filled with random numbers, while the others will be set to 0.
    #. Open the details of the ITFC apps and set their “Flag_Create_test_ITFC” to TRUE. Click on “Load ITFC”.

Generate the PLC code
""""""""""""""""""""""""""""""""

:ref:`See previous example<user_guide_example1_generateplccode>`

Testing the framework
""""""""""""""""""""""""""""""""

The testing of the project follows very closely what was done for the "met_mast_reader". A few remarks:

    #. The syncronization must be performed on all three ITFC applications
    #. *SUBMODELS* will be run at "FAST" frequency, which may differ from the *HOST* variables frequency