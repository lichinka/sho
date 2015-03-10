import sys


def read_data (fileNames):
    """Reads the first three columns of each file and
    displays the average for each one.-"""

    files, line  = [], []
    totals       = []
    line_counter = 0
    
    #
    # Create a CSV reader for each file
    #
    for i in range (0, len (fileNames)):
        try:
            files.append (open (fileNames[i], 'r'))
        except:
            sys.stderr.write ("Error while opening file [")
            sys.stderr.write (fileNames[i] + "]\n")
            sys.exit (1)
    
    #
    # Calculate the average for the first column
    #
    while (line != None):
        subtotals = [0.0] * 10
        
        for i in range (0, len (files)):
            line = files[i].readline ( )
            
            if (not line):
                line = None
            else:
                line_counter += 1
                fields        = line.split ("\t")
                
                for j in range (0, len (fields)):
                    try:
                        subtotals[j] += float (fields[j])
                    except ValueError:
                        pass
            
        #
        # Add the subtotals calculated to the totals list
        #
        totals.append (list (subtotals))

    #
    # Calculate the final average
    #
    for row in totals:
        for i in range (0, len (row)):
            row[i] = row[i] / len(files)

    #
    # Display the values just calculated
    #
    for row in totals:
        for element in row:
            print "%5.2f\t" % element,
        print
                    
    
    
    
#
# This script's entry point.-
#
if __name__ == "__main__":
    #
    # Check the number of files received as command line arguments
    #
    if (len (sys.argv) > 1):
        #
        # Read data from all the files
        #
        read_data (sys.argv[1:])
            
        """
        if (len (sys.argcargv) < 5):
            printHelp (None)
            sys.exit (1)
        else:
            #
            # Read and process the input parameters
            #
            cellConfDict      = build_configuration_dict (sys.argv[1])
            cellConfDict_orig = dict (cellConfDict)
            neighbourMatrix   = build_adjacency_matrix (cellConfDict, sys.argv[2])
            startTemperature  = int (sys.argv[3])
            steps             = int (sys.argv[4])
        """
    else:
        sys.stderr.write("\nUsage: " + sys.argv[0] + " (files ...)\n")
        sys.stderr.write("\nCalculates the average (by column) of the files given.\n")
        
