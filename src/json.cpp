#include "main.hpp"
#include "files.hpp"
#include "json.hpp"

#include "../odin/json_shim/json_shim.hpp"

#include <cassert>

const Uint32 BinaryFormatTag = *"spff";

// Opaque state backing FileInterface when writing JSON. Delegates to the
// Odin JSON shim (odin/json_shim); the FileInterface methods forward here.
struct JsonWriterState {
	void* handle;

	JsonWriterState(EFileFormat format) {
		handle = json_writer_create(format == EFileFormat::Json_Compact);
	}
	~JsonWriterState() {
		json_writer_destroy(handle);
	}

	bool beginObject() { return json_writer_begin_object(handle); }
	void endObject() { json_writer_end_object(handle); }

	bool beginArray(Uint32&) { return json_writer_begin_array(handle); }
	void endArray() { json_writer_end_array(handle); }

	void propertyName(const char* fieldName) { json_writer_key(handle, fieldName); }

	bool value(Uint32& value) { return json_writer_uint(handle, value); }
	bool value(Sint32& value) { return json_writer_int(handle, value); }
	bool value(float& value) { return json_writer_double(handle, (double)value); }
	bool value(double& value) { return json_writer_double(handle, value); }
	bool value(bool& value) { return json_writer_bool(handle, value); }
	bool value(std::string& value) { return json_writer_string(handle, value.c_str()); }
	bool value(DynamicString& value) { return json_writer_string(handle, value.c_str()); }

	void save(File* file) {
		file->puts(json_writer_get_string(handle));
	}
};

struct JsonReaderState {
	void* handle = nullptr;

	JsonReaderState() = default;
	~JsonReaderState() {
		if (handle) json_reader_destroy(handle);
	}

	bool beginObject() { return json_reader_begin_object(handle); }
	void endObject() { json_reader_end_object(handle); }

	bool beginArray(Uint32& size) { return json_reader_begin_array(handle, &size); }
	void endArray() { json_reader_end_array(handle); }

	void propertyName(const char* fieldName) { json_reader_property_name(handle, fieldName); }

	bool value(Uint32& value) { return json_reader_value_uint(handle, &value); }
	bool value(Sint32& value) { return json_reader_value_int(handle, &value); }
	bool value(float& value) { return json_reader_value_float(handle, &value); }
	bool value(double& value) { return json_reader_value_double(handle, &value); }
	bool value(bool& value) { return json_reader_value_bool(handle, &value); }
	bool value(std::string& value) {
		const char* s;
		if (!json_reader_value_string(handle, &s)) return false;
		value = s;
		return true;
	}
	bool value(DynamicString& value) {
		const char* s;
		if (!json_reader_value_string(handle, &s)) return false;
		value = s;
		return true;
	}

	bool readAllFileData(File* fp) {
		long size = fp->size();

		// reserve an extra byte for the null terminator
		char* data = (char*)calloc(sizeof(char), size + 1);
		assert(data);

		size_t bytesRead = fp->read(data, sizeof(char), size);
		if (bytesRead != size) {
			printlog("JsonFileReader: failed to read data (%d)", errno);
			free(data);
			return false;
		}

		// null terminate
		data[size] = 0;

		handle = json_reader_parse(data);

		free(data);

		if (!handle) {
			printlog("JsonFileReader: parse error");
			return false;
		}

		return true;
	}
};

struct BinaryWriterState {

	BinaryWriterState(File* file)
	: fp(file)
	{
	}

	bool beginObject() {
	    return true;
	}

	void endObject() {
	}

	bool beginArray(Uint32& size) {
		return fp->write(&size, sizeof(size), 1) == 1;
	}

	void endArray() {
	}

	void propertyName(const char*) {
	}

	bool value(Uint32& v) {
		return fp->write(&v, sizeof(v), 1) == 1;
	}
	bool value(Sint32& v) {
		return fp->write(&v, sizeof(v), 1) == 1;
	}
	bool value(float& v) {
		return fp->write(&v, sizeof(v), 1) == 1;
	}
	bool value(double& v) {
		return fp->write(&v, sizeof(v), 1) == 1;
	}
	bool value(bool& v) {
		return fp->write(&v, sizeof(v), 1) == 1;
	}
	bool value(std::string& v) {
		return writeStringInternal(v);
	}
	bool value(DynamicString& v) {
		return writeStringInternal(v);
	}

	void writeHeader() {
		(void)fp->write(&BinaryFormatTag, sizeof(BinaryFormatTag), 1);
	}

	bool writeStringInternal(const std::string& v) {
		Uint32 len = (Uint32)v.size();
		bool result = true;
		result = fp->write(&len, sizeof(len), 1) == 1 ? result : false;
		if (len) {
			result = fp->write(v.c_str(), sizeof(char), len) == len ?
			    result : false;
		}
		return result;
	}

	bool writeStringInternal(const DynamicString& v) {
		Uint32 len = (Uint32)v.size();
		bool result = true;
		result = fp->write(&len, sizeof(len), 1) == 1 ? result : false;
		if (len) {
			result = fp->write(v.c_str(), sizeof(char), len) == len ?
			    result : false;
		}
		return result;
	}

	File* fp = nullptr;
};

struct BinaryReaderState {

	BinaryReaderState(File* file)
		: fp(file)
	{
	}

	bool beginObject() {
	    return true;
	}

	void endObject() {
	}

	bool beginArray(Uint32& size) {
		return fp->read(&size, sizeof(size), 1) == 1;
	}

	void endArray() {
	}

	void propertyName(const char*) {
	}

	bool value(Uint32& v) {
		size_t read = fp->read(&v, sizeof(v), 1);
		return read == 1;
	}
	bool value(Sint32& v) {
		size_t read = fp->read(&v, sizeof(v), 1);
		return read == 1;
	}
	bool value(float& v) {
		size_t read = fp->read(&v, sizeof(v), 1);
		return read == 1;
	}
	bool value(double& v) {
		size_t read = fp->read(&v, sizeof(v), 1);
		return read == 1;
	}
	bool value(bool& v) {
		size_t read = fp->read(&v, sizeof(v), 1);
		return read == 1;
	}
	bool value(std::string& v) {
		bool result = readStringInternal(v);
		return result;
	}
	bool value(DynamicString& v) {
		bool result = readStringInternal(v);
		return result;
	}

	bool readHeader() {
		Uint32 fileFormatTag;
		size_t read = fp->read(&fileFormatTag, sizeof(fileFormatTag), 1);
		if (read != 1) {
			printlog("BinaryFileReader: failed to read format tag (%d)", errno);
			return false;
		}

		if (fileFormatTag != BinaryFormatTag) {
			printlog("BinaryFileReader: file format tag mismatch (expected %x, got %x)", BinaryFormatTag, fileFormatTag);
			return false;
		}

		return true;
	}

	bool readStringInternal(std::string & v) {
		Uint32 len;
		bool result = true;
		size_t read = fp->read(&len, sizeof(len), 1);
		result = read == 1 ? result : false;

		if (len) {
			v.reserve(len);
			read = fp->read(&v[0u], sizeof(char), len);
		    result = read == len ? result : false;
		}

		return result;
	}

	// DynamicString read: reads the length-prefixed bytes and appends them
	// (sets len correctly; the DynamicString version's reserve+operator[]
	// leaves size stale — we match the FORMAT, not the bug).
	bool readStringInternal(DynamicString & v) {
		Uint32 len;
		bool result = true;
		size_t read = fp->read(&len, sizeof(len), 1);
		result = read == 1 ? result : false;

		v.clear();
		if (len) {
			// read into a temp then append (sets len properly)
			char* tmp = (char*)malloc(len);
			if (tmp) {
				read = fp->read(tmp, sizeof(char), len);
				result = read == len ? result : false;
				v.append(tmp, (int64_t)len);
				free(tmp);
			} else {
				result = false;
			}
		}

		return result;
	}

	File* fp;
};

static EFileFormat GetFileFormat(File * file) {
	Uint32 fileFormatTag = 0;
	file->read(&fileFormatTag, sizeof(fileFormatTag), 1);
	file->seek(0, File::SeekMode::SET);

	if (fileFormatTag == BinaryFormatTag) {
		return EFileFormat::Binary;
	}
	else {
		return EFileFormat::Json;
	}
}

FileInterface::~FileInterface() {
	delete jsonWriter;
	delete jsonReader;
}

FileInterface::FileInterface(FileInterface&& other) noexcept
	: format(other.format), reading(other.reading), fp(other.fp),
	  jsonWriter(other.jsonWriter), jsonReader(other.jsonReader) {
	other.format = EFileFormat::Json;
	other.reading = false;
	other.fp = nullptr;
	other.jsonWriter = nullptr;
	other.jsonReader = nullptr;
}

FileInterface& FileInterface::operator=(FileInterface&& other) noexcept {
	if (this != &other) {
		delete jsonWriter;
		delete jsonReader;
		format = other.format;
		reading = other.reading;
		fp = other.fp;
		jsonWriter = other.jsonWriter;
		jsonReader = other.jsonReader;
		other.format = EFileFormat::Json;
		other.reading = false;
		other.fp = nullptr;
		other.jsonWriter = nullptr;
		other.jsonReader = nullptr;
	}
	return *this;
}



FileInterface FileInterface::makeWriter(File* file, EFileFormat format) {
	FileInterface result;
	result.format = format;
	result.reading = false;
	result.fp = file;
	if (format == EFileFormat::Binary) {
		// binary writes the header immediately; state is nil (fp used directly)
		(void)file->write(&BinaryFormatTag, sizeof(BinaryFormatTag), 1);
	} else {
		result.jsonWriter = new JsonWriterState(format);
	}
	return result;
}

extern "C" FileInterface FileInterface_makeWriter(File * file, EFileFormat format) { return FileInterface::makeWriter(file, format); }


FileInterface FileInterface::makeReader(File* file) {
	FileInterface result;
	EFileFormat format = GetFileFormat(file);
	result.format = format;
	result.reading = true;
	result.fp = file;
	if (format == EFileFormat::Binary) {
		// GetFileFormat peeked the 4-byte tag then seeked back to 0;
		// skip past the tag so data reads start at the first payload byte.
		Uint32 tag = 0;
		(void)file->read(&tag, sizeof(tag), 1);
	} else {
		result.jsonReader = new JsonReaderState();
		result.jsonReader->readAllFileData(file);
	}
	return result;
}

extern "C" FileInterface FileInterface_makeReader(File * file) { return FileInterface::makeReader(file); }


bool FileInterface::beginObject() {
	if (format == EFileFormat::Binary) {
		return true;
	}
	return reading ? jsonReader->beginObject() : jsonWriter->beginObject();
}

extern "C" bool FileInterface_beginObject(FileInterface* self) { return self->beginObject(); }


void FileInterface::endObject() {
	if (format == EFileFormat::Binary) {
		return;
	}
	if (reading) jsonReader->endObject(); else jsonWriter->endObject();
}

extern "C" void FileInterface_endObject(FileInterface* self) { return self->endObject(); }


bool FileInterface::beginArray(Uint32& size) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&size, sizeof(size), 1) == 1 : fp->write(&size, sizeof(size), 1) == 1;
	}
	return reading ? jsonReader->beginArray(size) : jsonWriter->beginArray(size);
}

extern "C" bool FileInterface_beginArray(FileInterface* self, Uint32 & size) { return self->beginArray(size); }


void FileInterface::endArray() {
	if (format == EFileFormat::Binary) {
		return;
	}
	if (reading) jsonReader->endArray(); else jsonWriter->endArray();
}

extern "C" void FileInterface_endArray(FileInterface* self) { return self->endArray(); }


void FileInterface::propertyName(const char* name) {
	if (format == EFileFormat::Binary) {
		return;
	}
	if (reading) jsonReader->propertyName(name); else jsonWriter->propertyName(name);
}

extern "C" void FileInterface_propertyName(FileInterface* self, const char * name) { return self->propertyName(name); }


bool FileInterface::value(Uint32& v) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&v, sizeof(v), 1) == 1 : fp->write(&v, sizeof(v), 1) == 1;
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}


extern "C" bool FileInterface_value_10(FileInterface* self, int & v) { return self->value(v); }




extern "C" bool FileInterface_value_7(FileInterface* self, DynamicString & v) { return self->value(v); }


extern "C" bool FileInterface_value_6(FileInterface* self, std::string & v) { return self->value(v); }


extern "C" bool FileInterface_value_5(FileInterface* self, bool & v) { return self->value(v); }


extern "C" bool FileInterface_value_4(FileInterface* self, double & v) { return self->value(v); }


extern "C" bool FileInterface_value_3(FileInterface* self, float & v) { return self->value(v); }


extern "C" bool FileInterface_value_2(FileInterface* self, int & v) { return self->value(v); }


extern "C" bool FileInterface_value(FileInterface* self, int & v) { return self->value(v); }


bool FileInterface::value(Sint32& v) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&v, sizeof(v), 1) == 1 : fp->write(&v, sizeof(v), 1) == 1;
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

bool FileInterface::value(float& v) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&v, sizeof(v), 1) == 1 : fp->write(&v, sizeof(v), 1) == 1;
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

bool FileInterface::value(double& v) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&v, sizeof(v), 1) == 1 : fp->write(&v, sizeof(v), 1) == 1;
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

bool FileInterface::value(bool& v) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&v, sizeof(v), 1) == 1 : fp->write(&v, sizeof(v), 1) == 1;
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

bool FileInterface::value(std::string& v) {
	if (format == EFileFormat::Binary) {
		return reading ? readStringInternalBinary(v) : writeStringInternalBinary(v);
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

bool FileInterface::value(DynamicString& v) {
	if (format == EFileFormat::Binary) {
		return reading ? readStringInternalBinary(v) : writeStringInternalBinary(v);
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

void FileInterface::flushToFile() {
	if (!reading && jsonWriter) {
		jsonWriter->save(fp);
	}
}

extern "C" void FileInterface_flushToFile(FileInterface* self) { return self->flushToFile(); }


bool FileInterface::writeStringInternalBinary(const std::string& v) {
	Uint32 len = (Uint32)v.size();
	bool result = true;
	result = fp->write(&len, sizeof(len), 1) == 1 ? result : false;
	if (len) {
		result = fp->write(v.c_str(), sizeof(char), len) == len ?
		    result : false;
	}
	return result;
}

extern "C" bool FileInterface_writeStringInternalBinary_2(FileInterface* self, const DynamicString & v) { return self->writeStringInternalBinary(v); }


extern "C" bool FileInterface_writeStringInternalBinary(FileInterface* self, const std::string & v) { return self->writeStringInternalBinary(v); }


bool FileInterface::writeStringInternalBinary(const DynamicString& v) {
	Uint32 len = (Uint32)v.size();
	bool result = true;
	result = fp->write(&len, sizeof(len), 1) == 1 ? result : false;
	if (len) {
		result = fp->write(v.c_str(), sizeof(char), len) == len ?
		    result : false;
	}
	return result;
}

bool FileInterface::readStringInternalBinary(std::string& v) {
	Uint32 len;
	bool result = true;
	size_t read = fp->read(&len, sizeof(len), 1);
	result = read == 1 ? result : false;

	if (len) {
		v.reserve(len);
		read = fp->read(&v[0u], sizeof(char), len);
	    result = read == len ? result : false;
	}

	return result;
}

extern "C" bool FileInterface_readStringInternalBinary_2(FileInterface* self, DynamicString & v) { return self->readStringInternalBinary(v); }


extern "C" bool FileInterface_readStringInternalBinary(FileInterface* self, std::string & v) { return self->readStringInternalBinary(v); }


bool FileInterface::readStringInternalBinary(DynamicString& v) {
	Uint32 len;
	bool result = true;
	size_t read = fp->read(&len, sizeof(len), 1);
	result = read == 1 ? result : false;

	v.clear();
	if (len) {
		char* tmp = (char*)malloc(len);
		if (tmp) {
			read = fp->read(tmp, sizeof(char), len);
			result = read == len ? result : false;
			v.append(tmp, (int64_t)len);
			free(tmp);
		} else {
			result = false;
		}
	}

	return result;
}

//TODO: NX PORT: Update for the Switch?
bool FileHelper::writeObjectInternal(const char * filename, EFileFormat format, const SerializationFunc& serialize) {
	File * file = FileIO::open(filename, "wb");
#ifndef NDEBUG
	printlog("Opening file '%s' for write", filename);
#endif
	if (!file) {
		printlog("Unable to open file '%s' for write (%d)", filename, errno);
		return false;
	}

	bool success = false;
	{
		FileInterface fi = FileInterface::makeWriter(file, format);
		if (fi.beginObject()) {
		    success = serialize(&fi);
		    fi.endObject();
		}
		// JSON writer flushes its buffer to the file on destruction
		if (format != EFileFormat::Binary) {
			fi.flushToFile();
		}
	}

	FileIO::close(file);

	return success;
}



bool FileHelper::readObjectInternal(const char * filename, const SerializationFunc& serialize) {
	File * file = FileIO::open(filename, "rb");
#ifndef NDEBUG
	printlog("Opening file '%s' for read", filename);
#endif
	if (!file) {
		printlog("Unable to open file '%s' for read (%d)", filename, errno);
		return false;
	}

	bool success = false;
	{
		FileInterface fi = FileInterface::makeReader(file);
		if (fi.beginObject()) {
		    success = serialize(&fi);
		    fi.endObject();
		}
	}

	FileIO::close(file);

	return success;
}



bool FileInterface::value(DynamicArrayS32& v, Uint32 maxLength ) {
		Uint32 size = (Uint32)v.size();
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    bool result = true;
		    if (isReading()) {
		        v.clear();
		        for (Uint32 index = 0; index < size; ++index) {
			        int32_t elem = 0;
			        result = value(elem) ? result : false;
			        v.push_back(elem);
		        }
		    } else {
		        for (Uint32 index = 0; index < size; ++index) {
			        result = value(v[index]) ? result : false;
		        }
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

bool FileInterface::value(DynamicArrayU32& v, Uint32 maxLength ) {
		Uint32 size = (Uint32)v.size();
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    bool result = true;
		    if (isReading()) {
		        v.clear();
		        for (Uint32 index = 0; index < size; ++index) {
			        uint32_t elem = 0;
			        result = value(elem) ? result : false;
			        v.push_back(elem);
		        }
		    } else {
		        for (Uint32 index = 0; index < size; ++index) {
			        result = value(v[index]) ? result : false;
		        }
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}

bool FileInterface::value(DynamicStringPair_t& v) {
	    bool result = false;
	    if (beginObject()) {
	        result = true;
	        result = property("first", v.first) ? result : false;
	        result = property("second", v.second) ? result : false;
	        endObject();
	    }
	    return result;
	}

bool FileInterface::value(DynamicArray& v, Uint32 maxLength ) {
		typedef std::pair<int, std::pair<int, int>> recipe_t;
		Uint32 size = (Uint32)dynarray_size<recipe_t>(v);
		if (beginArray(size) && (maxLength == 0 || size <= maxLength)) {
		    bool result = true;
		    if (isReading()) {
		        v.len = 0;
		        for (Uint32 index = 0; index < size; ++index) {
			        int a = 0, b = 0, c2 = 0;
			        result = value(a) ? result : false;
			        result = value(b) ? result : false;
			        result = value(c2) ? result : false;
			        recipe_t r = std::make_pair(a, std::make_pair(b, c2));
			        dynarray_push<recipe_t>(v, r);
		        }
		    } else {
		        for (Uint32 index = 0; index < size; ++index) {
			        recipe_t* r = dynarray_at<recipe_t>(v, index);
			        result = value(r->first) ? result : false;
			        result = value(r->second.first) ? result : false;
			        result = value(r->second.second) ? result : false;
		        }
		    }
		    endArray();
		    return result;
		} else {
		    return false;
		}
	}
