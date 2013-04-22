lib_loader
==========

This is my attempt at an rtld (RunTime Link eDitor)

It mostly works... it requires a number of "fixups" to be defined before loading a library.

The test/zlib/zlib_list.c has a good example of how to do the fixup.

There isn't an install rule yet, so to use the tests you'll need to do
<pre>
LD_LIBRARY_PATH=$(PWD)/lib:$LD_LIBRARY_PATH
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
 to deflate: test zlib/libz_loader < to_compress > ouput_file
 to inflate: test/zlib/libz_loader -d < compressed_file > decompressed_file
</pre>

Things that need to be done:
<pre>
 * Write documentation
 * Refactor the code
 * Test everything
</pre>
