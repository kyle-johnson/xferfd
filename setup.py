from distutils.core import setup, Extension

module1 = Extension('xferfd',
                    sources = ['xferfd.c'])

setup (name = 'Transfer File Descriptor',
       version = '1.0',
       description = 'This wraps sendmsg() for sending file descriptors between processes.',
       ext_modules = [module1])