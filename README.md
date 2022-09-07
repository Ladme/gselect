# gselect: Selecting Atoms From Gromacs Simulations

Selects a specified group of atoms from a simulation frame or a trajectory. Outputs the selected atoms into a new `gro` file and `xtc` file (if requested). Uses [groan selection language](https://github.com/Ladme/groan#groan-selection-language).

## Dependencies

`gselect` requires you to have groan library installed. You can get groan from [here](https://github.com/Ladme/groan). See also the [installation instructions](https://github.com/Ladme/groan#installing) for groan.

## Installation

1) Run `make groan=PATH_TO_GROAN` to create a binary file `gselect` that you can place wherever you want. `PATH_TO_GROAN` is a path to the directory containing groan library (containing `groan.h` and `libgroan.a`).
2) (Optional) Run `make install` to copy the the binary file `gselect` into `${HOME}/.local/bin`.

## Options

```
Usage: gselect -c GRO_FILE -s SELECTION [OPTION]...

OPTIONS
-h               print this message and exit
-c STRING        gro file to read
-s STRING        selection of atoms
-f STRING        xtc file to read (optional)
-n STRING        ndx file to read (optional, default: index.ndx)
-o STRING        output file name (default: selection.gro / selection.xtc)
```

## Example usage

```
gselect -c md.gro -s "name CA and resname LEU"
```
This command will select all atoms with name 'NA' belonging to residues named 'LEU' from `md.gro`. The selected atoms will be output into `selection.gro` (default option).

```
gselect -c md.gro -f md.xtc -s "Backbone and resname SER" -n groups.ndx -o my_selection
```
This command will select all atoms belonging to ndx group 'Backbone' and at the same time belonging to residues with name 'SER'. Ndx groups will be read from `groups.ndx`. Select atoms from `md.gro` will be saved into `my_selection.gro` and a file `my_selection.xtc` will be written containing only the coordinates of the selected atoms.

In case the selection query corresponds to no atoms, `gselect` will inform the user about this fact but it will still write the output files.

## Limitations

Only tested on Linux. Probably will not work on anything that is not UNIX-like.
