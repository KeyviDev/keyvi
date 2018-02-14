from setuptools import setup, Extension
import distutils.command.build as _build
import distutils.command.bdist as _bdist
import distutils.command.build_ext as _build_ext
import distutils.command.build_py as _build_py
import distutils.command.sdist as _sdist
import distutils.command.install_data as _install_data
import os
import sys
import subprocess
import multiprocessing
import shutil
import glob

from contextlib import contextmanager
from os import path

pykeyvi_pyx = '_core.pyx'
pykeyvi_cpp = '_core.cpp'
keyvi_cpp_source = '../keyvi'
keyvi_cpp = 'src/cpp'
keyvi_cpp_link = path.join(keyvi_cpp, 'keyvi')

try:
    cpu_count = multiprocessing.cpu_count()
except:
    cpu_count = 1


def generate_pykeyvi_source():
    addons = glob.glob('src/addons/*')
    pxds = glob.glob('src/pxds/*')
    converters = 'src/converters'
    converter_files = glob.glob(path.join(converters, '*'))
    max_modification_time = max([path.getmtime(fn) for fn in addons + pxds + converter_files])

    if not path.exists(pykeyvi_cpp) or max_modification_time > path.getmtime(pykeyvi_cpp):
        import autowrap.Main
        autowrap.Main.run(pxds, addons, [converters], pykeyvi_pyx)


@contextmanager
def symlink_keyvi():
    if not path.exists(keyvi_cpp_link):
        if not path.exists(keyvi_cpp):
            os.makedirs(keyvi_cpp)
        os.symlink(path.abspath(keyvi_cpp_source), keyvi_cpp_link)
        shutil.copy('../CMakeLists.txt', path.join(keyvi_cpp, 'CMakeLists.txt'))
        keyvi_source_path = os.path.realpath(os.path.join(os.getcwd(), keyvi_cpp_source))
        pykeyvi_source_path = os.path.join(os.getcwd(), keyvi_cpp_link)
        yield (pykeyvi_source_path, keyvi_source_path)
        os.unlink(keyvi_cpp_link)
        os.remove(path.join(keyvi_cpp, 'CMakeLists.txt'))
    else:
        yield None, None


with symlink_keyvi() as (pykeyvi_source_path, keyvi_source_path):
    # workaround for autowrap bug (includes incompatible boost)
    autowrap_data_dir = "autowrap_includes"

    dictionary_sources = path.abspath(keyvi_cpp_link)
    keyvi_build_dir = path.join(keyvi_cpp, 'build')
    keyvi_install_prefix = 'install'
    keyvi_install_dir = path.join(keyvi_build_dir, keyvi_install_prefix)
    keyvi_lib_dir = path.join(keyvi_install_dir, 'lib')

    additional_compile_flags = []

    # workaround for https://bitbucket.org/pypy/pypy/issues/2626/invalid-conversion-from-const-char-to-char
    if os.environ.get('PYTHON_VERSION', '') == 'pypy2':
        additional_compile_flags.append('-fpermissive')

    # re-map the source files in the debug symbol tables to there original location so that stepping in a debugger works
    if pykeyvi_source_path is not None:
        additional_compile_flags.append('-fdebug-prefix-map={}={}'.format(pykeyvi_source_path, keyvi_source_path))

    linklibraries_static_or_dynamic = [
        "boost_program_options",
        "boost_iostreams",
        "boost_filesystem",
        "boost_system",
        "boost_regex",
        "boost_thread",
        "snappy"
    ]

    linklibraries = [
        "tiny-process-library",
        "tpie",
        "z"
    ]

    mac_os_static_libs_dir = 'mac_os_static_libs'

    extra_link_arguments = []
    link_library_dirs = [keyvi_lib_dir]
    zlib_root = None

    if sys.platform == 'darwin':
        additional_compile_flags.append("-DOS_MACOSX")
        additional_compile_flags.append('-mmacosx-version-min=10.9')
        linklibraries_static_or_dynamic.remove('boost_thread')
        linklibraries_static_or_dynamic.append('boost_thread-mt')
        link_library_dirs.append(mac_os_static_libs_dir)
        extra_link_arguments.append('-L{}'.format(mac_os_static_libs_dir))

    #########################
    # Custom 'build' command
    #########################

    custom_user_options = [('mode=',
                            None,
                            "build mode."),
                           ('staticlinkboost',
                            None,
                            "special mode to statically link boost."),
                           ('zlib-root=',
                            None,
                            "zlib installation root"),
                           ]

    class custom_opts:

        parent = None

        def initialize_options(self):
            self.parent.initialize_options(self)
            self.mode = 'release'
            self.staticlinkboost = False
            self.zlib_root = None

        def run(self):
            global additional_compile_flags
            global linklibraries
            global linklibraries_static_or_dynamic
            global extra_link_arguments
            global ext_modules
            global zlib_root
            print ("Building in {0} mode".format(self.mode))

            if self.mode == 'debug':
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

            # custom zlib location
            if self.zlib_root:
                zlib_root = self.zlib_root
                for ext_m in ext_modules:
                    include_dirs = [path.join(self.zlib_root, "include")] + getattr(ext_m, 'include_dirs')
                    setattr(ext_m, 'include_dirs', include_dirs)
                    if sys.platform == 'darwin':
                        if not os.path.exists(mac_os_static_libs_dir):
                            os.makedirs(mac_os_static_libs_dir)
                        src_file = path.join(self.zlib_root, "lib", "libz.a")
                        dst_file = path.join(mac_os_static_libs_dir, "libz.a")
                        shutil.copyfile(src_file, dst_file)
                    else:
                        library_dirs = [path.join(self.zlib_root, "lib")] + getattr(ext_m, 'library_dirs')
                        setattr(ext_m, 'library_dirs', library_dirs)

            self.parent.run(self)

    class build(custom_opts, _build.build):
        parent = _build.build
        user_options = _build.build.user_options + custom_user_options

    class sdist(_sdist.sdist):

        def run(self):
            generate_pykeyvi_source()
            _sdist.sdist.run(self)

    class bdist(custom_opts, _bdist.bdist):
        parent = _bdist.bdist
        user_options = _bdist.bdist.user_options + custom_user_options

    have_wheel = False
    try:
        import wheel.bdist_wheel as _bdist_wheel

        class bdist_wheel(custom_opts, _bdist_wheel.bdist_wheel):
            parent = _bdist_wheel.bdist_wheel
            user_options = _bdist_wheel.bdist_wheel.user_options + custom_user_options

        have_wheel = True
    except: None

    class build_py(_build_py.build_py):

        def run(self):
            generate_pykeyvi_source()

            if sys.platform == 'darwin':
                if not os.path.exists(mac_os_static_libs_dir):
                    os.makedirs(mac_os_static_libs_dir)

                for lib in linklibraries_static_or_dynamic:
                    lib_file_name = 'lib{}.a'.format(lib)
                    src_file = path.join('/usr/local/lib', lib_file_name)
                    dst_file = path.join(mac_os_static_libs_dir, lib_file_name)
                    shutil.copyfile(src_file, dst_file)

            CMAKE_CXX_FLAGS = '-fPIC -std=c++11'
            if sys.platform == 'darwin':
                CMAKE_CXX_FLAGS += ' -mmacosx-version-min=10.9'

            keyvi_build_cmd = 'mkdir -p {}'.format(keyvi_build_dir)
            keyvi_build_cmd += ' && cd {}'.format(keyvi_build_dir)
            keyvi_build_cmd += ' && cmake -D CMAKE_BUILD_TYPE:STRING=python ' \
                                ' -D CMAKE_CXX_FLAGS="{CXX_FLAGS}"' \
                                ' -D CMAKE_INSTALL_PREFIX={INSTALL_PREFIX}'.format(
                CXX_FLAGS=CMAKE_CXX_FLAGS, INSTALL_PREFIX=keyvi_install_prefix)
            if zlib_root is not None:
                 keyvi_build_cmd += ' -D ZLIB_ROOT={ZLIB_ROOT}'.format(ZLIB_ROOT=zlib_root)
            keyvi_build_cmd += ' ..'
            keyvi_build_cmd += ' && make -j {}'.format(cpu_count)
            keyvi_build_cmd += ' && make install'

            print ("Building keyvi C++ part: " + keyvi_build_cmd)
            subprocess.call(keyvi_build_cmd, shell=True)

            # patch keyvimerger into the package
            # note: package_data does not work for this as it would break sdist
            build_dir = os.path.join(*([self.build_lib] + ['keyvi', '_bin']))
            self.data_files.append(("keyvi._bin", keyvi_install_dir + "/bin", build_dir, ['keyvimerger']))

            _build_py.build_py.run(self)

    class install_data(_install_data.install_data):

        def run(self):
            _install_data.install_data.run(self)
            for fn in self.get_outputs():
                if fn.endswith("keyvimerger"):
                    # make it  executable
                    mode = ((os.stat(fn).st_mode) | 0o555) & 0o7777
                    os.chmod(fn, mode)

    class build_ext(_build_ext.build_ext):

        def run(self):
            os.environ['ARCHFLAGS'] = '-arch x86_64'
            _build_ext.build_ext.run(self)

    ext_modules = [Extension('keyvi._core',
                             include_dirs=[autowrap_data_dir,
                                           path.join(dictionary_sources, '3rdparty/tpie'),
                                           path.join(os.path.join(keyvi_build_dir, 'keyvi/3rdparty/tpie')),
                                           path.join(dictionary_sources, 'include/keyvi'),
                                           path.join(dictionary_sources, '3rdparty/rapidjson/include'),
                                           path.join(dictionary_sources, '3rdparty/msgpack-c/include'),
                                           path.join(dictionary_sources, '3rdparty/tiny-process-library'),
                                           path.join(dictionary_sources, '3rdparty/utf8'),
                                           path.join(dictionary_sources, '3rdparty/misc'),
                                           path.join(dictionary_sources, '3rdparty/xchange/src')],
                             language='c++',
                             sources=[pykeyvi_cpp],
                             extra_compile_args=['-std=c++11', '-msse4.2'] + additional_compile_flags,
                             extra_link_args=extra_link_arguments,
                             library_dirs=link_library_dirs,
                             libraries=linklibraries)]

    PACKAGE_NAME = 'keyvi'

    version = '0.2.6'

    install_requires = [
        'msgpack-python',
    ]

    commands = {'build_py': build_py, 'build_ext': build_ext, 'sdist': sdist, 'build': build, 'bdist': bdist, 'install_data': install_data}
    if have_wheel:
        commands['bdist_wheel'] = bdist_wheel

    setup(
        name=PACKAGE_NAME,
        version=version,
        description='Python package for keyvi',
        author='Hendrik Muhs',
        author_email='hendrik.muhs@gmail.com',
        license="ASL 2.0",
        cmdclass=commands,
        scripts=['src/py/bin/keyvi'],
        packages=['keyvi',
                  'keyvi.cli',
                  'keyvi.compiler',
                  'keyvi.completion',
                  'keyvi.dictionary',
                  'keyvi.index',
                  'keyvi.util',
                  'keyvi._pycore'],
        package_dir={'': 'src/py'},
        ext_modules=ext_modules,
        zip_safe=False,
        url='http://keyvi.org',
        download_url='https://github.com/KeyviDev/keyvi/tarball/v{}'.format(version),
        keywords=['FST'],
        classifiers=[
            'Programming Language :: C++',
            'Programming Language :: Cython',
            'Programming Language :: Python',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3.4',
            'Programming Language :: Python :: 3.5',
            'Programming Language :: Python :: Implementation :: CPython',
            'Programming Language :: Python :: Implementation :: PyPy',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: Unix',
        ],
        install_requires=install_requires,
    )
