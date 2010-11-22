Build instructions for Qt agent (qttas):

Important note:
Mobility fixture is now default in the Agent_Qt, so now in Windows and Linux one needs to install mobility API.

If one needs to compile without mobility, "qmake -r CONFIG+=no_mobility" disables mobility fixture in Agent_Qt build.

1.0 version sources can be found from:
http://qt.nokia.com/products/appdev/add-on-products/catalog/4/new-qt-apis/mobility

Plain Qt:
1. qmake
2. make (mingw32-make)
3. make release (mingw32-make) (optional if you use release builds)
4. make install (mingw32-make release)

Orbit:
1. qmake "CONFIG+=orbit"
2. make (mingw32-make)
3. make release (mingw32-make) (optional if you use release builds)
4. make install (mingw32-make release)

Dui(Maemo):
1. qmake "CONFIG+=maemo"
2. make 
3. make release (optional if you use release builds)
4. make install 

Symbian (s60 port needed)
1. qmake / qmake "CONFIG+=orbit" (if orbit used)
2. bldmake bldfiles
3. abld build armv5 udeb (or what ever your target is...)

Notes:
Currently only debug or release builds are supported. Use one or the other.

Testability loading can be done in several methods:

1. Application loads the plugin
Implement the plugin loading inside the application. See documentation for more details.

2. Style plugin
By default the build will make a style plugin called "testabilitystyle" which will load testability.
The plugin is enabled in the command line: "yourapphere -style testabilityplugin" 
(yourapphere naturally means the name of your app executable)

3. Testability plugin
By default the build will make a plugin called "testability" which will load testability.
Load the plugin in your app and call Initialize. 

4. Orbit and Dui
Orbit and Dui applications load the plugin buy deafault so no action required.

Logging:
Create /logs/testability folder in your target machine to enable logging. Two logs will be created
one for qttasserver and one for the application under testing.

