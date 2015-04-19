Having done a C++ version of the (Make-A-Lisp)[kanaka/mal] project, I wanted to try building a simpler version of the language, with the following goals:
* Minimise native code and data structures. Where possible, use the hosted code and data structures.
* Stop-and-copy garbage collection. My C++ implementation used reference-counted pointers. Self-referential loops are unavoidable.

The language is based off SICP Scheme, but makes no attempts at completeness.
