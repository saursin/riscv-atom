# gowin-ddr3 (vendored)

Unmodified copy of `ddr3/` files from [e-karge/gowin-ddr3](https://github.com/e-karge/gowin-ddr3)
at commit `84a1a11681082fa599c61d3445ee173bc60ae659`, Apache-2.0 (see `LICENSE`). Fixes go upstream;
to update, fetch the same files at a new commit.

Only the slow PHY is vendored (`ddr3_ctrl`, `ddr3_cmd`, `ddr3_init`, `ddr3_calib`, `ddr3_phy_slow`).
`ddr3_ctrl.sv` includes the other controller files as `ddr3/...`: put this directory on the include
path. `../DDR3_wb.v` includes `ddr3_ctrl.sv`; the board shell lists `ddr3_phy_slow.sv`.
