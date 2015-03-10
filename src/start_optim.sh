#!/bin/bash

if [ $# -gt 1 ]; then
    MOCRETA="$1"
    SHO="$2"
    
	#
	# Remove the commas (,) from the power file header and convert everything to lower case
	#
	head -n1 ${MOCRETA} | tr -d ',' | tr "[:upper:]" "[:lower:]"  > ${MOCRETA}.tmp
	
	#
	# Output the rest of the power file (i.e. without header) sorted by cell name
	#
	tail --lines=+2 ${MOCRETA} | sort >> ${MOCRETA}.tmp
    
	#
	# Output the content of the SHO file sorted by cell and number of HOs
	#
	echo "cell neigh started success fail" > ${SHO}.tmp
	cat ${SHO} | tr '-' ' ' | sort -t' ' -k1,1 -k3,3rn >> ${SHO}.tmp
	
    #
    # Calculate the power differences of this file
    #
    /usr/bin/env python2 optimizer.py ${MOCRETA}.tmp ${SHO}.tmp $3 $4

	#
	# Remove the temporary files
	#
	#rm -f ${MOCRETA}.tmp ${SHO}.tmp
else
	/usr/bin/env python2 optimizer.py
fi
