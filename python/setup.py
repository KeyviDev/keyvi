from distutils.dir_util import remove_tree
from setuptools import setup, Extension
import distutils.command.build as _build
import distutils.command.bdist as _bdist
import distutils.command.build_ext as _build_ext
import distutils.command.clean as _clean
import distutils.command.sdist as _sdist
import json
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
pykeyvi_p_cpp = '_core_p.cpp'
keyvi_cpp_source = '../keyvi'
keyvi_cpp = 'src/cpp'
keyvi_cpp_link = path.join(keyvi_cpp, 'keyvi')
keyvi_build_dir = path.join(keyvi_cpp, 'build-{}'.format(platform.platform()))
here = os.path.abspath(os.path.dirname(__file__))

try:
    cpu_count = multiprocessing.cpu_count()
except:
    cpu_count = 1

#################

VERSION_MAJOR = 0
VERSION_MINOR = 5
VERSION_PATCH = 7
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
            wrapper.ret = f(*args, **kwargs)
        return wrapper.ret
    wrapper.has_run = False
    wrapper.ret = None
    return wrapper


def write_version_file():
    version_file_path = os.path.join(here, 'src/py/keyvi/_version.py')
    content = """
# THIS FILE IS GENERATED FROM KEYVI SETUP.PY

__version__ = '{}'

""".format(VERSION)

    with open(version_file_path, 'w') as f_out:
        f_out.write(content)

def clean_pykeyvi_build_directory():
    if os.path.exists(keyvi_build_dir):
        remove_tree(keyvi_build_dir)

def generate_pykeyvi_source():
    addons = glob.glob('src/addons/*')
    pxds = glob.glob('src/pxds/*')
    converters = 'src/converters'
    converter_files = glob.glob(path.join(converters, '*'))
    max_modification_time = max([path.getmtime(fn) for fn in addons + pxds + converter_files])

    if not path.exists(pykeyvi_cpp) or max_modification_time > path.getmtime(pykeyvi_cpp):
        try:
            import autowrap.Main
            autowrap.Main.run(pxds, addons, [converters], pykeyvi_pyx)
            # rewrite generated cpp to use std::shared_ptr instead of boost::shared_ptr
            with open(pykeyvi_cpp, "rt") as fin:
                with open(pykeyvi_p_cpp, "wt") as fout:
                    for line in fin:
                        if line.find("shared_ptr.hpp") > 0:
                            continue
                        fout.write(line.replace('boost::shared_ptr', 'std::shared_ptr'))

        except:
            if not path.exists(pykeyvi_cpp):
                raise
            else:
                print ("Could not find autowrap, probably running from sdist environment")

@contextmanager
def symlink_keyvi():
    if not path.exists(keyvi_cpp_link):
        try:
            if not path.exists(keyvi_cpp):
                os.makedirs(keyvi_cpp)
            os.symlink(path.abspath(keyvi_cpp_source), keyvi_cpp_link)
            shutil.copy('../CMakeLists.txt', path.join(keyvi_cpp, 'CMakeLists.txt'))
            shutil.copytree('../cmake_modules', path.join(keyvi_cpp, 'cmake_modules'))
            keyvi_source_path = os.path.realpath(os.path.join(os.getcwd(), keyvi_cpp_source))
            pykeyvi_source_path = os.path.join(os.getcwd(), keyvi_cpp_link)
            yield (pykeyvi_source_path, keyvi_source_path)
        finally:
            os.unlink(keyvi_cpp_link)
            os.remove(path.join(keyvi_cpp, 'CMakeLists.txt'))
            shutil.rmtree(path.join(keyvi_cpp, 'cmake_modules'))
    else:
        yield None, None

@run_once
def cmake_configure(build_path, build_type, zlib_root, additional_compile_flags):
    # needed for shared library
    CMAKE_CXX_FLAGS = additional_compile_flags + ' -fPIC'

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
            cmake_flags[k] = " ".join(v.split())

    # set additional compiler flags
    set_additional_flags('extra_compile_args', cmake_flags['KEYVI_CXX_FLAGS_ALL'].split(' '))

    # set defines
    if cmake_flags['KEYVI_COMPILE_DEFINITIONS']:
        define_macros = []

        for macro in cmake_flags['KEYVI_COMPILE_DEFINITIONS'].split(' '):
            if macro.count("=") == 0:
                define_macros.append((macro, None))
            else:
                define_macros.append(macro.split("=", 1))

        set_additional_flags('define_macros', define_macros)

    # set includes
    if cmake_flags['KEYVI_INCLUDES']:
        set_additional_flags('include_dirs', cmake_flags['KEYVI_INCLUDES'].split(' '))

    # set link libraries
    if cmake_flags['KEYVI_LINK_LIBRARIES_STATIC']:
        if sys.platform == 'darwin':
            set_additional_flags('libraries', cmake_flags['KEYVI_LINK_LIBRARIES_STATIC'].split(' '))

        else:
            extra_link_arguments = ['-Wl,-Bstatic']
            for lib in cmake_flags['KEYVI_LINK_LIBRARIES_STATIC'].split(' '):
                extra_link_arguments.append("-l{}".format(lib))

            # reset to dynamic
            extra_link_arguments.append('-Wl,-Bdynamic')
            set_additional_flags('extra_link_args', extra_link_arguments)

    if cmake_flags['KEYVI_LINK_LIBRARIES_DYNAMIC']:
        set_additional_flags('libraries', cmake_flags['KEYVI_LINK_LIBRARIES_DYNAMIC'].split(' '))

    # set link args
    if cmake_flags['KEYVI_LINK_FLAGS']:
        set_additional_flags('extra_link_args', cmake_flags['KEYVI_LINK_FLAGS'].split(' '))

    return cmake_flags


def set_additional_flags(key, additional_flags):
     # patch the flags specified in key
    for ext_m in ext_modules:
        flags = getattr(ext_m, key) + additional_flags
        setattr(ext_m, key, flags)


def patch_for_custom_zlib(zlib_root):
    for ext_m in ext_modules:
        include_dirs = [path.join(zlib_root, "include")] + getattr(ext_m, 'include_dirs')
        setattr(ext_m, 'include_dirs', include_dirs)
        library_dirs = [path.join(zlib_root, "lib")] + getattr(ext_m, 'library_dirs')
        setattr(ext_m, 'library_dirs', library_dirs)


with symlink_keyvi() as (pykeyvi_source_path, keyvi_source_path):
    # workaround for autowrap bug (includes incompatible boost)
    autowrap_data_dir = "autowrap_includes"

    dictionary_sources = path.abspath(keyvi_cpp_link)

    additional_compile_flags = ''

    # re-map the source files in the debug symbol tables to there original location so that stepping in a debugger works
    if pykeyvi_source_path is not None:
        additional_compile_flags += ' -fdebug-prefix-map={}={}'.format(pykeyvi_source_path, keyvi_source_path)

    link_library_dirs = [
        keyvi_build_dir,
        '/usr/local/lib/',  # as of 17/07/2022 Python 3.10 build on GH actions needs '/usr/local/lib/' link library dir
    ]

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
            self.mode = None
            self.staticlinkboost = False
            self.zlib_root = None
            self.options = {}

        def load_options(self):
            # preserves setting between build and install
            if not self.mode and not self.zlib_root:
                try:
                    f = open(path.join(keyvi_build_dir, "custom_opts"), "r")
                    self.options = json.loads(f.readline())
                    return
                except: pass
            self.options['mode'] = "release" if not self.mode else self.mode
            if self.zlib_root:
                self.options['zlib_root'] = self.zlib_root

        def save_options(self):
            # store the options
            f = open(path.join(keyvi_build_dir, "custom_opts"), "w")
            f.write(json.dumps(self.options))

        def run(self):
            self.load_options()
            self.cmake_flags = cmake_configure(keyvi_build_dir, self.options['mode'], self.options.get('zlib_root'), additional_compile_flags)
            self.save_options()
            self.parent.run(self)

    class build(custom_opts, _build.build):
        parent = _build.build
        user_options = _build.build.user_options + custom_user_options

    class sdist(_sdist.sdist):

        def run(self):
            clean_pykeyvi_build_directory()
            generate_pykeyvi_source()
            _sdist.sdist.run(self)

    class bdist(custom_opts, _bdist.bdist):
        parent = _bdist.bdist
        user_options = _bdist.bdist.user_options + custom_user_options

    class clean(_clean.clean):

        def run(self):
            clean_pykeyvi_build_directory()
            _clean.clean.run(self)

    have_wheel = False
    try:
        import wheel.bdist_wheel as _bdist_wheel

        class bdist_wheel(custom_opts, _bdist_wheel.bdist_wheel):
            parent = _bdist_wheel.bdist_wheel
            user_options = _bdist_wheel.bdist_wheel.user_options + custom_user_options

        have_wheel = True
    except: None

    class build_cxx(_build_ext.build_ext):

        def initialize_options(self):
            _build_ext.build_ext.initialize_options(self)

        def run(self):
            generate_pykeyvi_source()

            # custom zlib location
            if 'zlib_root' in self.options:
                patch_for_custom_zlib(self.options['zlib_root'])

            keyvi_build_cmd = 'cd {} && make -j {} bindings'.format(keyvi_build_dir, cpu_count)

            print ("Building keyvi C++ part: " + keyvi_build_cmd)
            subprocess.call(keyvi_build_cmd, shell=True)

            _build_ext.build_ext.run(self)

    class build_ext(custom_opts, build_cxx):

        parent = build_cxx
        user_options = build_cxx.user_options + custom_user_options

    ext_modules = [Extension('keyvi._core',
                             include_dirs=[autowrap_data_dir],
                             language='c++',
                             sources=[pykeyvi_p_cpp],
                             library_dirs=link_library_dirs)]

    PACKAGE_NAME = 'keyvi'
    with open(os.path.join(here, 'description.md'), "rt", encoding="utf-8") as desc_f:
        long_desc = desc_f.read()

    install_requires = [
        'msgpack>=1.0.0',
    ]

    commands = {'build_ext': build_ext, 'sdist': sdist, 'build': build, 'bdist': bdist, 'clean': clean}
    if have_wheel:
        commands['bdist_wheel'] = bdist_wheel
    for e in ext_modules:
        e.cython_directives = {"embedsignature": True}

    write_version_file()
    setup(
        name=PACKAGE_NAME,
        version=VERSION,
        description='Python package for keyvi',
        long_description=long_desc,
        long_description_content_type="text/markdown",
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
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: 3.8',
            'Programming Language :: Python :: 3.9',
            'Programming Language :: Python :: 3.10',
            'Programming Language :: Python :: Implementation :: CPython',
            'Programming Language :: Python :: Implementation :: PyPy',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: Unix',
        ],
        install_requires=install_requires,
    )
