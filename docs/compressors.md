Sorbet internally uses a lizard(LZ5) compressor to compress internal datastructures, with compression level 10.
Decision to use it was made via the following benchmark(run with https://github.com/inikep/lzbench)
Note that we care a lot about decompression speed, followed by size. We care a bit about compression speed, but not much.
The numbers below are measured on the GlobalState of Sorbet for the stdlib.
lz5 provides great decompression(~5 GB/s) while compressing to around 1/3rd of the size and with ~100M/s compression speed.

| Compressor name         | Compression| Decompress.| Compr. size | Ratio | Filename |
| ---------------         | -----------| -----------| ----------- | ----- | -------- |
| lzsse8fast 2019-04-18   |   16223 us |     712 us |     1307792 | 35.92 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lz4fast 1.9.2 -17       |    3524 us |     787 us |     2033328 | 55.85 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzsse4fast 2019-04-18   |   15781 us |     793 us |     1316506 | 36.16 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lz4fast 1.9.2 -3        |    5414 us |     829 us |     1500239 | 41.21 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lz4 1.9.2               |    6161 us |     866 us |     1396124 | 38.35 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -15          |   40246 us |     899 us |     1161560 | 31.90 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -14          |   39478 us |     903 us |     1171562 | 32.18 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -13          |   36268 us |     907 us |     1188123 | 32.63 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -12          |   26601 us |     917 us |     1218547 | 33.47 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -10          |    6746 us |     935 us |     1398895 | 38.42 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -19          |  648527 us |     952 us |     1120946 | 30.79 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -5           |   79818 us |     954 us |     1126708 | 30.95 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -2           |   79787 us |     954 us |     1126708 | 30.95 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -1           |   79942 us |     956 us |     1126708 | 30.95 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lizard 1.0 -11          |    9830 us |     964 us |     1333983 | 36.64 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| pithy 2011-12-24 -9     |    6343 us |    1506 us |     1255721 | 34.49 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| pithy 2011-12-24 -6     |    5916 us |    1541 us |     1276049 | 35.05 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| pithy 2011-12-24 -3     |    5624 us |    1625 us |     1332676 | 36.60 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| pithy 2011-12-24 -0     |    5243 us |    1656 us |     1409221 | 38.71 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| snappy 2019-09-30       |    6284 us |    1750 us |     1452984 | 39.91 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| shrinker 0.1            |    8846 us |    2048 us |     1305439 | 35.86 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| density 0.14.2 -1       |    3422 us |    2119 us |     2367790 | 65.04 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzvn 2017-03-08         |   48864 us |    2928 us |     1147413 | 31.52 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzfse 2017-03-08        |   46924 us |    3137 us |      976604 | 26.82 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -12          |  109128 us |    3726 us |      873937 | 24.00 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| density 0.14.2 -2       |    5663 us |    3783 us |     1791894 | 49.22 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -10          |   66981 us |    3785 us |      880465 | 24.18 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -1           |    9179 us |    3893 us |     1089702 | 29.93 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzf 3.6 -1              |   10730 us |    4063 us |     1441358 | 39.59 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -2           |   10461 us |    4192 us |     1041280 | 28.60 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzf 3.6 -0              |   10049 us |    4192 us |     1488389 | 40.88 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -3           |   13529 us |    4226 us |      986910 | 27.11 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -4           |   15534 us |    4275 us |      980295 | 26.93 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzo1c 2.10 -1           |   11834 us |    4284 us |     1424465 | 39.13 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzo1x 2.10 -1           |    5679 us |    4303 us |     1451507 | 39.87 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -20          |  795030 us |    4315 us |      801329 | 22.01 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzo1y 2.10 -1           |    5719 us |    4346 us |     1448549 | 39.79 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzo1b 2.10 -1           |   12882 us |    4353 us |     1371249 | 37.66 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| zstd 1.4.4 -5           |   29924 us |    4373 us |      944741 | 25.95 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzo1f 2.10 -1           |   13097 us |    4660 us |     1430914 | 39.30 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| fastlz 0.1 -2           |   10776 us |    5020 us |     1389951 | 38.18 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| fastlz 0.1 -1           |   11455 us |    5118 us |     1462824 | 40.18 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzrw 15-Jul-1991 -1     |   11591 us |    5301 us |     1702140 | 46.75 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzrw 15-Jul-1991 -3     |   10862 us |    5307 us |     1535721 | 42.18 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzjb 2010               |   11024 us |    5538 us |     1852615 | 50.89 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzrw 15-Jul-1991 -5     |   19477 us |    6024 us |     1346634 | 36.99 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| quicklz 1.5.0 -1        |    6666 us |    6081 us |     1310173 | 35.99 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| tornado 0.6a -1         |   10191 us |    6431 us |     1463980 | 40.21 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| lzrw 15-Jul-1991 -4     |    9606 us |    6748 us |     1470050 | 40.38 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| tornado 0.6a -2         |   11920 us |    6932 us |     1251726 | 34.38 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| quicklz 1.5.0 -2        |   12054 us |    7179 us |     1187468 | 32.62 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| density 0.14.2 -3       |    9822 us |   10258 us |     1606968 | 44.14 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|
| tornado 0.6a -3         |   19214 us |   11493 us |     1022778 | 28.09 | sorbet-stdlib-state-a2f6fab208ee500774c6faa9c62c228c3b21fe08|

Note: LZ5 has a downsize though. It's really bad in compressing small pieces of data. <1KB data sizes normally _grow_ after compression.
