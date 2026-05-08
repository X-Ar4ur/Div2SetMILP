# Golden balanced-bit sets

Each `<cipher>_R<rounds>_<activebits>.json` file contains the ground-truth
balanced output bits for one (cipher, rounds, activebits) configuration,
sourced from the published 2-subset BDP results.

Format:

```json
{
  "cipher": "PRESENT",
  "rounds": 9,
  "activebits": "60",
  "paper_ref": "Xiang et al., CRYPTO 2016, Table 6",
  "balanced_bits": ["x...", "x..."],
  "n_balanced": 1,
  "notes": "..."
}
```

`make_tables.py --table 1` joins these against the `result_*.txt` files
under `data/division/<cipher>/milp/` to populate the correctness table.

To populate this directory after a clean reproduction run, copy the
balanced-bit list from `parse_log.py --with-balanced` into the JSON
template above and commit. We treat the *current verified* run as the
golden until the literature comparison is published.
