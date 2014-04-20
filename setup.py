#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
test with:
    python setup.py build_ext --inplace
"""
DESCRIPTION = ("Fast text search for large texts using the FM-index algorithm"
               "hopefully this will be a common interface for other libs too")
LONG_DESCRIPTION = """
**fmindex-py** is Python C-API bindings for fmindexV2 which is originally 
available at

http://www.di.unipi.it/~ferragin//Libraries/fmindexV2/index.html

due to the need to modify the makefile, the source is included in the distro.

License is GPLv2 due to fmindexV2 and ds_ssort.  see the folders fmindexV2 and 
subfolder ds_ssort for copyright info.

see tests for usage.

text input below SMALLFILESIZE of 51201 bytes (50 kB) 
and SMALLSMALLFILESIZE 1025 bytes 1 kB uses the Boyer-Moore algorithm for search
and SMALLSMALLFILESIZE doesn't compress the text
"""

DISTNAME = 'fmindex-py'
LICENSE = 'GPLv2'
AUTHORS = "Nick Conway"
EMAIL = "a.grinner@gmail.com"
URL = ""
DOWNLOAD_URL = ''
CLASSIFIERS = [
    'Development Status :: 1 - Beta',
    'Environment :: Console',
    'Intended Audience :: Science/Research',
    'Programming Language :: Python',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 2.7',
    'Programming Language :: Python :: 3.2',
    'Programming Language :: Python :: 3.3',
    'Programming Language :: Cython',
    'Topic :: Scientific/Engineering',
]

from distutils.core import setup, Extension
import numpy.distutils.misc_util
import os
from os.path import join as pjoin
from os.path import relpath as rpath
import sys
import subprocess

fmindexpy_path = os.path.abspath(os.path.dirname(__file__))
package_path = pjoin(fmindexpy_path, 'fmindex')
fmindex_version = 'V2'
fmindexV2_home = pjoin(package_path, "src", "fmindex" + fmindex_version)
ds_ssort_home = pjoin(fmindexV2_home, 'ds_ssort')

if not os.path.exists(fmindexV2_home):
    raise IOError("fmindex %s missing, please add to fmindexpy root directory" % 
        (fmindex_version))

do_remake_fmindex = True
if do_remake_fmindex:
    command = "make"
    # command = "make clean; make"
    kwargs = {"timeout": 60 } if sys.version_info[0] == 3 else {}
    # will raise Error if doesn't work
    subprocess.check_call(["/bin/bash", "-c", 
      'cd %s; %s' % (ds_ssort_home, command)], **kwargs)
    subprocess.check_call(["/bin/bash", "-c", 
      'cd %s; %s' % (fmindexV2_home, command)], **kwargs)
    
objects_files = [pjoin(fmindexV2_home,'fm_index.a'),
                    pjoin(ds_ssort_home, 'ds_ssort.a')]

fmindex_ext = Extension('fmindex._fmindex',
                        sources=['fmindex/src/fmindex_py.c'],
                        extra_objects=objects_files,
                        include_dirs=[fmindexV2_home, 
                                    ds_ssort_home,
                                    'fmindex/src'],
                      )

# Find all fmindex data files
fmi_files = [rpath(pjoin(root, f), package_path) for root, _, files in
            os.walk(fmindexV2_home) for f in files]

setup(
    name=DISTNAME,
    maintainer=AUTHORS,
    packages=['fmindex'],
    package_data={'fmindex': fmi_files},
    ext_modules=[fmindex_ext],
    include_dirs=numpy.distutils.misc_util.get_numpy_include_dirs(),
    maintainer_email=EMAIL,
    description=DESCRIPTION,
    license=LICENSE,
    url=URL,
    download_url=DOWNLOAD_URL,
    long_description=LONG_DESCRIPTION,
    classifiers=CLASSIFIERS
)