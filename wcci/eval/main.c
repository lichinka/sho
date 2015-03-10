//
// Calling the objective function evaluation,
// implemented in Python, from C code
// 
#include <Python.h>



/**
 * Entry point
 */
int main (int argc, char *argv[])
{
    int i;
    PyObject *pName, *pModule, *pDict, *pClass, *pValue;
    PyObject *pEvaluation, *pEvaluationParams;
    PyObject *pCellList, *pCell;

    if (argc < 2)
    {
        fprintf (stderr, "Usage: %s pythonfile [args]\n", argv[0]);
        return 1;
    }

    // initialize the Python interpreter
    Py_Initialize ( );

    // add the current directory to the Python path
    PyRun_SimpleString ("import sys");
    PyRun_SimpleString ("sys.path.append ('.')");

    // load and import the Python module (file)
    pName = PyString_FromString (argv[1]);
    pModule = PyImport_Import (pName);
    Py_DECREF (pName);

    if (pModule != NULL)
    {
        // borrowed reference to the module '__dict__'
        pDict = PyModule_GetDict (pModule);
        // borrowed reference to the 'Evaluation' class
        pClass = PyDict_GetItemString (pDict, "Evaluation");
        // create a tuple with the constructor parameters
        pEvaluationParams = PyTuple_New (6);
        
        if (pEvaluationParams != NULL) 
        {
            // add the cell configuration file parameter
            pValue = PyString_FromString ("./cell_powers.txt");

            if (pValue != NULL)
                PyTuple_SetItem (pEvaluationParams, 0, pValue);
            else
            {
                PyErr_Print ( );
                return EXIT_FAILURE;
            }
            // add the DL path loss directory parameter
            pValue = PyString_FromString ("../../data/dl_path_loss");

            if (pValue != NULL)
                PyTuple_SetItem (pEvaluationParams, 1, pValue);
            else
            {
                PyErr_Print ( );
                return EXIT_FAILURE;
            }
            // add the UL path loss directory parameter
            pValue = PyString_FromString ("../../data/dl_path_loss");

            if (pValue != NULL)
                PyTuple_SetItem (pEvaluationParams, 2, pValue);
            else
            {
                PyErr_Print ( );
                return EXIT_FAILURE;
            }
            // add the best server DL path loss file parameter
            pValue = PyString_FromString ("../../data/dl_path_loss/best_server.GRD");

            if (pValue != NULL)
                PyTuple_SetItem (pEvaluationParams, 3, pValue);
            else
            {
                PyErr_Print ( );
                return EXIT_FAILURE;
            }
            // add the area height parameter
            pValue = PyInt_FromLong (370);

            if (pValue != NULL)
                PyTuple_SetItem (pEvaluationParams, 4, pValue);
            else
            {
                PyErr_Print ( );
                return EXIT_FAILURE;
            }
            // add the area width parameter
            pValue = PyInt_FromLong (661);

            if (pValue != NULL)
                PyTuple_SetItem (pEvaluationParams, 5, pValue);
            else
            {
                PyErr_Print ( );
                return EXIT_FAILURE;
            }

            // now that we have all constructor parameters,
            // create an instance of the class
            if (PyCallable_Check (pClass))
            {
                // create a new instance of the 'Evaluation' object
                pEvaluation = PyObject_CallObject (pClass, pEvaluationParams);
             
                // free memory taken by the parameters
                // of the 'Evaluation' constructor
                Py_DECREF (pEvaluationParams);

                if (pEvaluation != NULL)
                { 
                    // a new reference to the cell_list contained in Evaluation
                    pCellList = PyObject_GetAttrString (pEvaluation, "cell_list");
                    
                    if (pCellList == NULL)
                    { 
                        PyErr_Print ( );
                        return EXIT_FAILURE;
                    }

                    // call the 'init_gpu' method
                    pValue = PyObject_CallMethod (pEvaluation, "init_gpu", NULL);

                    if (pValue != NULL)
                    {
                        // free memory taken by the result
                        // of the 'init_gpu' method call
                        Py_DECREF (pValue);
                        printf ("*** INFO: GPU initialized!\n");
                        
                        // simulate some evaluations on GPU
                        for (i = 0; i < 1000; i ++)
                        {
                            // calculate the objective function on the GPU
                            pValue = PyObject_CallMethod (pEvaluation, "calculate_objective_on_gpu", NULL);
                            if (pValue != NULL)
                            {
                                printf ("%ld\n", PyInt_AsLong (pValue));
                                // free memory taken by the result of the
                                // 'calculate_objective_on_gpu' method call
                                Py_DECREF (pValue);

                                // get a reference to a Cell object
                                pCell = PyList_GetItem (pCellList, i % 20);
                                pValue = PyObject_GetAttrString (pCell, "pilot_pwr");
                                printf ("Cell %d has pilot power %d\n", i%20, *pValue);
                                // free memory taken by the result of the
                                // 'pilot_pwr' attribute
                                Py_DECREF (pValue);
                            }
                            else
                            { 
                                PyErr_Print ( );
                                return EXIT_FAILURE;
                            }
                        }
                    }
                    else
                    {
                        PyErr_Print ( );
                        return EXIT_FAILURE;
                    }
                    // free memory taken by the 'cell_list' of the 'Evaluation' object
                    Py_DECREF (pCellList);
                    // free memory taken by the 'Evaluation' object
                    Py_DECREF (pEvaluation);
                }
            }
        }
        // free memory taken by the loaded module
        Py_DECREF (pModule);
    }
    else
    {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
        return 1;
    }
    Py_Finalize ( );
    return EXIT_SUCCESS;
}

