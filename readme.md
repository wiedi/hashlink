# hashlink - create hardlinks from hashes.

### Usage

	hashlink <source-index> <source-dir> <target-index> <target-dir>


### Function

hashlink uses two index files and two directory trees.
These index files have the following format:

	<hashkey> <filename>

Files from the source-dir tree, identified by hashes from the source-index are hardlinked into the target-dir according to the target-index.

Entries not found in one of the indexes are skipped.

### Use case

This tool can be useful as a pre-step to rsync where large portions of the data exists on both sides, but many files have been renamed.

### hash

The hashkey can by any type of hashsum. It is only used as an identifier/key.

For a secure hash function with great performance [BLAKE2](https://blake2.net) can be recommended.

Example:

	find . -type f -print0 | xargs -0 b2sum -l 256 > index.b2

