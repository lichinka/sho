	D I F F E R E N T I A L    E V O L U T I O N

C-code in the file de.c implements Differential Evolution (DE) algorithm,
more precisely DE/rand/1/bin [1,2,3] version of the method. This DE
variation is probably the most used DE variation and perform well with
arbitrary problems.

Whole algorithm is in the file de.c and you may implement your
optimization problem to a separate file. You do not have to make any
changes to the de.c file, all the problem specific definitions are to be
made only the file where objective function is in function func. See for
example file rastrigin_2d.c for problem description.

You can compile the program with command:

gcc -Wall -pedantic -ansi -O -o de de.c problem.c -lm

, where you replace problem.c file with the file name containing your 
optimization problem.

All the parameters for the program are given on command line and you can
run ./de -h to see usage. You may run this program also without any
parameters using default values.

Basic control parameters are:

-N NP (20*D)	defines size of the population NP (20*dimension of the 
		problem is default)

-G Gmax (1000)	defines number of generations

-C CR (0.9)	defines crossover constant

-F F (0.9)	defines mutation scaling factor

Other parameter for the program are:

-u
-h		usage of the problem

-o <fname>	output file name for the final population

-s		uses same random number generator seed

Format of output file is such that every row contains one member of the
population, first are decision variables and final value is the objective
function value, i.e.,

	x1_1 x1_2 x1_3	...	x1_D f1
	x2_1 x2_2 x2_3	...	x2_D f2
	x3_1 x3_2 x3_3	...	x3_D f3
	 :    :    :		 :
	xN_1 xN_2 xN_3	...	xN_D fN	


Parameter CR controls the crossover operation and its value is [0.0, 1.0].
Parameter F is a scaling factor for the mutation and its value is
typically (0,1+]. In practice, CR controls the rotational invariance of
the search, and its small value (e.g. 0.1) is practicable with separable
problems and larger values (e.g 0.9) are practicable for non-separable
problems. Control parameter F controls the speed and robustness of the
search, i.e., lower value for F increases the convergence rate but also
the risk of stacking into a local optimum. If nothing is known about
problem characteristics then it might be good to use the control parameter
values CR = F = 0.9 which give slow convergence but provide the global
optimum reliable.

You may try out following commands:

gcc -Wall -pedantic -ansi -O -o de de.c rastrigin_20d.c -lm
./de -N 20 -G 500 -C 0.0 -F 0.5

gcc -Wall -pedantic -ansi -O -o de de.c rastrigin_20d_skewed.c -lm
./de -N 100 -G 10000 -C 0.2 -F 0.5

gcc -Wall -pedantic -ansi -O -o de de.c schwefel_20d.c -lm
./de -N 50 -G 400 -C 0.2 -F 0.5


Author and copyright:

Saku Kukkonen
Lappeenranta University of Technology
Department of Information Technology
P.O.Box 20, FIN-53851 LAPPEENRANTA, Finland
E-mail: saku.kukkonen@lut.fi

Code is free for scientific and academic use. Use for other purpose is not
allowed without a permission of the author. There is no warranty of any
kind about correctness of the code and if you find a bug, please, inform
the author.

Please, acknowledge and inform the author if you use this code.


References:

[1] Rainer Storn and Kenneth V. Price, Differential Evolution - A simple 
and efficient adaptive scheme for global optimization over continuous 
spaces, Technical Report, TR-95-012, ICSI, March, 1995, [Online] 
Available: 
www.icsi.berkeley.edu/ftp/global/pub/techreports/1995/tr-95-012.pdf, 
14.6.2005.

[2] Rainer Storn and Kenneth V. Price, Differential evolution - a Simple
and Efficient Adaptive Scheme for Global Optimization Over paces, Journal
of Global Optimization, 11 (4), pp. 341-359, Dec, 1997, Kluwer Academic
Publisher.

[3] Kenneth V. Price, Rainer Storn, and Jouni Lampinen, Differential
Evolution: A Practical Approach to Global Optimization, Springer-Verlag,
Berlin, ISBN: 3-540-20950-6, 2005.
