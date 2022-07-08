#!/usr/bin/env python3
import sys
import lmdb
import pprint
import itertools
import csv

# This script is an assortment of code that I have found useful when digging
# into LMDB cache sizes in the past.
#
# It's not tested, and is mostly useful for invoking by-hand on a Sorbet cache,
# but it should at least serve as a way to document gotchas and things to look
# for when attempting to debug Sorbet cache sizes.
#
#


def list_flavors(env, txn):
    unnamed_dbi = env.open_db()
    with txn.cursor(unnamed_dbi) as cursor:
        flavors = []
        for key, _val in cursor:
            flavors.append(key)

    return flavors


def all_used_bytes(stats):
    return stats['psize'] * (stats['branch_pages'] + stats['leaf_pages'] + stats['overflow_pages'])

def print_cache_size(env):
    total_bytes = 0

    with env.begin() as txn:
        stats = env.stat()
        print('Stats for unnamed database:')
        pprint.pprint(stats)
        total_bytes += all_used_bytes(stats)

        flavors = list_flavors(env, txn)

    for flavor in flavors:
        # For some reason we have to open the DBs once outside of a transaction
        # before we can stat them 🤔
        flavor_dbi = env.open_db(flavor)

    with env.begin() as txn:
        for flavor in flavors:
            flavor_dbi = env.open_db(flavor)
            stats = txn.stat(flavor_dbi)
            print(f'\nStats for {flavor} database:')
            pprint.pprint(stats)
            total_bytes += all_used_bytes(stats)

    print(f'Total bytes: {total_bytes}')

def entry_sizes(env):
    results = {}

    with env.begin() as txn:
        stats = env.stat()
        flavors = list_flavors(env, txn)

    for flavor in flavors:
        # For some reason we have to open the DBs once outside of a transaction
        # before we can stat them 🤔
        flavor_dbi = env.open_db(flavor)

    with env.begin(buffers=True) as txn:
        for flavor in flavors:
            print(f'flavor={flavor}... ', file=sys.stderr)
            results[flavor] = []

            flavor_dbi = env.open_db(flavor)
            with txn.cursor(flavor_dbi) as cursor:
                for idx, (key, val) in enumerate(cursor):
                    # Not every key is the same...
                    # There is a single `GlobalState` key that's huge, and also
                    # the sorbet version string that's always the same size.
                    # But for the time being this script doesn't look at any
                    # key specially, and just racks up all the sizes. But it
                    # does put a big skew on things like mean
                    if idx % 50000 == 0:
                        print(f'idx={idx}... ', file=sys.stderr)

                    # 8 is the bytes LMDB needs to remember a single key value pair
                    size = 8 + len(key) + len(val)
                    # if size > 100000:
                    #     print(f'idx={idx} key={bytes(key)} val={val} size={size}')
                    results[flavor].append(size)

    return results

def summarize_entry_sizes(results):
    for flavor in results.keys():
        print(f'flavor={flavor}')
        column = results[flavor]

        bigger_than_2040 = 0
        total_bytes = 0
        wasted_overflow_bytes = 0
        for size in column:
            total_bytes += size
            if size > 2040:
                # 2040 is size of the largest record that can fit wholly within
                # a leaf page
                bigger_than_2040 += 1
                # 4080 is the size that fits in an overflow page because the
                # other 16 bytes are used to link the overflow pages back into
                # the B-tree. Values larger than 2040 will be sent into
                # overflow pages, where the whole overflow page will be owned
                # by that record, regardless of how much of the space it needs.
                #
                # This is only wasted bytes in overflow pages--this doesn't
                # account at all for fragmentation that might occur in leave
                # pages (I haven't been able to figure out how to compute that)
                #
                # 
                wasted_overflow_bytes += 4080 - size % 4080

        print(f'count={len(column)}')
        print(f'average={total_bytes / len(column)}')
        print(f'bigger_than_2040={bigger_than_2040}')
        print(f'wasted_overflow_bytes={wasted_overflow_bytes}')
        print('')

def write_entry_sizes_csv(results):
    headers = results.keys()
    outfile = 'out.csv'
    with open(outfile, 'w') as f:
        rows = [
            ['' if not b else b for b in i] for i in itertools.zip_longest(*[results[c] for c in headers])
        ]
        write = csv.writer(f)
        write.writerow([header.decode('ascii') for header in headers])
        write.writerows(rows)

    print(f'Wrote CSV to {outfile}')


def main(cache_dir):
    env = lmdb.open(cache_dir, max_dbs=128, create=False, readonly=True)

    print_cache_size(env)
    sizes = entry_sizes(env)
    summarize_entry_sizes(sizes)
    write_entry_sizes_csv(sizes)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'usage: {sys.argv[0]} <cache-dir>', file=sys.stderr)
        exit(1)

    main(sys.argv[1])
