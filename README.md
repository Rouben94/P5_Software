# Software_Projekt_5


Building with SEGGER Embedded Studio
SEGGER Embedded Studio (SES) provides a way of quickly getting the example code up and running with full debug capability.

First time setup
Before building the mesh examples with SEGGER Embedded Studio for the first time, you must complete a one-time setup of the SDK_ROOT macro in SEGGER Embedded Studio. This macro is used to find the nRF5 SDK files.

You can either:

Use the default settings of the SDK_ROOT macro. It defaults to an nRF5 SDK 15.2.0 instance unzipped right next to the mesh folder.
Set the SDK_ROOT macro to a custom nRF5 SDK instance.
To set the SDK_ROOT macro manually in SEGGER Embedded Studio:

Go to Tools -> Options.
Select Building.
Under Build in the configuration list, edit Global macros to contain SDK_ROOT=<the path to nRF5 SDK instance>.
Save the configuration.
You can verify the path by opening one of the source files under the nRF5 SDK file group. If the macro is set correctly, the file opens in the editor window. If not, an error message is displayed with information that the file cannot be found.

For more info on SEGGER Embedded Studio macros, see the SES Project macros page.

Building with SES
To build an example with SEGGER Embedded Studio:

Open the desired project file located in the examples/ folder, for instance examples/light_switch/client/light_switch_client_nrf52832_xxAA_s132_6_1_0.emProject.
Go to Build -> Build < name of the emProject file>, for instance Build light_switch_client_nrf52832_xxAA_s132_6.1.0.
Wait for the compilation to finish.
You can now run the example using SEGGER Embedded Studio.
 
