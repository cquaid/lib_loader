lib_loader
==========

This is my attempt at an rtld (RunTime Link eDitor)

It mostly works... it requires a number of "fixups" to be defined before loading a library.

The zlib/zlib_list.c has a good example of how to do the fixup.

To test test_loader:
<pre>
 run compile.sh
 ./lib_test/test_loader ./lib_test/llib.so
</pre>

To test libz_loader:
<pre>
 modify zlib/libz_loader.c to include the path to your libz.so
 run compile.sh
 to deflate: zlib/libz_loader < to_compress > ouput_file
 to inflage: zlib/libz_loader -d < compressed_file > decompressed_file
</pre>

Things that need to be done:
<pre>
 * Write a makefile
 * Write documentation
 * Refactor the code
 * Test everything
</pre>

If you're actually attempting to look at this right now: good luck.
