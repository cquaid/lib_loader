lib_loader
==========

This is my attempt at an rtld (RunTime Link eDitor)

It mostly works... it requires a number of "fixups" to be defined before loading a library.

The way the fixup function code works you can substitute a function for another.
If you're library requires the malloc symbol and you want to hook it with something else:
    #include "librtld.h"
	/* ... */
	void* my_malloc(size_t l){ /* ... */}
	/* ... */
	Anchor malloc_fixup = { "malloc", my_malloc };
	/* ... */
	add_fixup_anchor(&malloc_fixup);


The test/zlib/zlib_list.c has a good example of how to do the fixup.

There isn't an install rule yet, so to use the tests you'll need to do
<pre>
LD_LIBRARY_PATH=$(pwd)/lib:$LD_LIBRARY_PATH
</pre>
before running the tests (assuming you're in the root source directory)

To test test_loader:
<pre>
 run make
 ./test/lib_test/test_loader ./test/lib_test/lib.so
</pre>

To test libz_loader:
<pre>
 modify test/zlib/libz_loader.c to include the path to your libz.so
 run make
 to deflate: test zlib/libz_loader &lt; to_compress &gt; ouput_file
 to inflate: test/zlib/libz_loader -d &lt; compressed_file &gt; decompressed_file
</pre>

Things that need to be done:
<pre>
 * Write documentation
 * Refactor the code
 * Test everything
</pre>
