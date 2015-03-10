#define _COUNTER_ELEMENTS_  3

/**
 * Calculates the objective function value, based on the number
 * of cells covering a pixel, and how many of them are in SHO DL
 * and SHO UL in each pixel of the area:
 * - first byte indicates the number of cells covering the pixel;
 * - second byte indicates the number of cells in SHO DL;
 * - third byte indicates the number of cells in SHO UL.-
 */
__kernel void calc_obj (__global const char *in_sho,
                        __global float *ret_value)
{
    unsigned int idx = get_global_id (0);
    unsigned int in_sho_idx = _COUNTER_ELEMENTS_ * idx;

    //
    // impose penalty scores, based on uncovered pixels
    //
    if (in_sho[in_sho_idx] < 1)
    {
        ret_value[idx] = 15;
    }
    else
    {
        //
        // impose penalty scores, based on how SHO areas overlap:
        //
        // [in_sho_idx + 1] represents SHO DL
        // [in_sho_idx + 2] represents SHO UL
        //
        if ((in_sho[in_sho_idx + 1] > 1) && (in_sho[in_sho_idx + 2] < 2))
        {
            ret_value[idx] = 2;
        }
        else if ((in_sho[in_sho_idx + 1] < 2) && (in_sho[in_sho_idx + 2] > 1))
        {
            ret_value[idx] = 13;
        }
        else
        {
            ret_value[idx] = 0;
        }
    }
}



/**
 * Marks whether the received cell covers a pixel, and whether it is
 * in SHO DL and/or SHO UL area:
 * - first byte indicates the number of cells covering the pixel;
 * - second byte indicates the number of cells in SHO DL;
 * - third byte indicates the number of cells in SHO UL.-
 */
__kernel void in_sho (const unsigned int cell_id,
                      const float pilot_pwr,
                      const float cable_loss,
                      __global const float *cell_dl_pl,
                      __global const float *cell_ul_pl,
                      __global const float *bs_dl_pl,
                      __global char *ret_value)
{
    unsigned int idx = get_global_id (0);
    unsigned int in_sho_idx = _COUNTER_ELEMENTS_ * idx;

    //
    // Coverage
    //
    float coverage = pilot_pwr - cell_dl_pl[idx];
    //
    // SHO DL
    //
    float sho_dl = pilot_pwr - cell_dl_pl[idx];
    // 27.512 dBm = avg Tx power for best server
    // 4 dB = SHO window
    float bs_rscp = (27.512f - bs_dl_pl[idx]) - 4.0f;
    //
    // SHO UL
    //
    float sho_ul = 21.0f - cell_ul_pl[idx];     // 21 = UE Tx power in dBm
    sho_ul -= cable_loss;
    //
    // whether this pixel is covered
    //
    if (coverage >= -115.0f)
    {
        ret_value[in_sho_idx] += 1;
    }
    //
    // whether we are within SHO DL area
    //
    if ((sho_dl >= -115.0f) && (sho_dl >= bs_rscp))
    {
        ret_value[in_sho_idx + 1] += 1;
    }
    //
    // whether we are within SHO UL area
    //
    if (sho_ul >= -115.0f)
    {
        ret_value[in_sho_idx + 2] += 1;
    }
}


