#!/usr/bin/env python2

import sys
import pyopencl as cl
from csv import DictReader
from time import time
from random import randint
from objects import *

#
# Number of counters within each pixel of the 'in_sho' array:
#   - first element shows the number of cells covering the pixel;
#   - second element shows the number of cell in SHO DL over the pixel;
#   - third element shows the number of cells in SHO UL over the pixel.
#
COUNTER_ELEMENTS = 3



class Evaluation ( ):
    """
    Contains all the elements to calculate the objective function value.-
    """
    #
    # the OpenCL context and its queue, used for calculating the
    # objective function value; and one program object
    #
    ctx = None
    queue = None
    program = None
    #
    # all the cells within the grid area
    #
    cell_list = None
    #
    # the best server, where the path-loss matrix for best servers in DL is kept
    #
    best_server = None
    #
    # the width and height of the grid area under optimization
    #
    height = None
    width = None


    def __init__ (self, cell_conf_file, dl_path_loss_dir,
                        ul_path_loss_dir, bs_path_loss_file,
                        height, width):
        """
        Initializes the objective function evaluation object, including
        the OpenCL execution context:
            - cell_conf_file: file containing the configuration for all
                              cells within the area under optimization;
            - dl_path_loss_dir: directory containing all DL path loss matrices;
            - ul_path_loss_dir: directory containing all UL path loss matrices;
            - bs_path_loss_file: file name of the DL path loss matrix, containing
                                 best server path loss values at every pixel;
            - height: height of the area under optimization (in number of pixels);
            - width: width of the area under optimization (in number of pixels).
        """
        self.ctx = cl.create_some_context ( )
        self.queue = cl.CommandQueue (self.ctx)
        self.read_cell_configuration (cell_conf_file)
        self.read_cell_path_loss_matrices (dl_path_loss_dir,
                                           ul_path_loss_dir)
        self.read_best_server_path_loss (bs_path_loss_file)
        self.set_grid_size (height, width)

    def dump_2D_map (self, power_list, out_file_name):
        """
        Creates a 2D map in 'out_file_name' of coverage and 
        the SHO DL-SHO UL overlap, taking the received 'power_list' 
        into account.-
        """
        #
        # allocate the SHO counter buffer:
        # byte at even position indicates the number of cells in SHO DL,
        # byte at odd position indicates the number of cells in SHO UL
        #
        mf = cl.mem_flags
        self.in_sho = np.zeros ((self.height * self.width * COUNTER_ELEMENTS,),
                                dtype=np.byte)
        self.in_sho_gpu = cl.Buffer (self.ctx, 
                                     mf.READ_WRITE | mf.COPY_HOST_PTR, 
                                     hostbuf=self.in_sho)
        #
        # take the power list into account
        #
        for c_id in range (0, len (power_list)):
            power = np.single (power_list[c_id])
            cell = self.cell_list[c_id]
            if cell.has_asc:
                cable_loss = np.single (0.0)
            else:
                cable_loss = np.single (cell.cable_loss)
            self.program.in_sho (self.queue, 
                                 (self.height * self.width,),
                                 None,
                                 np.uintc (c_id),
                                 power,
                                 cable_loss,
                                 cell.DL_path_loss_matrix_gpu,
                                 cell.UL_path_loss_matrix_gpu,
                                 self.best_server.DL_path_loss_matrix_gpu,
                                 self.in_sho_gpu)
            cl.enqueue_read_buffer (self.queue, 
                                    self.in_sho_gpu, 
                                    self.in_sho).wait ( )
        with open (out_file_name, 'w') as f:
            for h in range (0, self.height):
                for w in range (0, self.width):
                    in_sho_idx = (h * self.width + w) * COUNTER_ELEMENTS
                    is_covered = self.in_sho[in_sho_idx] > 0
                    in_sho_dl  = self.in_sho[in_sho_idx + 1] > 1
                    in_sho_ul  = self.in_sho[in_sho_idx + 2] > 1
                    if not is_covered:
                        flag = 0
                    elif in_sho_dl and in_sho_ul:
                        flag = 1
                    elif not in_sho_dl and in_sho_ul:
                        flag = 2
                    elif in_sho_dl and not in_sho_ul:
                        flag = 3
                    else:
                        flag = 5
                    f.write ('%i %i %i\n' % (w, h, flag))
        f.close ( )

            
    def load_ocl_program (self, file_name):
        """
        Loads and compiles an OpenCL program from 'file_name'.-
        """
        #
        # read in the OpenCL source file as a string
        #
        f = open (file_name, 'r')
        fstr = ''.join (f.readlines ( ))
        print "Loading kernel functions from [%s] ..." % file_name,
        #
        # compile the program
        #
        self.program = cl.Program (self.ctx, fstr).build ( )
        print "ok"

    def read_cell_configuration (self, file_name):
        """
        Reads cell configuration from 'file_name', saving all
        the cells in the 'cell_list' member.-
        """
        try:
            pwrRdr = DictReader (open (file_name, 'r'), delimiter=' ')
        except IOError:
            sys.stderr.write ("Error while opening file %s!\n" % file_name)
            sys.exit (1)
        #
        # cell configuration is a list of Cell objects
        #
        #   cell_list[cell_id]: Cell
        #
        self.cell_list = []
        for line in pwrRdr:
            c = Cell (self.ctx)
            c.name = line['cell']
            c.pilot_pwr = int (line['pilot']) / 10.0
            c.pilot_pwr = np.single (c.pilot_pwr)
            c.cable_loss = int (line['kabel']) / 10.0
            c.has_asc = True if line['asc'] == '1' else False
            self.cell_list.append (c)

    def read_cell_path_loss_matrices (self, dl_dir_name, ul_dir_name):
        """
        Reads the DL and UL path loss matrices for all cells that
        have a valid configuration. The path-loss matrices are
        read from '*dir_name', appending the cell name as file.-
        """
        for cell in self.cell_list:
            pl_file_name = '%s/%s.GRD' % (dl_dir_name, cell.name)
            print "Reading DL path loss data from [%s] ..." % pl_file_name
            cell.read_DL_path_loss_data (pl_file_name)
            pl_file_name = '%s/%s.GRD' % (ul_dir_name, cell.name)
            print "Reading UL path loss data from [%s] ..." % pl_file_name
            cell.read_UL_path_loss_data (pl_file_name)

    def read_best_server_path_loss (self, file_name):
        """
        Reads the path-loss matrix containing the best server
        path losses.-
        """
        self.best_server = Cell (self.ctx)
        self.best_server.name = "BEST SERVER"
        self.best_server.read_DL_path_loss_data (file_name)

    def set_grid_size (self, height, width):
        """
        Sets the size of the area under optimization
        These numbers are given in pixels.-
        """
        self.height = int (height)
        self.width = int (width)
        #
        # make sure all path loss matrices are the same size
        #
        print "Checking that all matrices are the same size ...",
        for cell in self.cell_list:
            assert (cell.DL_path_loss_matrix.shape == (self.height * self.width,))
            assert (cell.UL_path_loss_matrix.shape == (self.height * self.width,))
        assert (self.best_server.DL_path_loss_matrix.shape == (self.height * self.width,))
        print "ok"

    def calculate_objective_on_cpu (self):
        """
        Calculates the objective function value for the current 
        cell configuration state on the CPU.-
        """
        ret_value = 0
        for c_id in range (0, len (self.cell_list)):
            cell = self.cell_list[c_id]
            for idx in range (0, self.height * self.width):
                in_sho_idx = idx * COUNTER_ELEMENTS
                #
                # Coverage
                #
                coverage = cell.pilot_pwr - cell.DL_path_loss_matrix[idx]
                self.in_sho[in_sho_idx] += 1 if (coverage >= -115.0) else 0
                #
                # SHO DL
                #
                sho_dl = cell.pilot_pwr - cell.DL_path_loss_matrix[idx]
                # 27.512 dBm is an average Tx power for the best server, 
                # used to speed calculation up
                bs_rscp = 27.512 - self.best_server.DL_path_loss_matrix[idx]
                # 4 dB window for SHO DL
                if sho_dl >= -115.0:
                    self.in_sho[in_sho_idx + 1] += 1 if (sho_dl >= bs_rscp - 4) else 0
                #
                # SHO UL 
                # (21.0 = UE Tx power in dBm)
                #
                sho_ul = 21.0 - cell.UL_path_loss_matrix[idx]
                if not cell.has_asc:
                    sho_ul -= cell.cable_loss
                # -115.0 dBm RSCP
                self.in_sho[in_sho_idx + 2] += 1 if (sho_ul >= -115.0) else 0
        #
        # apply penalty scores, based on coverage
        # and the overlap of SHO DL and SHO UL areas
        #
        for idx in range (0, self.height * self.width):
            is_covered = self.in_sho[COUNTER_ELEMENTS * idx] > 0
            in_sho_dl  = self.in_sho[COUNTER_ELEMENTS * idx + 1] > 1
            in_sho_ul  = self.in_sho[COUNTER_ELEMENTS * idx + 2] > 1
            if not is_covered:
                ret_value += 15.0
            elif in_sho_dl and (not in_sho_ul):
                ret_value += 3.0
            elif (not in_sho_dl) and in_sho_ul:
                ret_value += 13.0
        return ret_value


    def calculate_objective_on_gpu (self):
        """
        Calculates the objective function value for the current state on the GPU.-
        """
        ret_value = 0
        #
        # allocate the SHO counter buffer:
        # byte at even position indicates the number of cells in SHO DL,
        # byte at odd position indicates the number of cells in SHO UL
        #
        mf = cl.mem_flags
        self.in_sho = np.zeros ((self.height * self.width * COUNTER_ELEMENTS,),
                                dtype=np.byte)
        self.in_sho_gpu = cl.Buffer (self.ctx, 
                                     mf.READ_WRITE | mf.COPY_HOST_PTR, 
                                     hostbuf=self.in_sho)
        #
        # run the kernel for each cell in the service area
        #
        for c_id in range (0, len (self.cell_list)):
            cell = self.cell_list[c_id]
            if cell.has_asc:
                cable_loss = np.single (0.0)
            else:
                cable_loss = np.single (cell.cable_loss)
            self.program.in_sho (self.queue, 
                                 (self.height * self.width,),
                                 None,
                                 np.uintc (c_id),
                                 cell.pilot_pwr,
                                 cable_loss,
                                 cell.DL_path_loss_matrix_gpu,
                                 cell.UL_path_loss_matrix_gpu,
                                 self.best_server.DL_path_loss_matrix_gpu,
                                 self.in_sho_gpu)
            self.program.calc_obj (self.queue, 
                                   (self.height * self.width,),
                                   None,
                                   self.in_sho_gpu,
                                   self.dest_gpu)
            cl.enqueue_read_buffer (self.queue, 
                                    self.dest_gpu, 
                                    self.dest).wait ( )
        ret_value = np.sum (self.dest)
        return ret_value


    def calculate_on_gpu (self, power_list):
        """
        Calculates the objective function value for the current state
        on the GPU, taking the new powers received into account.-
        """
        ret_value = 0
        #
        # allocate the SHO counter buffer:
        # byte at even position indicates the number of cells in SHO DL,
        # byte at odd position indicates the number of cells in SHO UL
        #
        mf = cl.mem_flags
        self.in_sho = np.zeros ((self.height * self.width * COUNTER_ELEMENTS,),
                                dtype=np.byte)
        self.in_sho_gpu = cl.Buffer (self.ctx, 
                                     mf.READ_WRITE | mf.COPY_HOST_PTR, 
                                     hostbuf=self.in_sho)
        #
        # run the kernel for each cell in the service area
        #
        for i in range (0, len (power_list)):
            power = np.single (power_list[i])
            cell = self.cell_list[i]
            if cell.has_asc:
                cable_loss = np.single (0.0)
            else:
                cable_loss = np.single (cell.cable_loss)
            self.program.in_sho (self.queue, 
                                 (self.height * self.width,),
                                 None,
                                 np.uintc (i),
                                 power,
                                 cable_loss,
                                 cell.DL_path_loss_matrix_gpu,
                                 cell.UL_path_loss_matrix_gpu,
                                 self.best_server.DL_path_loss_matrix_gpu,
                                 self.in_sho_gpu)
            self.program.calc_obj (self.queue, 
                                   (self.height * self.width,),
                                   None,
                                   self.in_sho_gpu,
                                   self.dest_gpu)
            cl.enqueue_read_buffer (self.queue, 
                                    self.dest_gpu, 
                                    self.dest).wait ( )
        ret_value = np.sum (self.dest)
        return ret_value


    def init_gpu (self):
        """
        This function should be called BEFORE executing calculation on GPU.-
        """
        #
        # load and build the OpenCL program
        #
        self.load_ocl_program ('objective.cl')
        mf = cl.mem_flags
        #
        # allocate the resulting buffer
        #
        self.dest = np.array (range (self.height * self.width),
                              dtype=np.single)
        self.dest_gpu = cl.Buffer (self.ctx, 
                                   mf.WRITE_ONLY | mf.COPY_HOST_PTR, 
                                   hostbuf=self.dest)
        for cell in self.cell_list:
            print cell.name, cell.pilot_pwr


    def read_powers (self, in_file_name):
        ret_value = []
        with open (in_file_name, 'r') as f:
            for line in f:
                if '[Solution]' in line:
                    break
            for line in f:
                if '=' in line:
                    param, val = line.split ('=')
                    ret_value.append (float (val))
                else:
                    break
        f.close ( )
        return ret_value




if __name__ == "__main__":
    #
    # Check the number of command line arguments
    #
    if (len (sys.argv) < 7):
        sys.stderr.write ("\nUsage: %s [pwr conf file] [dl pl dir] [ul pl dir] [bs pl file] [height] [width]\n" % sys.argv[0])
        sys.stderr.write ("\nEvaluates the (un)alignment of DL and UL SHO areas.\n\n")
        sys.exit (1)
    else:
        #
        # initialize the evaluation object
        #
        obj_eval = Evaluation (*sys.argv[1:])
        obj_eval.init_gpu ( )
   
        """
        cell_conf = obj_eval.read_powers ('/home/luka/etc/dr/wcci/src/dasa/stats/DASA-f101-25-21.stats')
        obj_eval.dump_2D_map (cell_conf, '/tmp/final.dat')
        """

        #
        # simulate some evaluations on CPU
        #
        t0 = time ( )
        for i in range (0, 5):
            #
            # randomize cell powers
            #
            for c in obj_eval.cell_list:
                c.pilot_pwr += np.single (randint (-1, 1) / 10.0)
            print '%10.2f' % obj_eval.calculate_objective_on_gpu ( )
            print '\t%10.2f' % obj_eval.calculate_objective_on_cpu ( )
        t1 = time ( ) - t0
        print "5 evaluations in %s seconds" % (t1)

        """
        #
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
