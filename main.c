// Released under MIT License.
// Copyright (c) 2022 Ladislav Bartos

#include <unistd.h>
#include <groan.h>

// frequency of printing during the calculation
static const int PROGRESS_FREQ = 10000;

static const char VERSION[] = "v2022/09/17";

/*
 * Parses command line arguments.
 * Returns zero, if parsing has been successful. Else returns non-zero.
 */
int get_arguments(
        int argc, 
        char **argv,
        char **gro_file,
        char **xtc_file,
        char **ndx_file,
        char **output_file,
        char **selected,
        char **geometry_reference,
        char **geometry_query) 
{
    int gro_specified = 0;

    int opt = 0;
    while((opt = getopt(argc, argv, "c:f:n:o:s:r:g:h")) != -1) {
        switch (opt) {
        // help
        case 'h':
            return 1;
        // gro file to read
        case 'c':
            *gro_file = optarg;
            gro_specified = 1;
            break;
        // xtc file to read
        case 'f':
            *xtc_file = optarg;
            break;
        // ndx file to read
        case 'n':
            *ndx_file = optarg;
            break;
        // output file name
        case 'o':
            *output_file = optarg;
            break;
        // selected atoms
        case 's':
            *selected = optarg;
            break;
        // reference atoms for geometry selection
        case 'r':
            *geometry_reference = optarg;
            break;
        // query for geometry selection
        case 'g':
            *geometry_query = optarg;
            break;
        default:
            //fprintf(stderr, "Unknown command line option: %c.\n", opt);
            return 1;
        }
    }

    if (!gro_specified) {
        fprintf(stderr, "Gro file must always be supplied.\n");
        return 1;
    }
    return 0;
}

void print_usage(const char *program_name)
{
    printf("Usage: %s -c GRO_FILE [OPTION]...\n", program_name);
    printf("\nOPTIONS\n");
    printf("-h               print this message and exit\n");
    printf("-c STRING        gro file to read\n");
    printf("-s STRING        selection of atoms (default: all)\n");
    printf("-f STRING        xtc file to read (optional)\n");
    printf("-n STRING        ndx file to read (optional, default: index.ndx)\n");
    printf("-o STRING        output file name (default: selection.gro / selection.xtc)\n");
    printf("-r STRING        reference atoms for geometry selection (optional)\n");
    printf("-g STRING        query for geometry selection (optional)\n");
    printf("\n");
}

/*
 * Prints parameters that the program will use for the calculation.
 */
void print_arguments(
        FILE *stream,
        const char *gro_file,
        const char *xtc_file,
        const char *ndx_file,
        const char *output_gro,
        const char *output_xtc,
        const char *selected)
{
    fprintf(stream, "\nParameters for Atom Selection:\n");
    fprintf(stream, ">>> gro file:         %s\n", gro_file);
    fprintf(stream, ">>> selection query:  %s\n", selected);
    if (xtc_file == NULL) fprintf(stream, ">>> xtc file:         ----\n");
    else fprintf(stream, ">>> xtc file:         %s\n", xtc_file);
    fprintf(stream, ">>> ndx file:         %s\n", ndx_file);
    if (xtc_file == NULL) fprintf(stream, ">>> output file:      %s\n", output_gro);
    else {
        fprintf(stream, ">>> output gro:       %s\n", output_gro);
        fprintf(stream, ">>> output xtc:       %s\n", output_xtc);
    }
    fprintf(stream, "\n");
}

int main(int argc, char **argv)
{
    // get arguments
    char *gro_file = NULL;
    char *xtc_file = NULL;
    char *ndx_file = "index.ndx";
    char *output_file = NULL;
    char *default_output = "selection";
    char *selected = "all";
    char *geometry_reference = NULL;
    char *geometry_query = NULL;

    if (get_arguments(argc, argv, &gro_file, &xtc_file, &ndx_file, &output_file, &selected, &geometry_reference, &geometry_query) != 0) {
        print_usage(argv[0]);
        return 1;
    }

    if (xtc_file != NULL && (geometry_query != NULL || geometry_reference != NULL)) {
        fprintf(stderr, "Geometry selection from xtc file is currently not supported.\n");
        return 1;
    }
    
    // prepare output file names
    char *output_gro = NULL;
    char *output_xtc = NULL;

    if (xtc_file == NULL) {
        if (output_file == NULL) {
            output_gro = calloc(strlen(default_output) + 5, 1);
            sprintf(output_gro, "%s.gro", default_output);
        } else {
            output_gro = calloc(strlen(output_file) + 1, 1);
            strcpy(output_gro, output_file);
        }
    } else {
        if (output_file == NULL) {
            output_gro = calloc(strlen(default_output) + 5, 1);
            sprintf(output_gro, "%s.gro", default_output);
            output_xtc = calloc(strlen(default_output) + 5, 1);
            sprintf(output_xtc, "%s.xtc", default_output);

        } else {
            char *output_copy = calloc(strlen(output_file) + 1, 1);
            strcpy(output_copy, output_file);
            char **split = NULL;
            strsplit(output_copy, &split, ".");

            output_gro = calloc(strlen(output_copy) + 5, 1);
            sprintf(output_gro, "%s.gro", output_copy);
            output_xtc = calloc(strlen(output_copy) + 5, 1);
            sprintf(output_xtc, "%s.xtc", output_copy);

            free(output_copy);
            free(split);
        }
    }

    if (xtc_file != NULL) print_arguments(stdout, gro_file, xtc_file, ndx_file, output_gro, output_xtc, selected);

    // read gro file
    system_t *system = load_gro(gro_file);
    if (system == NULL) {
        free(output_gro);
        free(output_xtc);
        return 1;
    }

    // try reading ndx file (ignore if this fails)
    dict_t *ndx_groups = read_ndx(ndx_file, system);

    // select all atoms
    atom_selection_t *all = select_system(system);

    // select selected atoms
    atom_selection_t *selection = smart_geometry(all, selected, geometry_reference, geometry_query, ndx_groups, system->box);
    if (selection == NULL) {
        fprintf(stderr, "Could not understand the selection query.\n");

        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(output_gro);
        free(output_xtc);
        return 1;
    }

    if (selection->n_atoms == 0) {
        fprintf(stderr, "Warning. Selection query corresponds to no atoms.\n");
    }

    // always prepare a gro file with the selected atoms
    FILE *output = fopen(output_gro, "w");

    if (output == NULL) {
        fprintf(stderr, "File %s could not be opened for writing.\n", output_gro);

        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selection);
        free(output_gro);
        free(output_xtc);
        return 1;
    }

    char *comment = calloc(strlen(selected) + strlen(gro_file) + strlen(VERSION) + 200, 1);
    sprintf(comment, "Generated with gselect (C Gromacs Selection Program) %s from file %s.", VERSION, gro_file);
    write_gro(output, selection, system->box, velocities, comment);
    free(comment);

    fclose(output);

    printf("File %s has been written.\n", output_gro);
    free(output_gro);

    // if there is no xtc file supplied, end now
    if (xtc_file == NULL) {
        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selection);
        free(output_xtc);
        return 0;
    }

    // open the input xtc file for reading
    XDRFILE *xtc = xdrfile_open(xtc_file, "r");
    if (xtc == NULL) {
        fprintf(stderr, "File %s could not be read as an xtc file.\n", xtc_file);
        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selection);
        free(output_xtc);
        return 1;
    }

    // check the validity of the xtc file
    if (!validate_xtc(xtc_file, (int) system->n_atoms)) {
        fprintf(stderr, "Number of atoms in %s does not match %s.\n", xtc_file, gro_file);
        xdrfile_close(xtc);
        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selection);
        free(output_xtc);
        return 1;
    }

    // open output xtc for writing
    XDRFILE *xtc_out = xdrfile_open(output_xtc, "w");
    if (xtc_out == NULL) {
        fprintf(stderr, "File %s could not be opened for writing.\n", output_xtc);
        xdrfile_close(xtc);
        dict_destroy(ndx_groups);
        free(system);
        free(all);
        free(selection);
        free(output_xtc);
        return 1;
    }

    // loop through the input xtc file, select atoms from each frame and write them to the output xtc
    int return_code = 0;
    while (read_xtc_step(xtc, system) == 0) {

        // print info about the progress of reading and writing
        if ((int) system->time % PROGRESS_FREQ == 0) {
            printf("Step: %d. Time: %.0f ps\r", system->step, system->time);
            fflush(stdout);
        }

        if (write_xtc_step(xtc_out, selection, system->step, system->time, system->box, system->precision) != 0) {
            fprintf(stderr, "\nWriting has failed.\n");
            return_code = 1;
            goto program_end;
        }
    }
    printf("\nFile %s has been written.\n", output_xtc);

    program_end:
    xdrfile_close(xtc);
    xdrfile_close(xtc_out);
    dict_destroy(ndx_groups);
    free(system);
    free(all);
    free(selection);
    free(output_xtc);

    return return_code;
}