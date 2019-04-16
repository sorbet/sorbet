Sorbet internally uses a lizard(LZ5) compressor to compress internal datastructures, with compression level 15.
Decision to use it was made via the following benchmark(run with https://github.com/inikep/lzbench)
Note that we care a lot about decompression speed, followed by size. We care a bit about compression speed, but not much.
The numbers below are measured on the GlobalState of Sorbet for the stdlib.
lz5 provides great decompression(~5 GB/s) while compressing to around 1/3rd of the size and with ~100M/s compression speed.

| Compressor name         | Compression| Decompress.| Compr. size | Ratio | Filename |
| ---------------         | -----------| -----------| ----------- | ----- | -------- |
| blosclz 2015-11-10 -1   |    1044 us |      42 us |     1529314 |100.00 | state|
| blosclz 2015-11-10 -3   |    1937 us |     225 us |     1256354 | 82.15 | state|
| lizard 1.0 -15          |   15981 us |     295 us |      561657 | 36.73 | state|
| lizard 1.0 -12          |    9864 us |     307 us |      579702 | 37.91 | state|
| lzsse8 2016-05-14 -6    |  167997 us |     311 us |      559791 | 36.60 | state|
| lzsse8 2016-05-14 -12   |  169642 us |     311 us |      559744 | 36.60 | state|
| lzsse8 2016-05-14 -16   |  170472 us |     312 us |      559744 | 36.60 | state|
| lizard 1.0 -19          |  329406 us |     313 us |      546238 | 35.72 | state|
| lz4hc 1.8.3 -12         |  128746 us |     317 us |      545460 | 35.67 | state|
| lzsse8 2016-05-14 -1    |   95875 us |     323 us |      578858 | 37.85 | state|
| lz4hc 1.8.3 -9          |   27944 us |     326 us |      546650 | 35.74 | state|
| lizard 1.0 -10          |    1974 us |     326 us |      621750 | 40.66 | state|
| lz4hc 1.8.3 -4          |   13990 us |     331 us |      552274 | 36.11 | state|
| lz4fast 1.8.3 -17       |    1229 us |     337 us |      816489 | 53.39 | state|
| lz4hc 1.8.3 -1          |   10867 us |     339 us |      560656 | 36.66 | state|
| lz4 1.8.3               |    1674 us |     342 us |      621711 | 40.65 | state|
| lz4fast 1.8.3 -3        |    1529 us |     349 us |      647944 | 42.37 | state|
| lzsse4 2016-05-14 -12   |  165404 us |     354 us |      570050 | 37.27 | state|
| lzsse4 2016-05-14 -16   |  161295 us |     354 us |      570050 | 37.27 | state|
| lzsse4 2016-05-14 -6    |  161199 us |     354 us |      570083 | 37.28 | state|
| lzsse4 2016-05-14 -1    |   90527 us |     364 us |      588745 | 38.50 | state|
| yappy 2014-03-22 -100   |   17794 us |     384 us |      594391 | 38.87 | state|
| yappy 2014-03-22 -10    |   12409 us |     393 us |      598969 | 39.17 | state|
| yappy 2014-03-22 -1     |    8822 us |     420 us |      613176 | 40.09 | state|
| lizard 1.0 -39          |  332227 us |     438 us |      528083 | 34.53 | state|
| lizard 1.0 -22          |    7595 us |     444 us |      572522 | 37.44 | state|
| lizard 1.0 -29          |  454892 us |     464 us |      495747 | 32.42 | state|
| lizard 1.0 -32          |    7509 us |     464 us |      566649 | 37.05 | state|
| lizard 1.0 -35          |   15435 us |     466 us |      544036 | 35.57 | state|
| lizard 1.0 -20          |    2374 us |     471 us |      632349 | 41.35 | state|
| lizard 1.0 -25          |   61760 us |     478 us |      520369 | 34.03 | state|
| lzsse2 2016-05-14 -16   |  169254 us |     481 us |      574274 | 37.55 | state|
| lzsse2 2016-05-14 -6    |  165553 us |     481 us |      574345 | 37.56 | state|
| lzsse2 2016-05-14 -12   |  173905 us |     493 us |      574274 | 37.55 | state|
| lzsse2 2016-05-14 -1    |   80359 us |     505 us |      602349 | 39.39 | state|
| pithy 2011-12-24 -6     |    2413 us |     564 us |      601835 | 39.35 | state|
| lizard 1.0 -30          |    2954 us |     571 us |      590693 | 38.62 | state|
| pithy 2011-12-24 -0     |    1868 us |     590 us |      625086 | 40.87 | state|
| pithy 2011-12-24 -3     |    1982 us |     595 us |      611793 | 40.00 | state|
| lizard 1.0 -45          |   57685 us |     609 us |      498525 | 32.60 | state|
| pithy 2011-12-24 -9     |    2852 us |     610 us |      597896 | 39.10 | state|
| snappy 1.1.4            |    2083 us |     651 us |      628378 | 41.09 | state|
| density 0.14.2 -1       |     943 us |     655 us |     1063698 | 69.55 | state|
| lizard 1.0 -49          |  477435 us |     699 us |      479283 | 31.34 | state|
| lizard 1.0 -42          |    8684 us |     720 us |      535154 | 34.99 | state|
| lizard 1.0 -40          |    3606 us |     847 us |      577344 | 37.75 | state|
| blosclz 2015-11-10 -6   |    3464 us |     910 us |      691501 | 45.22 | state|
| wflz 2015-09-16         |    5301 us |    1054 us |      669268 | 43.76 | state|
| blosclz 2015-11-10 -9   |    3596 us |    1088 us |      640463 | 41.88 | state|
| density 0.14.2 -2       |    1856 us |    1098 us |      814888 | 53.28 | state|
| lzvn 2017-03-08         |   21156 us |    1190 us |      542055 | 35.44 | state|
| shrinker 0.1            |    4367 us |    1223 us |      584006 | 38.19 | state|
| lzo1x 2.09 -11          |    1823 us |    1325 us |      639058 | 41.79 | state|
| lzo1x 2.09 -1           |    1870 us |    1329 us |      625952 | 40.93 | state|
| lzo1x 2.09 -12          |    1847 us |    1331 us |      632732 | 41.37 | state|
| lzo1x 2.09 -15          |    1893 us |    1340 us |      628414 | 41.09 | state|
| lzf 3.6 -1              |    3925 us |    1362 us |      635424 | 41.55 | state|
| quicklz 1.5.0 -3        |   25390 us |    1368 us |      538619 | 35.22 | state|
| lzo1b 2.09 -999         |  152773 us |    1379 us |      550011 | 35.96 | state|
| zstd 1.3.8 -15          |   96078 us |    1381 us |      441293 | 28.86 | state|
| lzo1y 2.09 -1           |    1867 us |    1392 us |      615873 | 40.27 | state|
| lzfse 2017-03-08        |   17341 us |    1402 us |      474874 | 31.05 | state|
| zstd 1.3.8 -11          |   41213 us |    1405 us |      444490 | 29.06 | state|
| zstd 1.3.8 -8           |   20073 us |    1418 us |      446456 | 29.19 | state|
| lzf 3.6 -0              |    3754 us |    1421 us |      641443 | 41.94 | state|
| lzo1c 2.09 -3           |    5627 us |    1462 us |      618432 | 40.44 | state|
| lzo1c 2.09 -99          |   13549 us |    1482 us |      574141 | 37.54 | state|
| zstd 1.3.8 -5           |   10874 us |    1497 us |      460279 | 30.10 | state|
| lzo1b 2.09 -99          |   14754 us |    1510 us |      575906 | 37.66 | state|
| lzo1c 2.09 -999         |   59081 us |    1518 us |      550239 | 35.98 | state|
| lzo1f 2.09 -1           |    6097 us |    1519 us |      617771 | 40.40 | state|
| zstd 1.3.8 -2           |    4128 us |    1519 us |      481874 | 31.51 | state|
| zstd 1.3.8 -1           |    3079 us |    1524 us |      493323 | 32.26 | state|
| lzo1a 2.09 -99          |   12761 us |    1524 us |      606867 | 39.68 | state|
| lzo1c 2.09 -1           |    5598 us |    1536 us |      622838 | 40.73 | state|
| lzo1f 2.09 -999         |   58016 us |    1561 us |      541716 | 35.42 | state|
| yalz77 2015-09-19 -12   |   59350 us |    1562 us |      591147 | 38.65 | state|
| yalz77 2015-09-19 -8    |   43026 us |    1573 us |      592867 | 38.77 | state|
| lzo1b 2.09 -9           |    8064 us |    1582 us |      595559 | 38.94 | state|
| lzo1b 2.09 -3           |    6581 us |    1586 us |      617654 | 40.39 | state|
| lzo1b 2.09 -1           |    6551 us |    1587 us |      622277 | 40.69 | state|
| lzo1c 2.09 -6           |    7174 us |    1590 us |      603054 | 39.43 | state|
| lzo1 2.09 -99           |   12206 us |    1594 us |      614013 | 40.15 | state|
| yalz77 2015-09-19 -4    |   27713 us |    1604 us |      597753 | 39.09 | state|
| lzo1a 2.09 -1           |    5063 us |    1608 us |      637555 | 41.69 | state|
| lzo1c 2.09 -9           |    9704 us |    1611 us |      591401 | 38.67 | state|
| lzo1b 2.09 -6           |    6775 us |    1640 us |      605900 | 39.62 | state|
| fastlz 0.1 -1           |    4147 us |    1646 us |      643223 | 42.06 | state|
| lzo1x 2.09 -999         |  253930 us |    1653 us |      519499 | 33.97 | state|
| gipfeli 2016-07-13      |    2731 us |    1654 us |      593697 | 38.82 | state|
| fastlz 0.1 -2           |    3645 us |    1681 us |      634357 | 41.48 | state|
| lzo1 2.09 -1            |    5109 us |    1687 us |      644397 | 42.14 | state|
| lzo1y 2.09 -999         |  219082 us |    1691 us |      515506 | 33.71 | state|
| lzo1z 2.09 -999         |  244205 us |    1697 us |      514499 | 33.64 | state|
| zstd 1.3.8 -22          |  531099 us |    1713 us |      418285 | 27.35 | state|
| zstd 1.3.8 -18          |  285765 us |    1713 us |      419821 | 27.45 | state|
| yalz77 2015-09-19 -1    |   17454 us |    1738 us |      618685 | 40.46 | state|
| lzg 1.0.8 -8            |  144809 us |    1758 us |      554068 | 36.23 | state|
| lzg 1.0.8 -6            |  101255 us |    1803 us |      566178 | 37.02 | state|
| lzg 1.0.8 -4            |   97989 us |    1827 us |      578561 | 37.83 | state|
| lzg 1.0.8 -1            |   92783 us |    1846 us |      605284 | 39.58 | state|
| libdeflate 1.0 -12      |  212818 us |    1945 us |      447587 | 29.27 | state|
| libdeflate 1.0 -6       |   13218 us |    1954 us |      457398 | 29.91 | state|
| libdeflate 1.0 -9       |   84526 us |    1979 us |      454991 | 29.75 | state|
| xpack 2016-06-02 -6     |   26408 us |    1985 us |      434726 | 28.43 | state|
| libdeflate 1.0 -3       |    8730 us |    1990 us |      471271 | 30.82 | state|
| lzrw 15-Jul-1991 -3     |    3527 us |    1997 us |      647617 | 42.35 | state|
| xpack 2016-06-02 -9     |   58034 us |    2019 us |      431720 | 28.23 | state|
| libdeflate 1.0 -1       |    8023 us |    2057 us |      483876 | 31.64 | state|
| quicklz 1.5.0 -1        |    2630 us |    2059 us |      585294 | 38.27 | state|
| xpack 2016-06-02 -1     |   10451 us |    2094 us |      476875 | 31.18 | state|
| lzjb 2010               |    3287 us |    2215 us |      675281 | 44.16 | state|
| lzrw 15-Jul-1991 -5     |   10384 us |    2222 us |      602154 | 39.37 | state|
| quicklz 1.5.0 -2        |    5073 us |    2238 us |      567629 | 37.12 | state|
| lzo2a 2.09 -999         |   46713 us |    2275 us |      533981 | 34.92 | state|
| lzrw 15-Jul-1991 -1     |    4472 us |    2283 us |      680382 | 44.49 | state|
| lzrw 15-Jul-1991 -4     |    3548 us |    2304 us |      638055 | 41.72 | state|
| brieflz 1.2.0 -9        |  165412 us |    2329 us |      519080 | 33.94 | state|
| brieflz 1.2.0 -6        |   25273 us |    2342 us |      520808 | 34.06 | state|
| tornado 0.6a -2         |    4516 us |    2404 us |      587279 | 38.40 | state|
| brieflz 1.2.0 -3        |   11715 us |    2452 us |      547391 | 35.79 | state|
| tornado 0.6a -1         |    3233 us |    2462 us |      665878 | 43.54 | state|
| brieflz 1.2.0 -1        |    6982 us |    2554 us |      566645 | 37.05 | state|
| density 0.14.2 -3       |    3577 us |    2794 us |      741898 | 48.51 | state|
| lzmat 1.01              |   33264 us |    2946 us |      518522 | 33.91 | state|
| crush 1.0 -2            |  760155 us |    3416 us |      494055 | 32.31 | state|
| crush 1.0 -1            |   68461 us |    3502 us |      500762 | 32.74 | state|
| brotli 1.0.7 -8         |   76095 us |    3506 us |      401831 | 26.28 | state|
| brotli 1.0.7 -2         |    7779 us |    3507 us |      465435 | 30.43 | state|
| brotli 1.0.7 -5         |   35285 us |    3519 us |      405696 | 26.53 | state|
| crush 1.0 -0            |   35640 us |    3665 us |      515452 | 33.70 | state|
| ucl_nrv2e 1.03 -9       |  827670 us |    3876 us |      479352 | 31.34 | state|
| zlib 1.2.11 -9          |   96948 us |    3909 us |      459286 | 30.03 | state|
| slz_zlib 1.0.0 -2       |    4583 us |    3917 us |      585256 | 38.27 | state|
| slz_zlib 1.0.0 -3       |    4589 us |    3922 us |      583422 | 38.15 | state|
| ucl_nrv2d 1.03 -9       |  796794 us |    3931 us |      480009 | 31.39 | state|
| zlib 1.2.11 -6          |   32701 us |    3936 us |      461340 | 30.17 | state|
| slz_zlib 1.0.0 -1       |    4578 us |    3945 us |      592465 | 38.74 | state|
| ucl_nrv2e 1.03 -6       |   53364 us |    3959 us |      483539 | 31.62 | state|
| ucl_nrv2d 1.03 -6       |   52991 us |    4028 us |      484319 | 31.67 | state|
| ucl_nrv2b 1.03 -9       |  809615 us |    4088 us |      483711 | 31.63 | state|
| brotli 1.0.7 -0         |    3183 us |    4201 us |      536119 | 35.06 | state|
| ucl_nrv2b 1.03 -6       |   52663 us |    4211 us |      486851 | 31.83 | state|
| brotli 1.0.7 -11        | 4144788 us |    4231 us |      349420 | 22.85 | state|
| zlib 1.2.11 -1          |   14010 us |    4273 us |      498242 | 32.58 | state|
| ucl_nrv2e 1.03 -1       |   28885 us |    4439 us |      520976 | 34.07 | state|
| ucl_nrv2d 1.03 -1       |   28572 us |    4452 us |      522192 | 34.15 | state|
| ucl_nrv2b 1.03 -1       |   28900 us |    4711 us |      528504 | 34.56 | state|
| tornado 0.6a -4         |    9542 us |    5649 us |      479900 | 31.38 | state|
| tornado 0.6a -3         |    7574 us |    5760 us |      480848 | 31.44 | state|
| zling 2016-01-10 -1     |   22383 us |    7873 us |      499812 | 32.68 | state|
| zling 2016-01-10 -0     |   22254 us |    8120 us |      503265 | 32.91 | state|
| zling 2016-01-10 -2     |   20813 us |    8206 us |      497847 | 32.55 | state|
| zling 2016-01-10 -4     |   25507 us |    8272 us |      494939 | 32.36 | state|
| zling 2016-01-10 -3     |   24679 us |    8328 us |      496589 | 32.47 | state|
| tornado 0.6a -10        |  274954 us |    9699 us |      428644 | 28.03 | state|
| tornado 0.6a -7         |   79165 us |    9883 us |      430480 | 28.15 | state|
| tornado 0.6a -16        |  452921 us |    9901 us |      407153 | 26.62 | state|
| tornado 0.6a -13        |  185248 us |    9958 us |      411690 | 26.92 | state|
| tornado 0.6a -6         |   45068 us |    9995 us |      437039 | 28.58 | state|
| tornado 0.6a -5         |   40086 us |   10145 us |      443377 | 28.99 | state|
| lzma 16.04 -9           |  447414 us |   18899 us |      351297 | 22.97 | state|
| lzma 16.04 -5           |  339966 us |   18913 us |      355339 | 23.24 | state|
| lzma 16.04 -2           |   59062 us |   19329 us |      384979 | 25.17 | state|
| lzma 16.04 -4           |   88962 us |   19655 us |      381020 | 24.91 | state|
| lzma 16.04 -0           |   46991 us |   19707 us |      391481 | 25.60 | state|
| xz 5.2.3 -6             |  348953 us |   22568 us |      351329 | 22.97 | state|
| xz 5.2.3 -3             |  121207 us |   23217 us |      377450 | 24.68 | state|
| xz 5.2.3 -0             |   65634 us |   24401 us |      392201 | 25.65 | state|
| xz 5.2.3 -9             |  378634 us |   24935 us |      351329 | 22.97 | state|
| lzlib 1.8 -6            |  328657 us |   25282 us |      354731 | 23.20 | state|
| lzlib 1.8 -9            |  485577 us |   25727 us |      348902 | 22.81 | state|
| lzlib 1.8 -3            |  147220 us |   25866 us |      385032 | 25.18 | state|
| lzlib 1.8 -0            |   43026 us |   28007 us |      394003 | 25.76 | state|
| ---------------         | -----------| -----------| ----------- | ----- | -------- |
| Compressor name         | Compression| Decompress.| Compr. size | Ratio | Filename |

Note: LZ5 has a downsize though. It's really bad in compressing small pieces of data. <1KB data sizes normally _grow_ after compression.
