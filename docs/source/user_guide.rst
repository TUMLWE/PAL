User Guide
===========

.. contents::

General Framework Architecture
------------------------------

The present framework adopts a structured three-layer approach. These are named *ITFC*, *HOST*, and *SUBMODEL*. Each layer contains specific applications, each designed to serve a distinct purpose:

**ITFC:**
*Interfaces* are the lowest-level applications, responsible for tasks such as sensor reading and providing input to actuators, effectively connecting to the physical layer. Ideally, *ITFC* apps can be developed by the owner of the hardware. The use of an *ITFC* layer is only necessary when data exchange towards sensors or equipment is required. Multiple *ITFC* apps are permitted. *ITFC* applications share variables through a shared variable interface (SVI), and these can be read from and written to by a *HOST* app. 

When created within PAL, *ITFC* apps solely consist of user-defined time histories for predefined variables. As a result, their function is restricted to serving as tools for testing interfaces; they won't operate live, and no connections to sensors or actuations are established. The utility of *ITFC* apps in PAL relies entirely on the SVI they provide, which is exchanged with the HOST layer and used for testing. This SVI **must** precisely replicate the one from the *ITFC* app developed by the hardware owner, as it is the version that will be active during live applications.

**HOST:**
This application operates in the middle layer and is responsible for tasks like data reading and writing to and from *ITFC* applications. *HOST* also manages variables crucial for experiment execution, monitoring, and communication between *SUBMODEL* applications. It generates output files in text format. Please note that as of release 1.0, each project supports only one *HOST* application. Data sharing is performed through a SVI, allowing read and write access by *SUBMODEL* apps.

**SUBMODEL:**
This is the top layer, comprising the primary application models, based on *Simulink* models. *Simulink* inputs and outputs are connected to an *HOST* SVI. Any number of *SUBMODELS* can be implemented within a project.

One sampling frequency must be specified for the execution of the whole framework. As of release 1.0, different frequency execution for different applications is not supported. An exemplary sketch of the framework data flow is shown in :numref:`data_flow_chart`.

Empty C source projects should be generated for each PLC application through *Bachmann SolutionCenter*. These source codes are then automatically modified by the framework to incorporate all the necessary variable interconnections required by the different applications.

The framework is equipped with a :ref:`Graphic User Interface (GUI)<user_guide_GUI>` that simplifies the development and testing process and is divided into :ref:`Develop/Deploy<user_guide_gui_dd>` and :ref:`Test<user_guide_gui_test>`.

.. figure:: images/data_flow_chart.png
   :width: 1000
   :name: data_flow_chart

   Data flow between the different layers


.. _user_guide_project_definition:

Project Definition 
-----------------------

When a new project is created using the :ref:`Graphical User Interface (GUI)<user_guide_GUI>`, a folder is automatically generated with the project name. Two additional folders, **ReferenceCFiles** and **SimulinkModels**, are created to store the reference PLC source code and necessary *Simulink* models for the *SUBMODEL* apps.

Additionally, two *Excel* files, **inputfile.xlsx** and **SVI_definition.xlsx** are created along with the folders. These files are crucial for project definition, describing the layers of the applications architecture, and explaining how data is exchanged among each application. Both these files are described in the following paragraphs.


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

.. _user_guide_example1:

.. include:: example_1.inc

.. _user_guide_example2:

.. include:: example_2.inc

Appendix
-----------------------

.. include:: appendix.inc

.. include:: types.inc