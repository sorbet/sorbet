Sorbet internally uses the LZ4 compressor to compress internal datastructures, with default compression level 1.
Decision to use it was made via the following benchmark (run with https://github.com/inikep/lzbench on Stripe's developer machines).

Note that we care a lot about decompression speed, followed by size. We care a bit about compression speed, but not as much.

The numbers below are measured on the GlobalState of Sorbet for the stdlib (created at commit 5753872590c7d982f5e12f7f967d4d2702c1b6fb by modifying serialize to disable compression).

Vanilla lz4 (bolded in the table below) with the default compression setting provides great decompression (~4GB/s) while compressing to around 1/3rd of the size and with ~500M/s compression speed.

| Compressor name         | Compression| Decompress.| Compr. size | Ratio | Filename |
| ---------------         | -----------| -----------| ----------- | ----- | -------- |
| lz4fast 1.9.3 -17       |    8368 us |    2023 us |     3555129 | 57.23 | uncompressed-gs-rle|
| lzsse8fast 2019-04-18   |   30112 us |    2065 us |     2301226 | 37.05 | uncompressed-gs-rle|
| lzsse4fast 2019-04-18   |   29646 us |    2189 us |     2306934 | 37.14 | uncompressed-gs-rle|
| lz4fast 1.9.3 -3        |   11855 us |    2196 us |     2696408 | 43.41 | uncompressed-gs-rle|
| **lz4 1.9.3**           |**13133 us**| **2233 us**| **2506875** |**40.36**|**uncompressed-gs-rle**|
| lizard 1.0 -14          |   82862 us |    2568 us |     2105097 | 33.89 | uncompressed-gs-rle|
| lizard 1.0 -13          |   76738 us |    2584 us |     2133780 | 34.35 | uncompressed-gs-rle|
| lizard 1.0 -12          |   48377 us |    2604 us |     2185818 | 35.19 | uncompressed-gs-rle|
| lizard 1.0 -10          |   15659 us |    2649 us |     2512351 | 40.45 | uncompressed-gs-rle|
| lizard 1.0 -11          |   22017 us |    2729 us |     2390097 | 38.48 | uncompressed-gs-rle|
| density 0.14.2 -1       |    6917 us |    4306 us |     4020138 | 64.72 | uncompressed-gs-rle|
| pithy 2011-12-24 -9     |   17397 us |    4467 us |     2269661 | 36.54 | uncompressed-gs-rle|
| pithy 2011-12-24 -6     |   15609 us |    4523 us |     2296277 | 36.97 | uncompressed-gs-rle|
| pithy 2011-12-24 -3     |   14433 us |    4726 us |     2384371 | 38.39 | uncompressed-gs-rle|
| pithy 2011-12-24 -0     |   14011 us |    4732 us |     2504507 | 40.32 | uncompressed-gs-rle|
| snappy 2020-07-11       |   16471 us |    5377 us |     2546948 | 41.00 | uncompressed-gs-rle|
| shrinker 0.1            |   18654 us |    5877 us |     2295727 | 36.96 | uncompressed-gs-rle|
| density 0.14.2 -2       |   13143 us |    6213 us |     3063046 | 49.31 | uncompressed-gs-rle|
| lzvn 2017-03-08         |  117788 us |    7422 us |     1977658 | 31.84 | uncompressed-gs-rle|
| lzfse 2017-03-08        |  104849 us |    8316 us |     1686341 | 27.15 | uncompressed-gs-rle|
| zstd 1.4.8 -1           |   18930 us |    8458 us |     1893112 | 30.48 | uncompressed-gs-rle|
| zstd 1.4.8 -2           |   22343 us |    9156 us |     1799672 | 28.97 | uncompressed-gs-rle|
| zstd 1.4.8 -3           |   32332 us |    9553 us |     1703713 | 27.43 | uncompressed-gs-rle|
| zstd 1.4.8 -4           |   38472 us |    9710 us |     1694480 | 27.28 | uncompressed-gs-rle|
| fastlz 0.5.0 -1         |   23567 us |    9840 us |     2572214 | 41.41 | uncompressed-gs-rle|
| lzf 3.6 -1              |   21870 us |    9996 us |     2535140 | 40.81 | uncompressed-gs-rle|
| zstd 1.4.8 -5           |   66964 us |   10048 us |     1641995 | 26.43 | uncompressed-gs-rle|
| lzo1c 2.10 -1           |   27047 us |   10273 us |     2505783 | 40.34 | uncompressed-gs-rle|
| lzf 3.6 -0              |   22583 us |   10285 us |     2607304 | 41.97 | uncompressed-gs-rle|
| lzo1b 2.10 -1           |   32938 us |   10426 us |     2435090 | 39.20 | uncompressed-gs-rle|
| fastlz 0.5.0 -2         |   23825 us |   10440 us |     2467538 | 39.72 | uncompressed-gs-rle|
| quicklz 1.5.0 -2        |   30271 us |   11437 us |     2089850 | 33.64 | uncompressed-gs-rle|
| quicklz 1.5.0 -1        |   17231 us |   11542 us |     2295992 | 36.96 | uncompressed-gs-rle|
| lzo1x 2.10 -1           |   13649 us |   11555 us |     2518928 | 40.55 | uncompressed-gs-rle|
| lzo1y 2.10 -1           |   14106 us |   11652 us |     2513456 | 40.46 | uncompressed-gs-rle|
| lzo1f 2.10 -1           |   29240 us |   11894 us |     2476132 | 39.86 | uncompressed-gs-rle|
| lzrw 15-Jul-1991 -3     |   22866 us |   12993 us |     2630980 | 42.36 | uncompressed-gs-rle|
| lzjb 2010               |   24327 us |   13664 us |     3097302 | 49.86 | uncompressed-gs-rle|
| lzrw 15-Jul-1991 -5     |   49182 us |   13672 us |     2333348 | 37.56 | uncompressed-gs-rle|
| lzrw 15-Jul-1991 -4     |   23145 us |   14030 us |     2522882 | 40.62 | uncompressed-gs-rle|
| lzrw 15-Jul-1991 -1     |   28190 us |   14839 us |     2891308 | 46.55 | uncompressed-gs-rle|
| tornado 0.6a -1         |   22015 us |   15421 us |     2545817 | 40.99 | uncompressed-gs-rle|
| tornado 0.6a -2         |   28389 us |   17302 us |     2173688 | 34.99 | uncompressed-gs-rle|
| density 0.14.2 -3       |   24069 us |   20378 us |     2748562 | 44.25 | uncompressed-gs-rle|
| tornado 0.6a -3         |   39060 us |   26313 us |     1798805 | 28.96 | uncompressed-gs-rle|
