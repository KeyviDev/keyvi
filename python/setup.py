from setuptools import setup, Extension
import distutils.command.build as _build
import distutils.command.bdist as _bdist
import distutils.command.build_ext as _build_ext
import distutils.command.sdist as _sdist
import os
import sys
import subprocess
import multiprocessing
import shutil
import glob
import platform

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

#################

VERSION_MAJOR = 0
VERSION_MINOR = 3
VERSION_PATCH = 3
VERSION_DEV = 0
IS_RELEASED = False

VERSION = "{}.{}.{}".format(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
if not IS_RELEASED:
    VERSION += '.dev{}'.format(VERSION_DEV)


###################


def run_once(f):
    def wrapper(*args, **kwargs):
        if not wrapper.has_run:
            wrapper.has_run = True
            return f(*args, **kwargs)
    wrapper.has_run = False
    return wrapper


def write_version_file():
    here = os.path.abspath(os.path.dirname(__file__))
    version_file_path = os.path.join(here, 'src/py/keyvi/_version.py')
    content = """
# THIS FILE IS GENERATED FROM KEYVI SETUP.PY

__version__ = '{}'

""".format(VERSION)

    with open(version_file_path, 'w') as f_out:
        f_out.write(content)


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
        try:
            if not path.exists(keyvi_cpp):
                os.makedirs(keyvi_cpp)
            os.symlink(path.abspath(keyvi_cpp_source), keyvi_cpp_link)
            shutil.copy('../CMakeLists.txt', path.join(keyvi_cpp, 'CMakeLists.txt'))
            keyvi_source_path = os.path.realpath(os.path.join(os.getcwd(), keyvi_cpp_source))
            pykeyvi_source_path = os.path.join(os.getcwd(), keyvi_cpp_link)
            yield (pykeyvi_source_path, keyvi_source_path)
        finally:
            os.unlink(keyvi_cpp_link)
            os.remove(path.join(keyvi_cpp, 'CMakeLists.txt'))
    else:
        yield None, None

@run_once
def cmake_configure(build_path, build_type, zlib_root, additional_compile_flags):
    # needed for shared library
    CMAKE_CXX_FLAGS = additional_compile_flags + ' -fPIC'

    # TODO: still needed?
    # workaround for https://bitbucket.org/pypy/pypy/issues/2626/invalid-conversion-from-const-char-to-char
    if os.environ.get('PYTHON_VERSION', '') == 'pypy2':
        CMAKE_CXX_FLAGS += ' -fpermissive'

    cmake_configure_cmd = 'mkdir -p {}'.format(build_path)
    cmake_configure_cmd += ' && cd {}'.format(build_path)
    cmake_configure_cmd += ' && cmake' \
                        ' -D CMAKE_CXX_FLAGS="{CXX_FLAGS}"'.format(CXX_FLAGS=CMAKE_CXX_FLAGS.strip())
    cmake_configure_cmd +=  ' -D CMAKE_BUILD_TYPE={BUILD_TYPE}'.format(BUILD_TYPE=build_type)

    if zlib_root is not None:
         cmake_configure_cmd += ' -D ZLIB_ROOT={ZLIB_ROOT}'.format(ZLIB_ROOT=zlib_root)
    cmake_configure_cmd += ' ..'

    print ("Building in {0} mode".format(build_type))
    print ("Run keyvi C++ cmake: " + cmake_configure_cmd)
    subprocess.call(cmake_configure_cmd, shell=True)

    cmake_flags = {}
    with open(os.path.join(build_path, "keyvi", "flags")) as flags:
        for line in flags:
            k, v = line.strip().split("=", 1)
            cmake_flags[k] = v.strip()

    # set additional compiler flags
    set_additional_flags('extra_compile_args', cmake_flags['KEYVI_CXX_FLAGS_ALL'].split(' '))

    # set defines
    define_macros = []
    for macro in cmake_flags['KEYVI_COMPILE_DEFINITIONS'].split(' '):
        if macro.count("=") == 0:
            define_macros.append((macro, None))
        else:
            define_macros.append(macro.split("=", 1))
    set_additional_flags('define_macros', define_macros)

    # set link libraries
    set_additional_flags('libraries', cmake_flags['KEYVI_LINK_LIBRARIES_ALL'].split(' '))

    return cmake_flags


def set_additional_flags(key, additional_flags):
     # patch the compile flags
    for ext_m in ext_modules:
        flags = getattr(ext_m, key) + additional_flags
        setattr(ext_m, key, flags)


with symlink_keyvi() as (pykeyvi_source_path, keyvi_source_path):
    # workaround for autowrap bug (includes incompatible boost)
    autowrap_data_dir = "autowrap_includes"

    dictionary_sources = path.abspath(keyvi_cpp_link)
    keyvi_build_dir = path.join(keyvi_cpp, 'build-{}'.format(platform.platform()))

    additional_compile_flags = ''

    # re-map the source files in the debug symbol tables to there original location so that stepping in a debugger works
    if pykeyvi_source_path is not None:
        additional_compile_flags += ' -fdebug-prefix-map={}={}'.format(pykeyvi_source_path, keyvi_source_path)

    mac_os_static_libs_dir = 'mac_os_static_libs'

    extra_link_arguments = []
    link_library_dirs = [keyvi_build_dir]
    zlib_root = None

    if sys.platform == 'darwin':
        link_library_dirs.append(mac_os_static_libs_dir)
        extra_link_arguments.append('-L{}'.format(mac_os_static_libs_dir))

    #########################
    # Custom 'build' command
    #########################

    custom_user_options = [('mode=',
                            None,
                            "build mode."),
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
            global ext_modules
            global zlib_root
            global build_type

            build_type = self.mode

            cmake_flags = cmake_configure(keyvi_build_dir, build_type, zlib_root, additional_compile_flags)

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

    class build_ext(custom_opts, _build_ext.build_ext):

        class build_cxx:

            def initialize_options(self):
                _build_ext.build_ext.initialize_options(self)

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

                keyvi_build_cmd = 'cd {} && make -j {} bindings'.format(keyvi_build_dir, cpu_count)

                print ("Building keyvi C++ part: " + keyvi_build_cmd)
                subprocess.call(keyvi_build_cmd, shell=True)

                os.environ['ARCHFLAGS'] = '-arch x86_64'
                _build_ext.build_ext.run(self)

        parent = build_cxx
        user_options = _build_ext.build_ext.user_options + custom_user_options


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
                             extra_link_args=extra_link_arguments,
                             library_dirs=link_library_dirs)]

    PACKAGE_NAME = 'keyvi'

    install_requires = [
        'msgpack-python>=0.5.6',
    ]

    commands = {'build_ext': build_ext, 'sdist': sdist, 'build': build, 'bdist': bdist}
    if have_wheel:
        commands['bdist_wheel'] = bdist_wheel

    write_version_file()
    setup(
        name=PACKAGE_NAME,
        version=VERSION,
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
                  'keyvi.vector',
                  'keyvi._pycore'],
        package_dir={'': 'src/py'},
        ext_modules=ext_modules,
        zip_safe=False,
        url='http://keyvi.org',
        download_url='https://github.com/KeyviDev/keyvi/tarball/v{}'.format(VERSION),
        keywords=['FST'],
        classifiers=[
            'Programming Language :: C++',
            'Programming Language :: Cython',
            'Programming Language :: Python',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3.4',
            'Programming Language :: Python :: 3.5',
            'Programming Language :: Python :: 3.6',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: Implementation :: CPython',
            'Programming Language :: Python :: Implementation :: PyPy',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: Unix',
        ],
        install_requires=install_requires,
    )
