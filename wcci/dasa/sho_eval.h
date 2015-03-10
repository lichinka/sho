#ifndef _SHO_OBJECTIVE_FUNCTION_
#define _SHO_OBJECTIVE_FUNCTION_

#ifdef NDEBUG
    #warning NDEBUG is defined! Assert will not work.
#endif
#define _SHO_EXTRA_PYTHON_PATH_     "sys.path.append ('../eval')"
#define _SHO_PYTHON_MODULE_         "evaluation"
#define _SHO_DL_PATH_LOSS_DIR_      "../../data/dl_path_loss"
#define _SHO_UL_PATH_LOSS_DIR_      "../../data/ul_path_loss"
#define _SHO_BEST_SERVER_PATH_LOSS_ "../../data/dl_path_loss/best_server.GRD"
#define _SHO_AREA_HEIGHT_           370
#define _SHO_AREA_WIDTH_            661

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"


//
// This global variable holds the full path to the
// cell configuration file. It is set from the command
// line argument, in 'main.c'
//
char cellConfFile [255];

//
// These two global variables hold references to the Python objects
// used during the objective function evaluation
//
static PyObject *pModule;
static PyObject *pEvaluation;


/**
 * Initializes the Python environment, including all data structures
 * needed for the objective function evaluation.-
 */
static void init_python_environment ( )
{
    int i;
    PyObject *pName, *pDict, *pClass, *pValue;
    PyObject *pEvaluationParams;

    // initialize the Python interpreter
    Py_Initialize ( );

    // update Python path, so that the interpreter will find our script
    PyRun_SimpleString ("import sys");
    PyRun_SimpleString (_SHO_EXTRA_PYTHON_PATH_);

    // load and import the Python module (file)
    pName = PyString_FromString (_SHO_PYTHON_MODULE_);
    pModule = PyImport_Import (pName);
    Py_DECREF (pName);
    assert (pModule != NULL);

    // borrowed reference to the module '__dict__'
    pDict = PyModule_GetDict (pModule);
    // borrowed reference to the 'Evaluation' class
    pClass = PyDict_GetItemString (pDict, "Evaluation");
    // create a tuple with the constructor parameters
    pEvaluationParams = PyTuple_New (6);
    assert (pEvaluationParams != NULL);

    // add the cell configuration file parameter
    pValue = PyString_FromString (cellConfFile);
    assert (pValue != NULL);
    PyTuple_SetItem (pEvaluationParams, 0, pValue);

    // add the DL path loss directory parameter
    pValue = PyString_FromString (_SHO_DL_PATH_LOSS_DIR_);
    assert (pValue != NULL);
    PyTuple_SetItem (pEvaluationParams, 1, pValue);

    // add the UL path loss directory parameter
    pValue = PyString_FromString (_SHO_UL_PATH_LOSS_DIR_);
    assert (pValue != NULL);
    PyTuple_SetItem (pEvaluationParams, 2, pValue);

    // add the best server DL path loss file parameter
    pValue = PyString_FromString (_SHO_BEST_SERVER_PATH_LOSS_);
    assert (pValue != NULL);
    PyTuple_SetItem (pEvaluationParams, 3, pValue);

    // add the area height parameter
    pValue = PyInt_FromLong (_SHO_AREA_HEIGHT_);
    assert (pValue != NULL);
    PyTuple_SetItem (pEvaluationParams, 4, pValue);

    // add the area width parameter
    pValue = PyInt_FromLong (_SHO_AREA_WIDTH_);
    assert (pValue != NULL);
    PyTuple_SetItem (pEvaluationParams, 5, pValue);

    // now that we have all constructor parameters,
    // create an instance of the class
    assert (PyCallable_Check (pClass));

    // create a new instance of the 'Evaluation' object
    pEvaluation = PyObject_CallObject (pClass, pEvaluationParams);
    Py_DECREF (pEvaluationParams);
    assert (pEvaluation != NULL);

    // call the 'init_gpu' method
    pValue = PyObject_CallMethod (pEvaluation, "init_gpu", NULL);
    if (pValue == NULL)
    {
        PyErr_Print ( );
        assert (pValue != NULL);
    }
    Py_DECREF (pValue);
    printf ("*** INFO: GPU initialized!\n");
}



/**
 * Calculates the objective function value, based the arrays received.-
 */
static TValue sho_evaluate (const TEVector cell_conf, 
                            const unsigned int cell_conf_len)
{
    int c_id, n_id;
    TValue ret_value = 0.0;
    PyObject *pParamList, *pValue, *pName;

    //
    // DEBUG: display the content of the current solution vector
    //
    //for (c_id = 0; c_id < cell_conf_len; c_id ++)
    //    printf ("%d\n", (int) cell_conf[c_id]*100);
    //

    // make sure the Python environment has been initialized
    if (!Py_IsInitialized ( ))
    {
        init_python_environment ( );
    }
    // pass the new settings to Python in a list
    pParamList = PyList_New (cell_conf_len);
    assert (pParamList != NULL);

    for (c_id = 0; c_id < cell_conf_len; c_id ++)
    {
        pValue = PyFloat_FromDouble ((double) cell_conf[c_id]);
        assert (pValue != NULL);
        PyList_SetItem (pParamList, c_id, pValue);
    }
    // name of the method to calculate the objective function on GPU
    pName = PyString_FromString ("calculate_on_gpu");
    assert (pName != NULL);
    // calculate the objective function on the GPU
    pValue = PyObject_CallMethodObjArgs (pEvaluation, pName, pParamList, NULL);
    Py_DECREF (pName);
    Py_DECREF (pParamList);
    if (pValue == NULL)
    {
        PyErr_Print ( );
        assert (pValue != NULL);
    }
    else
    {
        ret_value = (TValue) PyInt_AsLong (pValue);
        Py_DECREF (pValue);
    }
    return ret_value;
}


#endif
