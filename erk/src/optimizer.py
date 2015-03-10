import sys
import copy
import math
import random

from csv import *
from collections import deque

from anneal import Annealer



def printHelp (msg = None):
    """
    Displays the error message received as parameter, 
    followed by a usage message.-
    """
    if (msg is not None):
        sys.stderr.write (msg)

    sys.stderr.write ("\nUsage: optimizer.py [power configuration file] [SHO events file] [temperature] [steps]\n")
    sys.stderr.write ("\nOptimizes the difference (in dB) between UL and DL areas.\n")
    sys.stderr.write ("\n[temperature] Is the starting temperature of the algorithm.\n")
    sys.stderr.write ("[steps]       The number of iterations between every temperature change.\n")



def random_cpich_setting (curr_conf_dict, init_conf_dict, neigh_matrix, reset_cpich=False):
    """
    Changes the CPICH of a randomly-picked cell. The changes are limited 
    to an interval of +-2 dB of the initial configuration of the cell.
    Also, we do not change the CPICH of a cell with ASC which neighbour 
    also has ASC amplifier, since changes in such situations do not lower
    the power difference between them.
    The parameter 'reset_cpich' indicates whether to reset the CPICH of
    the randomly selected cell to its initial configuration. This is useful
    to help the search move out of a "dead-end".-
    """
    cells = curr_conf_dict.keys ( )
    cell_num = len(cells)
    max_trials = (cell_num ** 2) / 2
    changed = False
    trial = 0

    while (not changed and trial < max_trials):
        random_elem = random.randint (0, cell_num - 1)
        random_cell = cells[random_elem]
        init_pwr    = init_conf_dict[random_cell]['dl_pwr']

        if (reset_cpich):
            curr_pwr = init_pwr
        else:
            curr_pwr = curr_conf_dict[random_cell]['dl_pwr']

        #
        # check if we are still within the (-2dB, +2dB) change interval
        #
        if (abs (init_pwr - curr_pwr) < 20):
            #
            # randomly select a neighbour of this cell, if it has any
            #
            neighbours = neigh_matrix[random_cell]

            if (len(neighbours) > 0):
                random_elem = random.randint (0, len(neighbours) - 1)
                random_neigh = neighbours[random_elem]
            else:
                trial += 1
                continue
            #
            # if both cell and neighbour have ASC amplifier,
            # we select a different cell-neighbour pair
            #
            if ((curr_conf_dict[random_cell]['cable'] > 0) and
               (curr_conf_dict[random_neigh]['cable'] > 0)):
                trial += 1
                continue
            #
            # calculate the power difference between cell and neighbour
            #
            diff    = curr_pwr
            diff   -= curr_conf_dict[random_neigh]['dl_pwr']
            dl_diff = abs (diff)
            diff    = curr_pwr + curr_conf_dict[random_cell]['cable']
            diff   -= curr_conf_dict[random_neigh]['dl_pwr'] + \
                      curr_conf_dict[random_neigh]['cable']
            ul_diff = abs (diff)

            #
            # any change needed?
            #
            if ((dl_diff - ul_diff) != 0):
                #
                # apply a random change (-1, 0 or +1)
                #
                curr_conf_dict[random_cell]['dl_pwr'] = curr_pwr + random.randint (-1, 1)
                changed = True
            else:
                trial += 1
        else:
            #
            # the setting change limit of this cell
            # has been exceeded, choose another one
            #
            trial += 1
    #
    # Has anything been changed?
    #
    if (not changed):
        #
        # Rerun resetting a random CPICH setting to its initial value
        #
        random_cpich_setting (curr_conf_dict, init_conf_dict, neigh_matrix, True)



def calculate_power_difference (conf_dict, neigh_matrix):
    """
    Calculates the absolute difference (in dB) between UL and DL powers
    of neighbouring cells. Extra care has to be taken not to sum the
    same relation twice.-
    """
    processed_cells = []

    #
    # for each cell in the network ...
    #
    total_diff = 0
    for cell in conf_dict.keys ( ):
        for neigh in neigh_matrix[cell]:
            #
            # ignore neighbours that have already been processed
            #
            if (neigh not in processed_cells):
                #
                # calculate the power difference between cell and neighbour
                #
                diff       = conf_dict[cell]['dl_pwr']
                diff      -= conf_dict[neigh]['dl_pwr']
                dl_diff    = abs (diff)
                diff       = conf_dict[cell]['dl_pwr'] + \
                             conf_dict[cell]['cable']
                diff      -= conf_dict[neigh]['dl_pwr'] + \
                             conf_dict[neigh]['cable']
                ul_diff    = abs (diff)
                total_diff += abs (dl_diff - ul_diff)
        #
        # append this cell to the list of processed ones
        #
        processed_cells.append (cell)
    
    return total_diff



def build_adjacency_matrix (cellConfDict, shoFileName):
    """
    Returns the adjacency matrix representing an undirected graph
    containing all neighbour relations. The matrix is built using 
    the same order of cells as in the configuration dictionary. 
    Therefore, the element at (0,0) is the first cell appearing in 
    the configuration dictionary and contains a zero (i.e. no cell
    is neighbour of itself).
    The matrix is kept as a dictionary of lists containing cell names.-
    """
    adj_matrix = dict ( )
    for cell in cellConfDict.keys ( ):
        adj_matrix[cell] = []
    
    try:
        #
        # Create a CSV reader for the SHO file
        #
        sho_rdr = DictReader (open (shoFileName, 'r'), delimiter=' ')

    except:
        sys.stderr.write ("Error while opening SHO file!\n")
        sys.stderr.write (shoFileName)
        sys.exit (1)

    #
    # Walk through the SHO file, saving only the neighbour
    # relations of the cells for which we have a configuration
    #
    for line in sho_rdr:
        if (line['cell'] in adj_matrix.keys ( )):
            if (line['neigh'] in adj_matrix.keys ( )):
                adj_matrix[line['cell']].append (line['neigh'])
    
    return adj_matrix



def build_configuration_dict (confFile):
    """
    Returns a cell configuration dictionary 
    based on the data read from the input file.-
    """
    try:
        pwrRdr = DictReader (open (confFile, 'r'), delimiter=' ')
    
    except:
        sys.stderr.write ("Error while opening file!\n")
        sys.stderr.write (confFile)
        sys.exit (1)

    #
    # Create a cell configuration dictionary {'cell' : {'dl_pwr' : int, 
    #                                                   'cable'  : int}
    #
    cell_conf = dict ( )

    #
    # Save the power related configuration to the cell dictionary
    #
    for line in pwrRdr:
       cellName = line['cell']
      
       if not (cell_conf.has_key (cellName)):
          cell_conf[cellName] = dict ( )
           
       cell_conf[cellName]['dl_pwr'] = int (line['pilot'])
      
       #
       # if the BS is equiped with ASC, we annotate the cable gain
       #
       if (int (line['asc']) == 1):
          cell_conf[cellName]['cable'] = int (line['kabel'])
       else:
          cell_conf[cellName]['cable'] = 0

    return cell_conf



def display_cell_conf (cell_conf_dict):
    """
    Prints out the cell configuration dictionary.-
    """
    print "Cell configuration\n------------------------------\n",
    
    for cell in cell_conf_dict.keys ( ):
        print cell,
        conf = cell_conf_dict[cell]
        print conf['dl_pwr'], conf['cable'] 



def display_neighbour_matrix (neighbourMatrix, cellConfDict):
    """
    Prints out the neighbour matrix.-
    """
    print "Neighbour adjacency matrix\n------------------------------\n ",
   
    #
    # title line for matrix columns
    #
    cell_names = cellConfDict.keys ( )
    for cell in cell_names:
        print cell,
    print
    
    #
    # matrix rows
    #
    cell_names = cellConfDict.keys ( )
    for cell in cell_names:
        print cell,

        neigh_names = cellConfDict.keys ( )
        for neigh in neigh_names:
            if (neigh in neighbourMatrix[cell]):
                print 1,
            else:
                print '.',
        print
    print



#
# This script's entry point.-
#
if __name__ == "__main__":
    #
    # A warning about the numbers calculated
    #
    sys.stderr.write ("*** WARNING: CPICH powers are in expressed in tens of units (e.g. 18 => 1.8)\n\n")

    #
    # Check the number of command line arguments
    #
    if (len (sys.argv) < 5):
        printHelp (None)
        sys.exit (1)
    else:
        #
        # Read and process the input parameters
        #
        cellConfDict      = build_configuration_dict (sys.argv[1])
        cellConfDict_init = copy.deepcopy (cellConfDict)
        neighbourMatrix   = build_adjacency_matrix (cellConfDict, sys.argv[2])
        initial_temp      = int (sys.argv[3])
        steps             = int (sys.argv[4])
    
        #display_neighbour_matrix (neighbourMatrix, cellConfDict)
        #display_cell_conf (cellConfDict)
       
        def aneal_move (state):
            """
            Makes a move in the solution space of simulated annealing.
            """
            random_cpich_setting (state, cellConfDict_init, neighbourMatrix)
       
        def aneal_energy (state):
            """
            Calculates the current energy level in simulated annealing.
            """
            return calculate_power_difference (state, neighbourMatrix)
        
        #
        # Minimize the difference between DL
        # and UL powers by simulated annealing 
        #
        state    = cellConfDict
        annealer = Annealer (aneal_energy, aneal_move)
        state, e = annealer.anneal (state, initial_temp, 0.01, steps, steps/1000)
        # state, e = annealer.auto (state, 5, steps)
        
        #
        # Display the optimization results
        #
        print "\nBest CPICH configuration found"
        print "------------------------------"
        
        print "\t \torig\topt"
        for cell in state.keys ( ):
            print "\t%s\t%i\t%i" % (cell, cellConfDict_init[cell]['dl_pwr'], state[cell]['dl_pwr'])
        print "Initial difference => %s\nLowest difference => %s" % (calculate_power_difference (cellConfDict_init, neighbourMatrix),
                                                                     calculate_power_difference (state, neighbourMatrix))
