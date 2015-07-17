# keyvi

A key value index based on finite state technology:

Data structure: 
Sparse Array (See Storing a Sparse Table, Robert E. Tarjan et al. http://infolab.stanford.edu/pub/cstr/reports/cs/tr/78/683/CS-TR-78-683.pdf)

Construction: 
Incremental, which means minimization is done on the fly (See Incremental Construction of Minimal Acyclic Finite-State Automata, J. Daciuk et al.: http://www.mitpressjournals.org/doi/pdf/10.1162/089120100561601)
Plus a lot of unpublished tweaks

## Usage

  * Howtos
    * [Compiling Dictionaries/Indexes](/doc/usage/Building keyvi dictionaries.md)
    * Pykeyvi
      * [Compiling](/doc/usage/Building keyvi dictionaries with python.md)
  * [Crashcourse](/doc/usage/Crashcourse.md)
  
## Development
 
  * [Build/Packaging](/doc/development/Packaging keyvi.md)
  
## Internals
  
  * [Construction Basics](/doc/algorithm/Construction-Basics.md)
  * [Persistence Basics](/doc/algorithm/Persistence-Basics.md)
  * [Minimization](/doc/algorithm/Minimization.md)
  * [Scaling](/doc/algorithm/Scaling.md)
  * [Extensibility](/doc/algorithm/Extensibility.md)
  