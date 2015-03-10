Objective function evaluation using PyOpenCL, as defined by radio engineers @ TS.-


### Objective function evaluation
- finds SHO areas in DL and UL, by analyzing Montecarlo snapshots with path-loss matrices;
    - input: 
        - DL path-loss matrices for each of the cells in the optimized area,
        - DL best server path-loss matrix,
        - UL path-loss matrices for each of the cells in the optimized area,
        - Montecarlo snapshot of UE, biased on population density.

    - calc:
        - for each pixel, where UE is, we calculate:
            - if pixel is in SHO_UL:
                - with ASC
                    Tx UE - path_loss(pixel) >= Cell sensitivity at Node B (-115dBm, based on Finland reference and HSUPA)

                - without ASC
                    Tx UE - path_loss(pixel) - cable_loss(cell) >= Cell sensitivity at Node B (-115dBm, based on Finland reference and HSUPA)

            - if pixel is in SHO_DL:
                -115dBm (based on Holma) >= Tx_antenna(cell) - path_loss(pixel) >= Best_server_RSCP(pixel) - 4 dB (based on current network parameters of M@ and service provided, i.e. EUL)

        - for each SHO area:
            - calculate
                (SHO_DL intersec. SHO_UL) / SHO_DL

              with this, we measure how well the two areas fit (i.e. how well SHO_UL covers SHO_DL).

    - output:
        - score, based on the (un)alignment of SHO areas in DL and UL.

