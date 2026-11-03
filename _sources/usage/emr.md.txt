# Using keyvi in pyspark or mrjob

Imagine you want to do some lookups in your map-reduce jobs like matching or extracting certain patterns - in fuzzy manner if you like - and your lookup table is huge.
You could call a database or key value store remotely, but that kills performance and introduces a single point of failure in the otherwise distributed processing.
With keyvi you can easily do that, given you are using a python based framework like mrjob or pyspark.
Due to the shared memory model keyvi indexes are just loaded once in memory, independent on the number of mappers on one machine.
It works even if keyvi files are bigger than the main memory, for the price of some IO of course.
In the end all calls will be local and do not entail any networking.

## Bootstrapping keyvi

All we need is to install keyvi at bootstraping time.

Precompiled wheels are available for Linux, therefore just install the python version of keyvi during bootstrap:

```bash
sudo pip install keyvi
```

## Using keyvi

The rest is simple, get your keyvi files on the machines. You can download them during bootstrap from S3 or more convenient in pyspark, put them on hdfs:

```python
sc = SparkContext.getOrCreate()
sc.addFile(s3_filename)
```

Ensure you initialize the Dictionary e.g. in mapper_init (mrjob) or using a singleton pattern (pyspark) and not for every mapper call.

See this example loader for pyspark:

```python
from keyvi.dictionary import Dictionary

try:
    # only works if in spark
    from pyspark import SparkFiles
    from pyspark import SparkContext

    class Spark_KeyviLoader:
        # this is a singleton
        _instance = None

        def __init__(self):
            globals()[self.__class__.__name__] = self
            self.loaded_keyvi_dicts = {}

        def __call__(self, *args, **kwargs):
            return self

        def get(self, name):
            d = self.loaded_keyvi_dicts.get(name)

            if d is None:
                hdfs_filename = SparkFiles.get(name)
                if hdfs_filename is None:
                    raise Exception(
                        "Could not find dictionary with name {} in HDFS, did you load it?".format(
                            name
                        )
                    )

                try:
                    d = Dictionary(hdfs_filename)
                except Exception as e:
                    raise Exception(
                        "Failed to load keyvi dictionary {}: {}".format(name, e)
                    ) from e

                self.loaded_keyvi_dicts[name] = d

            return d

        @staticmethod
        def load_into_spark(s3_filename):
            sc = SparkContext.getOrCreate()
            sc.addFile(s3_filename)

    KeyviLoader = Spark_KeyviLoader

except ImportError:
    # fallback for standalone use and testing
    import os

    class Local_KeyviLoader:
        # this is a singleton
        _instance = None

        def __init__(self):
            globals()[self.__class__.__name__] = self
            self.loaded_keyvi_dicts = {}

        def __call__(self, *args, **kwargs):
            return self

        def get(self, name):
            d = self.loaded_keyvi_dicts.get(name)

            if d is None:
                d = Dictionary(os.path.join("my_keyvi_files_folder", name))
                self.loaded_keyvi_dicts[name] = d

            return d

    KeyviLoader = Local_KeyviLoader


def get_keyvi_dictionary(name):
    return KeyviLoader().get(name)
```
