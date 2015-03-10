#include <math.h>
#include <stdio.h>
#include <stdlib.h>



/**
 * Makes a random move in the search space, by changing the CPICH
 * of a randomly selected cell in 'cell_conf'. 
 * Only changes in the range of [-2 dB, +2 dB] of the original
 * CPICH setting are allowed.-
 * 
 */
void move (float *cell_conf,
           const float *cell_conf_orig,
           const unsigned int cell_conf_len)
{
    //
    // randomly select a cell
    //
    int rand_cell_id = rand ( ) % cell_conf_len;
    //
    // make sure the change is allowed before applying it
    //
    float new_pwr, is_allowed = 10.0f;
    while (is_allowed > 2.0f)
    {
        //
        // a random change to the CPICH of the selected cell,
        // one of [-0.01, 0, 0.01]
        //
        float rand_change = (rand ( ) % 3) - 1;
        rand_change /= 100.0f;
        //
        // check that the change is allowed
        //
        new_pwr = cell_conf[rand_cell_id] + rand_change;
        is_allowed = fabsf (cell_conf_orig[rand_cell_id] - new_pwr);
    }
    //
    // apply the change in CPICH
    //
    cell_conf[rand_cell_id] = new_pwr;
}



/**
 * Displays the cell configuration array
 */
void display_cell_conf (const float *cell_conf,
                        const unsigned int cell_conf_len)
{
    int i;
    for (i = 0; i < cell_conf_len; i ++)
    {
        printf ("%d\t%6.2f\n", i, cell_conf[i]);
    }
}


