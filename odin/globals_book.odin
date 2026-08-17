// book.odin — Odin-owned globals of book.hpp (single owner; C++ references via extern "C").
package main

// DynamicArrayT<Book_t> allBooks;  -> [dynamic]Book_t (40B mirror of DynamicArray)
@(export)
allBooks : [dynamic]Book_t

// BookParser_t bookParser_t;
@(export)
bookParser_t : BookParser_t

// int numbooks = 0;
@(export)
numbooks : i32 = 0
