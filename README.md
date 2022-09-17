# gselect: Selecting Atoms From Gromacs Simulations

Selects a specified group of atoms from a simulation frame or a trajectory. Outputs the selected atoms into a new `gro` file and `xtc` file (if requested). Uses [groan selection language](https://github.com/Ladme/groan#groan-selection-language).

## Dependencies

`gselect` requires you to have groan library installed. You can get groan from [here](https://github.com/Ladme/groan). See also the [installation instructions](https://github.com/Ladme/groan#installing) for groan.

## Installation

1) Run `make groan=PATH_TO_GROAN` to create a binary file `gselect` that you can place wherever you want. `PATH_TO_GROAN` is a path to the directory containing groan library (containing `groan.h` and `libgroan.a`).
2) (Optional) Run `make install` to copy the the binary file `gselect` into `${HOME}/.local/bin`.

## Options

```
Usage: gselect -c GRO_FILE [OPTION]...

OPTIONS
-h               print this message and exit
-c STRING        gro file to read
-s STRING        selection of atoms (default: all)
-f STRING        xtc file to read (optional)
-n STRING        ndx file to read (optional, default: index.ndx)
-o STRING        output file name (default: selection.gro / selection.xtc)
-r STRING        reference atoms for geometry selection (optional)
-g STRING        query for geometry selection (optional)
```

## Basic selection

```
gselect -c md.gro -s "name CA and resname LEU"
```
This command will select all atoms with a name `NA` belonging to residues named `LEU` from `md.gro`. The selected atoms will be output into `selection.gro` (default option).

```
gselect -c md.gro -f md.xtc -s "Backbone and resname SER" -n groups.ndx -o my_selection
```
This command will select all atoms belonging to ndx group `Backbone` and at the same time belonging to residues with name `SER`. Ndx groups will be read from `groups.ndx`. Selected atoms from `md.gro` will be saved into `my_selection.gro` and a file `my_selection.xtc` will be written containing only the coordinates of the selected atoms.

In case the selection query corresponds to no atoms, `gselect` will inform the user about this fact but it will still write the output files.

## Geometry selection

`gselect` can be used to select atoms based on their positions in the simulation box. Geometry selection is supported only for gro files.

The geometry selection is provided using the flag `-g` in the following format:
```
GEOMETRY_TYPE OPTIONS...
```
The character and the number of options differs between the geometry types.

Currently, `gselect` supports five geometry types:
1) xcylinder, ycylinder, zcylinder: selects atoms located inside a cylinder oriented along the x, y, or z axis, respectively.
2) sphere: selects atoms located inside a sphere
3) box: selects atoms located inside a box

Geometry types `cylinder` require the following query format:
```
(x/y/z)cylinder RADIUS MIN-MAX
```
where `RADIUS` corresponds to the radius of the cylinder, `MIN` specifies the position of the bottom of the cylinder in the corresponding axis and `MAX` specifies the top of the cylinder in the corresponding axis.

Geometry type `sphere` requires the following query format:
```
sphere RADIUS
```
where `RADIUS` is the radius of the sphere.

Geometry type `box` requires the following query format: 
```
box MINX-MAXX MINY-MAXY MINZ-MAXZ
```
where `MIN?` and `MAX?` specify the span of the box in the corresponding axis.

Note that spaces in the range specification (e.g. `MINX-MAXX`) are NOT supported (i.e. `MINX - MAXX` is not valid).

The cylinder, sphere, or box is placed to the origin of the simulation box (coordinates `x = 0, y = 0, z = 0`), unless a reference is provided using the flag `-r`. The reference can either be a geometric center of a selection of atoms specified using the groan selection language or it can be a point in space specified as `point X Y Z` (e.g. `point 3.2 4.3 5.4`).


### Examples
```
gselect -c md.gro -s "resname POPE" -g "zcylinder 2.5 -2-2" -r Protein
```
This command will select atoms corresponding to residues named `POPE` that are located inside a z-axis-oriented cylinder positioned in the geometric center of the selection `Protein` (ndx group read from `index.ndx`). This cylinder will have a radius of 2.5 nm and it will span from -2 nm to 2 nm on the z-axis relative to the center of the `Protein` selection. The selected atoms will be written into `selection.gro` (default option).

```
gselect -c md.gro -g "sphere 3.2" -r Protein
```
This command will select _all_ atoms (default option of the flag `-s`) that are located within 3.2 nm from the geometric center of `Protein`.

```
gselect -c md.gro -s "resname SOL" -g "box 3-8 0-8 2-11"
```
This command will select atoms corresponding to residues named `SOL` which x-coordinate lies between 3 and 8 nm, which y-coordinate lies between 0 and 8 nm, and which z-coordinate lies between 2 and 11 nm.

```
gselect -c md.gro -r "point 5 5 5" -g "sphere 3.2"
```
This command will select _all_ atoms that are located within 3.2 nm from the point at coordinates `x = 5 nm, y = 5 nm, z = 5 nm`.

## Limitations

For geometry selection assumes that the simulation box is rectangular and that periodic boundary conditions are applied in all three dimensions.

Always uses center of geometry instead of center of mass.

Only tested on Linux. Probably will not work on anything that is not UNIX-like.
