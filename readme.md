Project still in WIP

Project based on Legoino_1.1.0 library and NimBLE-Arduino_1.4.1

Software based on platformio
  
Laser files:
Model/laser/readyToCut/
	red lines - engraving
	black lines - cut
3d model:
	Model/3d_stl/

How to print color buttons:
1. Use printer with multicolor capabilities :)
2. Use bambulab P1S without AMS and strugle a lot.
3. Print buttons from Model\3d_stl\buttons and inserts from Model\3d_stl\buttons_withinsert and glue together

Toutorial how to print using P1S:
    -Open project from Model\3d_stl\buttons_withinsert with _1.3mf postfix.
    -Print model with bed leveling enabled
    -as soon as model is done, set bed temperature to 70*C (for textured PEI) to keep parts on table
    -remove cleaning line from front of table
    -change filament colr
    -open project _2.3mf postfix.
    -print WITHOUT bed leveling to avoid crushing toolhead into print - it is also recommended to slow down printer to silent mode to have cleaner line
    -Done!
