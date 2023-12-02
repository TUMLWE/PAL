Example 1: Met-Mast Data Reader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


In this first example, the user creates a simple framework to read
data from a Met-Mast *ITFC* app, calculate moving averages,
and determine additional inflow quantities, which are then
written back to the *ITFC*.

To begin, open the framework by double-clicking on the file "main.mlapp" to open the main GUI. Two options are available: loading an existing file or creating a new one. Click on "new." Specify the folder for your new project. Open the "Examples" folder and click OK.

Specify the new project name. Call it "met_mast_reader" and click OK (:numref:`mmr_new`). This will create a set of subfolders, as explained in :ref:`Project Definition<user_guide_project_definition>`.

.. figure:: images/mmr_new.png
   :width: 1000
   :name: mmr_new

   Define new project view

Now create three applications, one for each data layer:

   #. A *SUBMODEL*, performing the averaging function
   #. An *ITFC* for data transmission and testing
   #. A *HOST*, responsible for managing output data

.. _user_guide_example1_createsubmodel:

Create a new SUBMODEL
"""""""""""""""""""""""""""

#. To begin, create a *SUBMODEL* model by selecting "Add *SUBMODEL*" and providing a name, such as *calc_avg.* Click OK. The underlying *Simulink* model will process inputs from wind speed and wind direction data to calculate averages based on the desired window size. A new model will appear in the relative panel's first row with several red indicator lights (:numref:`mmr_new2`).  

   .. figure:: images/mmr_new2.png
      :width: 1000
      :name: mmr_new2

      Overview of newly create"calc_avg"  

#. Generate an empty Simulink model by clicking "Create" in the "Create Simulink Model" column. This action creates the Simulink model in the model folder along with an "init_MODELTAG.m" file, which can be used for initializing data and constants. A green light under "*SLmodel-Ready*" indicates the model's presence in the *Matlab* path.

#. Create a new *Bachman* Empty project within *SolutionCenter*, following the steps in the :ref:`Create Bachmann Applications <user_guide_appendix_createbachmann>` section. Call the application *calavg*. Ensure the main application source file is copied into the reference C folder, located in "ProjectName\\ReferenceCfiles\\Originals." If the source file name, without the appendix "_app.c" is different than the project name, update it by clicking "Details" and changing the field *refC_name* to the new name (e.g., *calavg* in the present case). A green "*C ref found*" light indicates the presence of the reference C file in the *Matlab*'s path.

#. Right-click on the "*C ref found*" light and select "*Add Matlab Fields*" (:numref:`mmr_new3`).  This modifies the reference files, enabling variables interconnection.

    .. figure:: images/mmr_new3.png
       :width: 1000
       :name: mmr_new3

       Add *Matlab* Fields for the SUBMODEL "calc_avg" 

#. Create the *Simulink* model in the appropriate folder. You can either modify the empty one, copy and paste into the existing model, or use the example provided under "Examples\\met_mast_reader_sample\\SimulinkModels\\calc_avg." If you plan to use a custom *Simulink* model, note that some model settings may differ, so it is advisable to copy the content of your model into the newly created one, as previously mentioned.

#. Open the model provided. The model hasthree main inputs: 
    #.	Met mast wind speed at 110m
    #.	Met mast wind speed at 60m
    #.	Met mast wind direction at 110m 
   The reference heights in this example are based on the `IEA Task 37 reference wind turbine <https://github.com/IEAWindTask37/IEA-3.4-130-RWT>`_. The model checks if variables are correctly being transmitted (e.g., ensuring they are not frozen) and then calculates moving averages. Several outputs are computed:

    #.	Moving average of the wind speed at 110m
    #.	Turbulence intensity calculated from the wind speed at 110m
    #.	Moving average of the wind speed at 60m
    #.	Power-law shear exponent
    #.	Moving average of the wind direction at 110m
    #.	Status check to monitor runtime activity

#. The model requires specific quantities to be defined, which can be found in the file “init_calc_avg.m” file
#. Quantities like startup time, moving average interval, and error time are here set to 600 s
#. With these settings in place, the *Simulink* model is ready to use


Create a new ITFC
""""""""""""""""""""""""""""""""

   #. Click on the "Add *ITFC*" button, and in the dialog box that appears, assign the name "*met_mast_ITFC*" to the *ITFC* app.
   #. Similarly to the steps taken for the "*calc_avg*" *SUBMODEL*, you'll need to generate a new empty PLC code. Refer to the :ref:`Create Bachmann Applications<user_guide_appendix_createbachmann>` section for detailed guidance. Within *SolutionCenter*, name this application "*mmitfc*." Copy and paste the reference app file into the "Originals" folder and update the "*refC_name*" to the new name through "Details".

   #. Right-click on the "*C ref found*" light and select "*Add Matlab Fields*" (:numref:`mmr_new3`). 

Create a new HOST
""""""""""""""""""""""""""""""""

#. Click on the "Add *HOST*" button. Assign "*host_calc_avg*," to your new host application *AppName*. 

#. Complete all the required fields by clicking the "Details" button. Detailed guidance on these fields is provided in the :ref:`inputfile definition<user_guide_inputfile_host>` section.

#. Similarly to the previous steps, generate a new empty PLC code for the host application following the instructions in the :ref:`Create Bachmann Applications<user_guide_appendix_createbachmann>` section. Name this source application *hcalc*.

#. Copy and paste the reference app file into the "Originals" folder, updating the details of the app accordingly. Right-click on the *C ref found* light, and from the menu that appears, select "Add Matlab Fields.".

#. Set the *out_filename* field in the "Details" section as *hcalc_outputs* and provide the *output_path_in_PLC* (note that this will be dependent on your PLC folder path). Ensure that the corresponding folder is pre-created on the PLC to prevent potential issues during framework startup.

#. The basic applications – *SUBMODEL*, *ITFC*, and *HOST* – have been successfully created. Save your progress by clicking the “Save” button below, which updates the excel file “inputfile.xlsx.”

.. _user_guide_example1_inputfile:

Modify the "inputfile.xlsx"
""""""""""""""""""""""""""""""""

Open the "inputfile.xlsx" to configure the main settings for the *met_mast_reader* project. Changes made to the *Submodels*, *ITFC*, and *HOST* tabs will be reflected in the GUI upon the next project load.

#. Click on the *Settings* tab. Adjust the *sample_time* parameter to your desired value. Here, 0.1 seconds is chosen. This parameter determines the operating frequency for all the project applications. Note that only *Bachmann* systems can be selected in the *PLC_system* column.

#. Navigate to the *Submodels* tab. In the "*host_apptag*" column for "*calc_avg*", specify "*host_calc_avg*". This designates the host application with which the *SUBMODEL* application exchanges data. As mentioned earlier, it's crucial to use the same host application for every *SUBMODEL*. Save your changes and close the file.

Below, an overview of the different tabs in "inputfile.xlsx" for the "met_mast_reader" project.


.. csv-table::  met_mast_reader - "inputfile.xlsx" - SUBMODEL
   :file: inputfile_met_mast_reader_1.csv
   :header-rows: 1

.. csv-table::  met_mast_reader - "inputfile.xlsx" - ITFC
   :file: inputfile_met_mast_reader_2.csv
   :header-rows: 1

.. csv-table::  met_mast_reader - "inputfile.xlsx" - HOST
   :file: inputfile_met_mast_reader_3.csv
   :header-rows: 1


.. csv-table::  met_mast_reader - "inputfile.xlsx" - Settings
   :file: inputfile_met_mast_reader_4.csv
   :widths: 30, 60, 10
   :header-rows: 1


Modify the “SVI_Definition”
""""""""""""""""""""""""""""""""

In this step, you will define the variables for each application and establish the necessary variable interconnections across different layers. Open "SVI definition.xlsx." Separate description is given for each respective layer.

**ITFC**

As a first step, it is necessary to create a dummy interface. For the current met-mast case, define two variables in the "ITFC":

#. Create a structure named "*met_mast*". Set both *TagName* and *VarName* to "*met_mast*." Set *InputNumber* to 1, ensuring *AppName* matches the *HOST* app in "inputfile.xlsx." Specify *VarType* as "struct," type "READ" in the *Access* field, and set *Create* to true. Note that structures can handle variables of different data types, but their size should be 1 for all numeric variables. You can leave the *VarSize* field empty for now.
#. Define three subvariables under "met_mast":
    #. Wind Speed at 110 m, named "*ws_110m*"
    #. Wind Speed at 60 m, named "*ws_60m*"
    #. Wind Direction at 110 m, named "*wd_110m*"

    For each subvariable, ensure *InputNumber* matches that of the structure, leave *AppName* empty, set *VarType* as "double," *VarSize* to 1, and mark *Create* as true. The *Access* field can be left empty since it will be inherited from the structure variable.

#. Create a variable named "*exchange_data_mm*" to store and write output values from the *HOST* to the *ITFC*. Set *InputNumber* to 2, *VarType* to double, *VarSize* to 5, and *Access* to "WRITE".

The resulting sheet can be viewed in the following :ref:`table<SVI_Definition_itfc_mmreader>`.

.. csv-table::  met_mast_reader - "SVI_Definition.xlsx" - ITFC
   :file: SVI_Definition_itfc_mmreader.csv
   :header-rows: 1
   :name: SVI_Definition_itfc_mmreader

**HOST**

The "*host_calc_avg*" app is equipped with 10 variables:

#. Create three variables corresponding to the inputs of your *Simulink* model. These variables, being read from the interface structure, necessitate defining *parent_App*,*parent_TagName*, and *parent_SubVar*. Set *Action* to "READ" and *output_freq* as "fast" for these input variables.

#. Specify the outputs of the Simulink model, which comprises six variables. Among these, five are exchanged with the interface. Therefore, provide information in the *parent_App*, *parent_TagName*, and *parent_SubVar* fields for these. The last variable, "*avg_inflow State*, will not be written back to the interface. For the averaged quantities, set *output_freq* as "slow," while for the “*avg_inflowState*,” can be printed at “ctrl” frequency.

#. Include a mandatory “*AppStatus*” variable for each submodel in the framework. As there is a single submodel named “calc_avg” in this example, add the variable “avg_inflow_AppStatus.” The type of this quantity can be a 16-bit unsigned integer. Output this variable at “ctrl” frequency.

The resulting sheet can be viewed in the following :ref:`table<SVI_Definition_host_mmreader>`.

.. csv-table::  met_mast_reader - "SVI_Definition.xlsx" - HOST
   :file: SVI_Definition_host_mmreader.csv
   :header-rows: 1
   :name: SVI_Definition_host_mmreader

**SUBMODEL**

The SUBMODEL "calc_avg" must be defined, according to the input port of the underlying *Simulink* model and its application status.

#. Define three variables, corresponding to the inputs of the Simulink model. Set *IO* as "input." Specify the fields *parent_App* and *parent_TagName*. Ensure that the *Action* is set as "READ.”
#. Define six variables corresponding to the outputs of the Simulink model. Set *IO* as "output." Specify the fields *parent_App* and *parent_TagName*. Ensure that the *Action* is set as "WRITE.”
#. Correctly define the *PortNumber* for both input and output variables based on the port numbering of the *Simulink* model. Additionally, ensure that the *PortName* matches the names in the *Simulink* model.
#. Create an "*AppStatus*" variable, denoted by the *IO* field set as "status." This variable doesn't require a port number, and the port name should be set as "*AppStatus*." Set this variable to "WRITE."

Remember to save and close the “SVI_Definition” file after making these modifications.


 .. csv-table::  met_mast_reader - "SVI_Definition.xlsx" - SUBMODELS
    :file: SVI_Definition_sm_mmreader.csv
    :header-rows: 1
    :name: SVI_Definition_sm_mmreader

Generate the test interface
""""""""""""""""""""""""""""""""

After modifying the "SVI_Definition.xlsx," the project requires reloading. An error will now occur, indicating:

    ITFC App "met_mast_ITFC" - variable "met_mast" is a struct of size NaN Bytes, but its subvariables sum up to 24. Please check the excel file.


This error results from leaving the “VarSize” field of the ITFC variable “met_mast” empty. To resolve this, insert 24 in the field (3 doubles of 8 bytes each).

Now, proceed with creating a dummy interface:

    #. Open the details of the ITFC app "met_mast_ITFC" and fill in the "test_ITFC_filename" for your ITFC app, including any relative path. Create this file in "./Examples/met_mast_reader/mm_ITFC.mat."

    #. Click on the "Create Random ITFC" button. Specify the duration of the time histories; the default is 100 seconds. Keep in mind storage considerations on your PLC. If the duration is too long, it might pose storage issues on your PLC. However, in this case, since we have only a few variables, it shouldn't be a problem. Let's set it to 1000 seconds and click "OK."

    #. A new "mm_ITFC.mat" file is generated. If you open it, you will find the "met_mast" structure and the "exchange_data_mm" array. Initially, these variables are populated with random data based on their data type. However, this might not be realistic for wind speed and direction data. You have two options: replace the random data with your own or use the provided dummy variables included within the folder "Examples\\met_mast_reader_sample", which contains with realistic data. By default, only the "read" variables are filled with random numbers, while "WRITE" variables are set to 0.

    #. Open the details of your ITFC_App and set its *Flag_Create_test_ITFC* to TRUE. Click on "Load ITFC."

.. _user_guide_example1_generateplccode:

Generate the PLC code
""""""""""""""""""""""""""""""""

To generate the PLC code, make sure all "*Generate PLC*" checkboxes are ticked, save your project, and then click on the "*Generate PLC*" button at the bottom of the GUI.

As a result, three folders will be created under the directory labeled *PLCApps*. To complete this process, transfer the contents of each of these folders into the corresponding applications folder within your PLC path.

.. _user_guide_example1_test:

Testing the framework
""""""""""""""""""""""""""""""""


#. Now, you can run the applications on the PLC through the *Bachmann SolutionCenter*. It is critical to follow a specific installation order: start with the *ITFC*, proceed to *HOST*, and conclude with *SUBMODEL* apps.

#. Within the *HOST* applications, a variable called *Flag_Record* is created. Use it to control the generation of output files. Wait now for the test to complete, which will take 1000 seconds. Once done, copy the output files back to the *Matlab* folder of the project.

#. In the GUI "Test" tab, generate a ".mat" file that aggregates the *HOST* outputs by clicking "Generate MAT file from HOST outputs." Select all the outputs (FAST, SLOW, and CTRL). This creates a file named "hcalc_outputs.mat."

#. Choose the HOST "*host_calc_avg*" from the dropdown menu, and load its data through the "Load Host data" button, using the ".mat" file you just created. All the host variables will be visible in the dropdown menu.

#. Select a variable, such as "*mm_ws_110m*." You will see two curves, one from the *ITFC* data and the other from the *HOST* application. Use the figurer to verify whether the two signals are identical. In the present case, you will see something like this:

.. figure:: images/mmr_new4.png
   :width: 1000
   :name: mmr_new4

   Data trasmission error: excessive *HOST* recording time

At the end of the time history, unphysical values are provided by the "*host_calc_avg*" application. This anomaly occurs because, after the 1000 seconds embedded in the ITFC application, random values are transmitted to the HOST application. To rectify this, you'll need to trim the unnecessary data. The "Brush Data" feature facilitates this process.

    #. Use "Brush Data" to select the data you wish to retain.
    #. Right-click and choose "Brush Data / Sync ITFC App."

.. figure:: images/mmr_new5.png
   :width: 1000
   :name: mmr_new5

   Data brushing to trim time history endings

The signals might still be affected by a time shift because the *ITFC* application was started earlier than the *HOST* application.

.. figure:: images/mmr_new6.png
   :width: 1000
   :name: mmr_new6

   Data brushing to trim time history endings

To correct this time shift, click "Find delay" and manually adjust the *ITFC* signal using the "+” and “-” buttons. Once synchronization is achieved, select the data to keep and use the "*Brush Data / Sync ITFC App*" function once more. This function will synchronize all variables originating from the same *ITFC* application, eliminating the need to do it individually for each *HOST* variables, such as "*mm_ws_60m*" and "*mm_wd_110m*." After this synchronization, check whether the data are perfectly coincident or if there are still some transmission errors, which may be a sign of incorrect setup of the "SVI_Definition.xlsx" or some PLC errors. 

.. figure:: images/mmr_new7.png
   :width: 1000
   :name: mmr_new7

   Syncronized data

When you select "*avg_ws_110m*" or any variable that should be an output of a *Simulink* model, you might observe that only the "*host_calc_avg*" results are visible. To check whether the *Simulink* model works the same way in both the PLC and *Matlab*, run the *Simulink* model by clicking "Run Simulink models." This uses the same inputs as the host application, allowing you to compare and ensure the C model is functioning correctly. Simulink outputs will be displayed as well.

.. figure:: images/mmr_new8.png
   :width: 1000
   :name: mmr_new8

   PLC vs Simulink time histories for the entry "avg_ws_60m"

The comparison should align well after the initial ramp-up time required for the moving average. Verify the last two variables, "*avg_inflow_AppStatus*," which should be 0 when the submodel "*calc_avg*" was not running and 1 otherwise, and "avg_inflowState," which should be 0 when the submodel is running correctly and a number between 1 and 7 otherwise.

Save your results by clicking on "Save" with the desired output file name to generate a ".mat" file for future reference.
This concludes the present tutorial. Your applications are now ready for execution.