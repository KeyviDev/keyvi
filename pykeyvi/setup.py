from setuptools import setup, Extension
from setuptools.command.install import install
import distutils.command.build as _build
from Cython.Distutils import build_ext
import os
import sys
import pkg_resources

# workaround for autwrap bug (includes incompatible boost)
autowrap_data_dir = "autowrap_includes"

dictionary_sources = os.path.abspath('../keyvi')

additional_compile_flags = []

linklibraries_static_or_dynamic = [
             "boost_program_options",
             "boost_iostreams",
             "boost_filesystem",
             "boost_system",
             "boost_regex",
             "boost_thread",
             "snappy"
             ]

linklibraries = ["tpie",
             "z"
             ]

extra_link_arguments = []

if (sys.platform == 'darwin'):
    additional_compile_flags.append("-DOS_MACOSX")
    linklibraries.remove('boost_thread')
    linklibraries.append('boost_thread-mt')

#########################
# Custom 'build' command
#########################
class  build(_build.build):

    user_options=_build.build.user_options + \
        [('mode=',
          None,
          "build mode."),
         ('staticlinkboost',
          None,
          "special mode to statically link boost."),
        ]
    def initialize_options(self):
        _build.build.initialize_options(self)
        self.mode = 'release'
        self.staticlinkboost = False
        
    def run(self):
        global additional_compile_flags
        global linklibraries
        global linklibraries_static_or_dynamic
        global extra_link_arguments
        global ext_modules
        print "Building in {} mode".format(self.mode)
        
        if (self.mode == 'debug'):
            additional_compile_flags.append("-O0")
            additional_compile_flags.append("-ggdb3")
            additional_compile_flags.append("-fstack-protector")
        else:
            additional_compile_flags.append("-O3")
            
        if self.mode == 'coverage':
            additional_compile_flags.append("--coverage")
            linklibraries.append("gcov")

        # check linking
        if self.staticlinkboost:
            # set static
            extra_link_arguments = ['-Wl,-Bstatic']
            for lib in linklibraries_static_or_dynamic:
                extra_link_arguments.append("-l{}".format(lib))
            # reset to dynamic
            extra_link_arguments.append('-Wl,-Bdynamic')
            extra_link_arguments.append('-static-libstdc++')
            extra_link_arguments.append('-static-libgcc')
            # workaround: link librt explicitly
            linklibraries.append("rt")
        else:
            # no static linking, add the libs to dynamic linker
            linklibraries += linklibraries_static_or_dynamic

        # patch the compile flags
        for ext_m in ext_modules:
            flags = getattr(ext_m, 'extra_compile_args') + additional_compile_flags
            setattr(ext_m, 'extra_compile_args', flags)
            setattr(ext_m, 'libraries', linklibraries)
            args = getattr(ext_m, 'extra_link_args') + extra_link_arguments
            setattr(ext_m, 'extra_link_args', args)

        _build.build.run(self)

ext_modules = [Extension('pykeyvi',
                        include_dirs = [autowrap_data_dir,
                                        os.path.join(dictionary_sources, 'src/cpp'),
                                        os.path.join(dictionary_sources, '3rdparty/rapidjson/include'),
                                        os.path.join(dictionary_sources, '3rdparty/msgpack-c/include'),
                                        os.path.join(dictionary_sources, '3rdparty/utf8'),
                                        os.path.join(dictionary_sources, '3rdparty/misc'),
                                        os.path.join(dictionary_sources, '3rdparty/tpie/build/install/include'),
                                        os.path.join(dictionary_sources, '3rdparty/xchange/src')],
                        language = 'c++',
                        sources = ['src/pykeyvi.cpp'],
                        extra_compile_args=['-std=c++11', '-msse4.2'] + additional_compile_flags,
                        extra_link_args=[],
                        library_dirs = [os.path.join(dictionary_sources, '3rdparty/tpie/build/install/lib')],
                        libraries = linklibraries)]

PACKAGE_NAME = 'pykeyvi'

setup(
    name = PACKAGE_NAME,
    version = '0.1.1',
    description = 'Python bindings for keyvi',
    author = 'Hendrik Muhs',
    author_email = 'hendrik.muhs@gmail.com',
    license="ASL 2.0",
    cmdclass = {'build_ext': build_ext, 'build': build},
    ext_modules = ext_modules,
    zip_safe = False,
    url = 'https://github.com/cliqz/keyvi',
    download_url = 'https://github.com/cliqz/keyvi/tarball/v0.1',
    keywords = ['FST'],
    classifiers = [],
)
