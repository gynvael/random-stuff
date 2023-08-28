# A brute-forcing zlib decompressor

by Gynvael Coldwind (http://gynvael.coldwind.pl)

_Note: This isn't really a tool - it's a code snippet that works by accident. No guarantee is given that it does its job._

It's somewhat useful to extract data from corrupted ZIP archives and other binary blobs which might contains DEFLATE streams.

Creates a lot of garbage files - that's normal.

Might not work good with larger streams (i.e. it will only dump the first several hundred bytes of the stream) - you can manually change this in the code though.
