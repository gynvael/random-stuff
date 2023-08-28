## A brute-forcing base64 extractor.
by Gynvael Coldwind (http://gynvael.coldwind.pl)

This tool basically goes through the whole file and finds every base64-looking string and decodes it. It produces A LOT of output, especially that a lot of normal ASCII strings are valid base64 strings.

`usage: brute_base64.py <fname>`
