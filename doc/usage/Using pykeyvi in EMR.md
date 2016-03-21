## Using keyvi in pyspark or mrjob


Imagine you want to do some lookups in you map-reduce jobs like matching or extracting certain patterns - in fuzzy manner if you like - and your lookup table is huge. 
You could call a database or key value store remotely, but that kills performance and introduces a single point of failure in the otherwise distributed processing.
With pykeyvi you can easily do that, given you are using a python based framework like mrjob or pyspark. 
Due to the shared memory model keyvi indexes are just loaded once in memory, independent on the number of mappers on one machine.
It works even if keyvi files are bigger than the main memory, for the price of some IO of course. 
In the end all calls will be local and do not entail any networking.

### Bootstrapping keyvi

All we need is to install keyvi at bootstraping time. 
You need a package that can be installed on the OS you are using. 
For Amazons EMR '4.3.0' - the latest at time of writing - a package is available for download:

    sudo yum install -y snappy snappy-devel
    sudo pip install python-snappy
    sudo pip-2.7 install msgpack-python
    wget https://github.com/cliqz-oss/keyvi-package/blob/master/bin/amazon-linux-2015.09/python-keyvi-0.1.16-1.x86_64.rpm?raw=true -O python-keyvi.rpm
    sudo rpm -i python-keyvi.rpm
    
Note: mspack-python and python-snappy are dependencies which we need to install manually in this case.

### Using keyvi

The rest is simply, get your keyvi files on the machines, e.g. download them during bootstrap from S3 or at runtime in your init function.

Ensure you initialize the Dictionary e.g. in mapper_init (mrjob) or using a singleton pattern (pyspark) and not for every mapper call.
