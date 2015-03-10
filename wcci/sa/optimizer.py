import sys
import copy
import ctypes
import numpy as np
from time import time
from random import randint

from csv import *
from collections import deque

from anneal import Annealer

#
# Full path to the evaluation module
#
EVAL_MODULE_PATH='../eval'



def display_cell_conf (cell_list):
    """
    Prints out the cell configuration.-
    """
    print "------------------------------"
    print "Cell configuration"
    print "------------------------------"
    for c in cell_list:
        print c.name, '\t', 
        print c.pilot_pwr, c.cable_loss, c.has_asc



#
# This script's entry point.-
#
if __name__ == "__main__":
    #   
    # Check the number of command line arguments
    #   
    if (len (sys.argv) < 8): 
        sys.stderr.write ("\nUsage: %s [pwr conf file] [dl pl dir] [ul pl dir] [bs pl file] [height] [width] [start temp] [steps]\n" % sys.argv[0])
        sys.stderr.write ("\nOptimizes the (un)alignment of DL and UL SHO areas by means of simulated annealing.\n\n")
        sys.exit (1)
    else:
        #
        # load the evaluation module
        #
        sys.path.append (EVAL_MODULE_PATH)
        from evaluation import Evaluation
        #
        # initialize the evaluation object
        #
        obj_eval = Evaluation (*sys.argv[1:7])
        obj_eval.init_gpu ( )
        """
        # simulate some evaluations on GPU
        #   
        cell_powers = [c.pilot_pwr for c in obj_eval.cell_list]
        t0 = time ( )
        for i in range (0, 100):
            #
            # randomize cell powers
            #   
            for i in range (0, len (cell_powers)):
                cell_powers[i] += randint (-1, 1) / 10.0
            print obj_eval.calculate_on_gpu (cell_powers)
        t1 = time ( ) - t0
        print "100 evaluations in %s seconds" % (t1)
        """
        #
        # starting temperature of the annealing process (7th parameter)
        #
        initial_temp = int (sys.argv[7])
        #
        # number of steps to run the optimization (8th parameter)
        #
        steps = int (sys.argv[8])
        #
        # making a copy of the original configuration will help
        # us find the gain at the end of the optimization
        #
        Cell_conf_type = ctypes.c_float * len (obj_eval.cell_list)
        cell_conf      = Cell_conf_type ( )
        cell_conf_orig = Cell_conf_type ( )
        for i in range (0, len (cell_conf)):
            cell_conf[i] = np.single (obj_eval.cell_list[i].pilot_pwr)
            cell_conf_orig[i] = np.single (obj_eval.cell_list[i].pilot_pwr)
        #
        # annealing move in C: load the external library
        #
        ext_lib = ctypes.CDLL ('libevaluate.so')

        #
        # DEBUG: make sure all structures are passed correctly to C
        #
        display_cell_conf (obj_eval.cell_list)
        print
        ext_lib.display_cell_conf (cell_conf_orig, len (cell_conf_orig))
        print

        def aneal_move (state):
            """
            A move in the solution space of simulated annealing.
            """
            ext_lib.move (ctypes.byref (state),
                          ctypes.byref (cell_conf_orig),
                          len (state))
       
        def aneal_energy (state):
            """
            Calculates the objective function value for simulated annealing.
            """
            return obj_eval.calculate_on_gpu (state) 

        #
        # dump initial 2D map
        #
        #print "Dumping initial map file ..."
        #obj_eval.dump_2D_map (cell_conf, '/tmp/initial.dat')

        #
        # begin minimization of differences between DL and UL 
        # powers by simulated annealing
        #
        state    = cell_conf
        annealer = Annealer (aneal_energy, aneal_move)
        state, e = annealer.anneal (state, initial_temp, 0.01, steps, steps/1000)
        #state, e = annealer.auto (state, 5, steps)
        
        #
        # dump final 2D map
        #
        #print "Dumping final map files ..."
        #obj_eval.dump_2D_map (state, '/tmp/final.dat')

        #
        # Display the optimization results
        #
        print "\nBest CPICH configuration found"
        print "------------------------------"
        print " \torig\tchg\topt"

        for i in range (0, len (obj_eval.cell_list)):
            chg = state[i] - cell_conf_orig[i]
            print "%s\t%5.2f\t%s%5.2f\t%5.2f" % (obj_eval.cell_list[i].name, 
                                        cell_conf_orig[i], 
                                        '+' if chg > 0 else '',
                                        chg,
                                        state[i])

        print "Original difference => %s" % obj_eval.calculate_on_gpu (cell_conf_orig)
        print "Best difference => %s" % obj_eval.calculate_on_gpu (state)

