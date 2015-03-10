#!/usr/bin/env python2

import sys

from evaluation import Evaluation



if __name__ == "__main__":
    #
    # Check the number of command line arguments
    #
    if len (sys.argv) < 8:
        sys.stderr.write ("\nUsage: %s [pwr conf file] [dl pl dir] [ul pl dir] [bs pl file] [height] [width] [output_prefix]\n" % sys.argv[0])
        sys.stderr.write ("\nDumps data to create a 2D map, based on the given data.\n")
        sys.stderr.write ("The output file is saved using the given output prefix.\n\n")
        sys.exit (1)
    else:
        #
        # initialize the evaluation object
        #
        obj_eval = Evaluation (*sys.argv[1:7])
        obj_eval.init_gpu ( )

        #
        # dump state
        #
        cell_conf = [float (c.pilot_pwr) for c in obj_eval.cell_list]
        obj_eval.dump_2D_map (cell_conf, '%s_state.dat' % sys.argv[7])

        print "Done writing data to '%s_state.dat'" % sys.argv[7]
        print "\nLegend:"
        print "\t0 = not covered"
        print "\t1 = normal SHO"
        print "\t2 = not SHO DL, SHO UL"
        print "\t3 = SHO DL, not SHO UL"
        print "\t5 = covered, no SHO"
