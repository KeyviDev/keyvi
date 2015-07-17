Coding guidelines
=================

Purpose of this document:

* To provide guidelines for how to write C++ code that is supposed to become
  part of the TPIE project.

How to modify this document:

* Contact the person who is responsible for the respective rule. 
* If the change is approved, substitute your initals for the initials of the
  previous author.
* If necessary, update the list of maintainers to include your name, initials,
  and e-mail address.
* Commit the changes (or have some member of the TPIE project commit them).

Maintainers: 
 
* Thomas Molhave (tm, thomasm@daimi.au.dk)
* Jan Vahrenhold (jv, jan.vahrenhold@cs.tu-dortmund.de)
* Henrik Blunck (hb, blunck@madalgo.au.dk)


Documentation
-------------

1.  Code has to be documented using [Doxygen](www.doxygen.org). (jv)

2.  Comments have to be given at least for all public and protected methods and
    attributes. (jv) 
    Additionally, Doxygen compatible comments for private members as well as
    for functions, enumeration types and header files are encouraged. (hb)

3.  Old C-style comments (`/*  */`) shall not be used. (jv)

4.  Comments for classes, methods and functions shall be written using the
    "three-slash" style followed and preceeded by a line of slashes. (hb)

    Example:

    <pre>
    /////////////////////////////////////////////////////////
    ///		
    ///  This class serves as an example.
    ///
    /////////////////////////////////////////////////////////
    </pre>

5.  Javadoc-style comments may be used for commenting attributes,
    enumerations and typedefs. (jv)

    Example:

    <pre>
    /**  This is a sample attribute.  */
    int attribute;
    </pre>


6.  An empty expression, e.g., an empty destructor should be commented using
    `// Do nothing.` (jv)


Naming Convention
-----------------

1.  Class names shall be all-lowercase using an underscore to separate parts.
    (tm)

    Example:
    <pre>
    class foo_bar_base { ... }
    </pre>

2.  Method names shall be all-lowercase using an underscore to separate parts.
    (tm)

    Example:
    <pre>
    int foo_bar();
    </pre>

3.  Names of attributes, parameters, and variables shall be starting with a
    lowercase letter and use uppercase letters to seperate parts. (jv)

    Example:
    <pre>
    int myLocalVariable;
    </pre>

4.  Instance and class attributes shall be prefixed by `m_`. (jv)

    Example:
    <pre>
    short m_anAttribute;
    </pre>

5.  The use of namespacing shall be enforced. The namespace for the
    TPIE-project is `tpie::`. (tm)

6.  Do not import symbols into the global namespace (this prohibits many uses
    of the `using` keyword). (tm)

Writing Efficient Code
----------------------

1.  Use `const` modifiers for methods and parameters wherever possible. (tm)

Class Design and Layout
-----------------------

1.  The components of a class shall appear in the following order: public -
    protected - private (jv)

2.  Bodies longer than 6 lines of code shall be put in an inline file (.inl)
    which is included after the class declaration. (tm)

3.  The "at-most-one-class-per-file" rule shall be enforced. (jv)

4.  Attributes shall not be public. Every attribute shall be accompanied by a
    `put`- and `get`-method. The name of such a method is the name of the
    attribute prefixed by `put_` or `get_` (using the naming convention, parts
    of the attribute name are sepearated by underscores).  Read-only attributes
    shall be accessed by a method that is named just like the attribute. (jv)

    Example:

    <pre>
    int get_my_attribute() const;
    void put_my_attribute(int myAttribute);

    int my_readonly_attribute() const;
    </pre>
