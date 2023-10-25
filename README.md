
# iis3dwb-fifo2xyzt

This is a tool to convert [iis3dwb-247](https://github.com/metebalci/iis3dwb-247) output to xyzt values.

It reads `fifo.bin` and `fifo.bin.meta` created by `iisdwb-247` and creates `x.bin`, `y.bin`, `z.bin` containing xyz accelerometer values stored as a `double` in g units. Also, it creates a `t.bin` containing timestamp values stored as a `double` in seconds units. These files can easily be loaded to MATLAB for further analysis.
