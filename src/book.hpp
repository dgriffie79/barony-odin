/*-------------------------------------------------------------------------------

	BARONY
	File: book.hpp
	Desc: declarations and such for readable books

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include "main.hpp"

class Book_t
{
public:
	Book_t() {};
	DynamicString text = "";
	DynamicString default_name = "";
	std::vector<DynamicString> formattedPages;
};
extern std::vector<Book_t> allBooks;

class BookParser_t
{
	static const int MAX_FILE_LENGTH = 192000;
	const int versionJSON = 1;
public:
	bool booksRequireCompiling();
	bool readCompiledBooks();
	void writeCompiledBooks();
	void createBooks(bool forceCacheRebuild);
	void createBook(DynamicString filename); //Take a book filename and generate all of its pages.
	void deleteBooks();
	DynamicMapStr tempBookData;
	void readBooksIntoTemp();
	std::list<std::string> getListOfBooksAfterFiltering();
};
extern BookParser_t bookParser_t;

int getBook(DynamicString bookTitle);
DynamicString getBookDefaultNameFromIndex(int index, bool censored = true);
DynamicString getBookLocalizedNameFromIndex(int index, bool censored = true);
extern int numbooks;

static const char* banned_books[] = {
	"The Lusty Goblin Maid"
};
static constexpr int num_banned_books =
	sizeof(banned_books) / sizeof(banned_books[0]);