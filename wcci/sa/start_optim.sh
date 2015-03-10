#!/bin/bash

if [ $# -gt 7 ]; then
    MOCRETA="$1"
    #
    # Run optimization with SA
    #
    /usr/bin/env python2.7 optimizer.py ${MOCRETA} $2 $3 $4 $5 $6 $7 $8
else
    /usr/bin/env python2.7 optimizer.py
fi
