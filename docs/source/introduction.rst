Introduction
=============

The objective of PAL is to provide a *Matlab*-based framework for the development, deployment, and testing of interconnected applications (provided as *Simulink* models), to be executed on real-time PLC systems. As of the present release, deployment is only possible on `Bachmann M1 systems <http://www.bachmann.info>`_, therefore the sections relative to deployment and testing of the present manual are addressed to such systems. Compatibility with other systems may be added in the future.

The present tool is developed by the **Wind Energy Institute** of the **Technical University of Munich**. Therefore, particular effort was put into addressing specific issues of research projects, including:

#. **Easier Model Deployment:**
   *Simulink* and *Matlab* are powerful tools for researchers, offering a user-friendly environment for designing and testing new models and theories. In research projects, it is common to deploy these models on real-time systems, which are the industry standard for industrial automation. This framework allows researchers, even those without in-depth PLC knowledge, to deploy applications on real-time machines.

#. **Streamlining Model Deployment:**
   Typically, models are handed over to the industrial partner, who then handles the deployment process. This can create an additional burden for the industrial partners, who need to understand the model and have access to *Matlab*, *Simulink*, and other associated products. This situation becomes even more complex when multiple research institutions collaborate with the same industrial partner. However, this framework provides a solution by enabling the establishment of a 'shared' PLC, capable of providing a unified point of contact for research institute, facilitating data exchange between partners.

#. **Confidentiality and Modular Development:**
   Live experiments often require access to data from various sensors, actuators, and hardware-specific sources. However, industrial partners may be hesitant to share this data due to confidentiality concerns. Conversely, hardware-specific details may hold limited relevance for researchers, whose primary focus lies in the overall research outcomes of the experiment. The modularity of this framework offers distinct advantages by enabling a clear separation between hardware-specific applications from application-specific ones, which can be developed by different partners. Effective communication between these two layers is ensured through a predefined set of shared variables, which is agreed between the partners.

#. **Enhancing Research Cooperation:**
   Collaboration with other research institutions is desirable and beneficial within research projects. The present approach allows all research-based applications to access the industrial hardware from a single point, simplifying the coordination process and reducing the overall implementation burden.

#. **Adaptability for Runtime Changes:**
   Research experiments often require runtime adjustments to models. The framework's modular approach enables the substitution of individual components without requiring the halt of the entire framework's execution. This flexibility is crucial, especially when the framework includes applications owned by various research or industrial partners.

#. **Monitoring Outputs:**
   The framework generates simple text outputs that can be used for monitoring the progress of experiments.

The manual is structured into three main sections. The first section provides an overview of the general framework architecture, explaining the underlying structure and data flow. The second section in-depth information about the development of a new project, while the third section explains the components of the graphic user interface (GUI). The last section of this manual offers guidance through two distinct examples of framework implementation. As the framework was developed by the Wind Energy Institute at the Technical University of Munich, the examples are related to wind energy applications. The first example demonstrates the implementation of a Met Mast Data Reader, while the second example covers a SCADA Data Reader.




