-----------------------------
Manual for wave packet viewer
-----------------------------

System requirements: Windows8/8.1/10 with DirectX runtime enabled

- click + mouse drag to view 
- "A"D"W"S" for basic navigation ("Q"E" for up/down movement)
- "R" for placing a new circular wave at the screen center (if looking up, the wave is placed at the scene center)
- "M" to show/hide menues
- the remaining functions are mostly self-explanatory
- the file "TestIsland.bmp" contains the scene (terrain height and water depth in the color channels)

-----------------------------
Manual for the source code
-----------------------------

Stored as Visual Studio 2015 solution.  
Needs the "Eigen" library ( http://eigen.tuxfamily.org )
The actual project is "SimpleSampleDirectXTK" (others are support classes for GUI and DirectX programming).

-"GlobalDefs" global -> variables
-"Packets.h/cpp" -> the complete packet management
-"Render.h/cpp" -> the scene and packet display
-"WavePackets.fx" -> all display/shading functions
-"WavePacketViewer.cpp" -> interface calling packet creating/simulation and rendering

Functions to conveniently create packets are "CreateCircularWavefront" "CreateLinearWavefront" and "CreateSpreadingPacket".


If you have any question, do not hesitate to ask me ( jeschke@stefan-jeschke.com )