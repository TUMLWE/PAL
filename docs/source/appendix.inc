.. _user_guide_appendix_createbachmann:

Create Bachmann Applications 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To create e new PLC application in *Bachmann*, open the *Bachmann SolutionCenter* and follow the steps:


   #. In the Solution Navigator: right click C/C++ projects, new, C Project.
   #. Define a project name, which could be any name. Under Project Type, select Bachmann module -> C Template. Under Toolchains, select Bachmann 2.95 Toolchain. Next.
   #. Insert your module name, which should be your "AppName". Check the “SVI client for reading SVI variables”. Finish.
   #. A new C project will be created. Under its “Source Files” folder, copy the “AppName_app.c” file into “ReferenceCFiles\\Originals”. IMPORTANT: this file name, without the appendix "_app.c" is the "refC_name" to be written in "inputfile.xlsx". This is the main application file, which needs to be modified to include the app interconnections.