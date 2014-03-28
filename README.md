fmindex-py
======

Search large texts using the FM-index algorithm

License: GPLv2 http://www.gnu.org/licenses/gpl-2.0.html


Python C-API bindings for fmindexV2 which is originally available at

http://www.di.unipi.it/~ferragin//Libraries/fmindexV2/index.html

due to the need to modify the makefile (for fPIC for Linux) and ease of 
distribution, the source is included in the distro.

License is GPLv2 due to fmindexV2 and ds_ssort.  see the folders fmindexV2 
and subfolder ds_ssort for that copyright info.  Also the README.txt in fmindexV2 
has more info.

bindings are Copyright (C) 2014 Nick Conway

see tests for usage.

text input below SMALLFILESIZE of 51201 bytes (50 kB) 
and SMALLSMALLFILESIZE 1025 bytes 1 kB uses the Boyer-Moore algorithm for search
and SMALLSMALLFILESIZE doesn't compress the text

    from fmindex import FMindex

    my_fmi = FMindex(from)