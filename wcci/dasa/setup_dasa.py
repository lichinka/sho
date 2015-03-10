#!/usr/bin/env python2.7

import sys
import copy
import math
import random
import ctypes

from csv import *
from collections import deque




class Cell (ctypes.Structure):
    """
    A struct to hold the cell name and its downlink and uplink 
    powers as integers, i.e. power in dBm * 10.
    It also defines comparisson and string representation methods.-
    """
    _fields_ = [("dl_pwr", ctypes.c_float),
                ("ul_pwr", ctypes.c_float)]

    def __cmp__ (self, other):
        return ((self.dl_pwr - other.dl_pwr) * 1000 + 
                (self.ul_pwr - other.ul_pwr))

    def __str__ (self):
        return '%s: %s, %s' % (self.id,
                               self.dl_pwr,
                               self.ul_pwr)


def print_help (command, msg=None):
    """
    Displays the error message received as parameter, 
    followed by a usage message.-
    """
    if msg is not None:
        sys.stderr.write (msg)
    sys.stderr.write ("\nUsage: %s [power conf file] [DASA params file]\n" % command)
    sys.stderr.write ("\nPrepares input data for the minimization of the difference between UL and DL areas,\n")
    sys.stderr.write ("using the DASA algorithm in C, and objective function evaluation in Python.-\n\n")
    sys.stderr.write ("[power conf file]\tFile name and path containing the power settings of the cells being optimized.\n")
    sys.stderr.write ("[DASA params file]\tName and path of the DASA params file to be created.\n")



def read_configuration (confFile):
    """
    Returns a tuple with two elements: 
        - the first is a list containing Cell objects based on the 
          data read from the input file;
        - the second is the cell-name-to-id map, used to assign an
          unique integer value to every cell name.-
    """
    try:
        pwrRdr = DictReader (open (confFile, 'r'), delimiter=' ')
    except:
        sys.stderr.write ("*** ERROR: cannot open file [%s]!\n" % confFile)
        sys.exit (1)
    #
    # a dictionary to map every cell name to a cell id
    #
    name_id_map = dict ( )

    #
    # cell configuration is a list of Cell structures
    #
    #   cell_conf[cell_id].dl_pwr: float
    #   cell_conf[cell_id].ul_pwr: float
    #
    cell_conf = []

    #
    # Save the power related configuration to the cell dictionary
    #
    for line in pwrRdr:
        cell_name = line['cell']
        if cell_name not in name_id_map.keys ( ):
            name_id_map[cell_name] = len (name_id_map)

        cell_id = name_id_map[cell_name]
        dl_pwr = int (line['pilot']) / 10.0
        ul_pwr = 21.0  # max UL pwr at UE 21 dBm

        new_cell = Cell (dl_pwr, ul_pwr)

        #
        # if the base station has no ASC, we annotate the cable loss
        #
        if int (line['asc']) == 0:
            new_cell.ul_pwr -= int (line['kabel']) / 10.0
        cell_conf.append (new_cell)
        assert (cell_conf[cell_id] == new_cell)
    #
    # convert the cell_conf list to an array, 
    # so that C will understand its contents
    #
    arr = (Cell * len (cell_conf)) ( )
    arr[:] = cell_conf
    arr.length = len (cell_conf)
    return (arr, name_id_map)



def write_cell_conf (cell_conf, file_name):
    """
    Outputs the cell configuration in DASA format to 'file_name'.
    This file tells the algorithm what parameters have to be 
    optimized, and in their range.-
    """
    with open (file_name, 'w') as f:
        #
        # file header, including the number of columns
        # and total number of parametes
        #
        f.write ('3 %i\n' % len (cell_conf))
        #
        # the parameters are the cell powers, with an
        # adjustment interval of +-2dB
        #
        for c_id in range (0, len (cell_conf)):
            param = dict ( )
            param['min_pwr'] = cell_conf[c_id].dl_pwr - 2.0
            param['max_pwr'] = cell_conf[c_id].dl_pwr + 2.0
            param['step'] = 0.01
            f.write ('%(min_pwr).1f %(max_pwr).1f %(step).2f\n' % param)
    f.close ( )


#
# This script's entry point.-
#
if __name__ == "__main__":
    #
    # Check the number of command line arguments
    #
    if len (sys.argv) < 2:
        print_help (sys.argv[0], None)
        sys.exit (1)
    else:
        #
        # read the cell configuration file (1st parameter)
        #
        (cell_conf, name_id_map) = read_configuration (sys.argv[1])
        #
        # create the DASA params file (2nd parameter)
        #
        params_file_name = sys.argv[2]
        sys.stdout.write ("Creating DASA params file in [%s] ... " % params_file_name)
        write_cell_conf (cell_conf, params_file_name)
        sys.stdout.write ("done\n")

