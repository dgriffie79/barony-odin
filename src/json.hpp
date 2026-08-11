/**
	A simple interface for reading and writing objects to files, has support for both json and binary.
	Basic types, enums and vectors are supported by default, other class and struct types need to
	implement the "bool serialize(FileInterface * file)" function.
	The interface is symmetric, meaning that there is only a single function for both saving and loading.

	class ExampleClass {
	private:
		Uint32 MyNumber;
		String MyString;
	public:
		bool serialize(FileInterface * file) {
			file->property("MyNumber", MyNumber);
			file->property("MyString", MyString);
			return true; // you may return false if any of the property() functions failed
		}
	};
*/

#pragma once

#include <functional>
#include <string>

// DynamicString ({data,len}, NUL-terminated) — the de-STL string that will
// replace std::string in shared structs. Mirrors Odin's string layout.
#include "../odin/containers/dynamic_string.hpp"

class File;

enum class EFileFormat {
	Json,
	Binary,
	Json_Compact
};

// Opaque per-format serialization state, owned by FileInterface and
// defined in json.cpp (keeps rapidjson types out of this header).
struct JsonWriterState;
struct JsonReaderState;

class FileInterface {
public:
	FileInterface() = default;
	~FileInterface();
	FileInterface(const FileInterface&) = delete;
	FileInterface& operator=(const FileInterface&) = delete;
	FileInterface(FileInterface&& other) noexcept;
	FileInterface& operator=(FileInterface&& other) noexcept;

	// Factory functions. Replace the former per-format constructors and
	// are the only way to obtain a FileInterface.
	// @param file the open file to serialize to/from
	// @param format the format to use (Json, Json_Compact, or Binary)
	// @return a FileInterface bound to the given file and format
	static FileInterface makeWriter(File* file, EFileFormat format);
	static FileInterface makeReader(File* file);

	// @return true if this interface is reading data from a file, false if it is writing
	bool isReading() const { return reading; }

	// Flush any buffered output (JSON writer) to the underlying file.
	// No-op for binary and reader interfaces.
	void flushToFile();

	// Signals the beginning of an object in the file
	bool beginObject();
	// Signals the end of an object in the file
	void endObject();

	// Signals the beginning of an array in the file
	// @param size number of items in the array
	bool beginArray(Uint32& size);
	// Signals the end of an array in the file
	void endArray();

	// Serializes the name of a property
	// @param name name of the property
	void propertyName(const char* name);

	// @param v the value to serialize
	bool value(Uint32& v);
	// @param v the value to serialize
	bool value(Sint32& v);
	// @param v the value to serialize
	bool value(float& v);
	// @param v the value to serialize
	bool value(double& v);
	// @param v the value to serialize
	bool value(bool& v);
	// @param v the value to serialize
	bool value(std::string& v);
	// @param v the value to serialize (DynamicString — de-STL string)
	bool value(DynamicString& v);

	// Serialize a vector with a max length
	// @param v the value to serialize
	// @param maxLength maximum number of items, 0 is no limit
	template<typename T, typename... Args>
	bool value(std::vector<T>& v, Uint32 maxLength = 0, Args ... args) {
		Uint32 size = (Uint32)v.size();
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    v.resize(size);
		    bool result = true;
		    for (Uint32 index = 0; index < size; ++index) {
			    result = value(v[index], args...) ? result : false;
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

		// Serialize a DynamicArrayS32 (std::vector<int> replacement)
	bool value(DynamicArrayS32& v, Uint32 maxLength = 0) {
		Uint32 size = (Uint32)v.size();
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    v.clear();
		    bool result = true;
		    for (Uint32 index = 0; index < size; ++index) {
			    int32_t elem = 0;
			    result = value(elem) ? result : false;
			    v.push_back(elem);
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}


	// Serialize a generic DynamicArrayT<T> (owning elements serialize via value())
	template <typename T>
	bool value(DynamicArrayT<T>& v, Uint32 maxLength = 0) {
		Uint32 size = (Uint32)v.size();
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    v.clear();
		    bool result = true;
		    for (Uint32 index = 0; index < size; ++index) {
			    T elem{};
			    result = value(elem) ? result : false;
			    v.push_back(elem);
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

	// Serialize a DynamicArrayU32 (std::vector<Uint32> replacement)
	bool value(DynamicArrayU32& v, Uint32 maxLength = 0) {
		Uint32 size = (Uint32)v.size();
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    v.clear();
		    bool result = true;
		    for (Uint32 index = 0; index < size; ++index) {
			    uint32_t elem = 0;
			    result = value(elem) ? result : false;
			    v.push_back(elem);
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

// Serialize a pair
	// @param v the pair to serialize
	template<typename T1, typename T2>
	bool value(std::pair<T1, T2>& v) {
	    bool result = false;
	    if (beginObject()) {
	        result = true;
	        result = property("first", v.first) ? result : false;
	        result = property("second", v.second) ? result : false;
	        endObject();
	    }
	    return result;
	}

	// Serialize a pointer by dereferencing it
	// @param v the pointer to dereference and serialize
	template<typename T>
	bool value (T*& v) {
		if (isReading()) {
			v = new T();
		}
		return value(*v);
	}

	// Serializes an enum value as its underlying type to the file
	// @param t the enum value to serialize
	template<typename T>
	typename std::enable_if<std::is_enum<T>::value, bool>::type
	value(T& v) {
		typename std::underlying_type<T>::type temp = v;
		return value(temp);
		v = (T)temp;
	}

		// Serialize a raw DynamicArray of recipe_t (pair<int,pair<int,int>>, 3 ints)
	bool value(DynamicArray& v, Uint32 maxLength = 0) {
		typedef std::pair<int, std::pair<int, int>> recipe_t;
		Uint32 size = (Uint32)dynarray_size<recipe_t>(v);
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    v.len = 0;
		    bool result = true;
		    for (Uint32 index = 0; index < size; ++index) {
			    int a = 0, b = 0, c2 = 0;
			    result = value(a) ? result : false;
			    result = value(b) ? result : false;
			    result = value(c2) ? result : false;
			    recipe_t r = std::make_pair(a, std::make_pair(b, c2));
			    dynarray_push<recipe_t>(v, r);
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

// Serializes a class or struct to the file using it's ::serialize(FileInterface*) function
	// @param v the object to serialize
	template<typename T>
	typename std::enable_if<std::is_class<T>::value, bool>::type
	value(T& v) {
	    bool result = false;
		if (beginObject()) {
		    result = v.serialize(this);
		    endObject();
		}
		return result;
	}
	
	// Serializes a fixed-size native array
	// @param v array to serialize
	template<typename T, Uint32 Size, typename... Args>
	bool value(T (&v)[Size], Args ... args) {
		Uint32 size = Size;
		if (beginArray(size) && size == Size) {
		    bool result = true;
		    for (Uint32 index = 0; index < size; ++index) {
			    result = value(v[index], args...) ? result : false;
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

	// Helper function to serialize a property name and value at the same time 
	// @param name name of the property
	// @param v value to serialize
	// @param args additional args to pass into the value() function
	template<typename T, typename... Args>
	bool property(const char * name, T& v, Args ... args) {
		propertyName(name);
		return value(v, args...);
	}

    // As above, but if check is false, the property won't be read.
    // this allows version checking with an expression, eg:
    // propertyVersion("myInt", version >= 2, i);
    template<typename T, typename... Args>
    bool propertyVersion(const char* name, bool check, T& v, Args ... args) {
        if (!isReading() || check) {
            propertyName(name);
            return value(v, args...);
        } else {
            return true;
        }
    }

private:
	EFileFormat format = EFileFormat::Json;
	bool reading = false;
	File* fp = nullptr;
	JsonWriterState* jsonWriter = nullptr;
	JsonReaderState* jsonReader = nullptr;

	// binary string length-prefixed write/read (used when format == Binary)
	bool writeStringInternalBinary(const std::string& v);
	bool readStringInternalBinary(std::string& v);
	bool writeStringInternalBinary(const DynamicString& v);
	bool readStringInternalBinary(DynamicString& v);
};

class FileHelper {
public:
	// Write an object's data to a file
	// @param filename the name of the file to write
	// @param v the object to write
	template<typename T>
	static bool writeObject(const char * filename, EFileFormat format, T & v) {
		using std::placeholders::_1;
		SerializationFunc serialize = std::bind(&T::serialize, &v, _1);
		return writeObjectInternal(filename, format, serialize);
	}

	// Read an object's data from a file
	// @param filename the name of the file to read
	// @param v the object to populate with data
	template<typename T>
	static bool readObject(const char * filename, T & v) {
		using std::placeholders::_1;
		SerializationFunc serialize = std::bind(&T::serialize, &v, _1);
		return readObjectInternal(filename, serialize);
	}

	typedef std::function<bool(FileInterface*)> SerializationFunc;

private:

	static bool writeObjectInternal(const char * filename, EFileFormat format, const SerializationFunc& serialize);
	static bool readObjectInternal(const char * filename, const SerializationFunc& serialize);
};
