/* A copy of st_lookup except that it can use a precomputed hash. */
int st_lookup_with_precomputed_hash(st_table *tab, st_data_t key, st_data_t *value, st_hash_t hash) {
    st_index_t bin;

retry:
    if (tab->bins == NULL) {
        bin = find_entry(tab, hash, key);
        if (EXPECT(bin == REBUILT_TABLE_ENTRY_IND, 0)) {
            goto retry;
        }
        if (bin == UNDEFINED_ENTRY_IND) {
            return 0;
        }
    } else {
        bin = find_table_entry_ind(tab, hash, key);
        if (EXPECT(bin == REBUILT_TABLE_ENTRY_IND, 0)) {
            goto retry;
        }
        if (bin == UNDEFINED_ENTRY_IND) {
            return 0;
        }
        bin -= ENTRY_BASE;
    }
    if (value != 0) {
        *value = tab->entries[bin].record;
    }
    return 1;
}
