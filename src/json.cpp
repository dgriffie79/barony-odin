#include "main.hpp"
#include "files.hpp"
#include "json.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"

#include <cassert>

const Uint32 BinaryFormatTag = *"spff";

// Opaque state backing FileInterface when writing JSON. Holds the
// rapidjson buffer + writer; the FileInterface methods forward here.
struct JsonWriterState {
	JsonWriterState(EFileFormat format)
	: buffer()
	, writer(buffer)
	{
		if ( format == EFileFormat::Json_Compact )
		{
			writer.SetIndent(' ', 2);
			writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
		}
	}

	bool beginObject() {
		return writer.StartObject();
	}
	void endObject() {
		writer.EndObject();
	}

	bool beginArray(Uint32&) {
		return writer.StartArray();
	}
	void endArray() {
		writer.EndArray();
	}

	void propertyName(const char* fieldName) {
		writer.Key(fieldName);
	}

	bool value(Uint32& value) {
		return writer.Uint(value);
	}
	bool value(Sint32& value) {
		return writer.Int(value);
	}
	bool value(float& value) {
		return writer.Double(value);
	}
	bool value(double& value) {
		return writer.Double(value);
	}
	bool value(bool& value) {
		return writer.Bool(value);
	}
	bool value(std::string& value) {
		return writer.String(value.c_str());
	}
	bool value(DynamicString& value) {
		return writer.String(value.c_str());
	}

	void save(File* file) {
		buffer.Flush();
		file->puts(buffer.GetString());
#ifdef NINTENDO
		file->putc('\0');
#endif
	}

private:
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer;
};

struct JsonReaderState {

	bool beginObject() {
		auto cv = GetCurrentValue();
		if (cv && cv->IsObject()) {
		    DocIterator di;
		    di.it = cv;
		    di.index = -1;
		    stack.push_back(di);
		    return true;
		} else {
		    return false;
		}
	}

	void endObject() {
	    if (!stack.empty()) {
		    stack.pop_back();
		}
	}

	bool beginArray(Uint32 & size) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsArray()) {
		    DocIterator di;
		    di.it = cv;
		    di.index = 0;
		    stack.push_back(di);
		    size = di.it->GetArray().Size();
		    return true;
		} else {
		    return false;
		}
	}

	void endArray() {
	    if (!stack.empty()) {
		    stack.pop_back();
		}
	}
	void propertyName(const char * fieldName) {
		propName = fieldName;
	}
	bool value(Uint32& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsUint()) {
		    value = cv->GetUint();
		    return true;
		} else {
		    return false;
		}
	}
	bool value(Sint32& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsInt()) {
		    value = cv->GetInt();
		    return true;
		} else {
		    return false;
		}
	}
	bool value(float& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsFloat()) {
		    value = cv->GetFloat();
		    return true;
		} else {
		    return false;
		}
	}
	bool value(double& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsDouble()) {
		    value = cv->GetDouble();
		    return true;
		} else {
		    return false;
		}
	}
	bool value(bool& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsBool()) {
		    value = cv->GetBool();
		    return true;
		} else {
		    return false;
		}
	}
	bool value(std::string& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsString()) {
			value = cv->GetString();
			return true;
		} else {
		    return false;
		}
	}
	bool value(DynamicString& value) {
		auto cv = GetCurrentValue();
		if (cv && cv->IsString()) {
			value = cv->GetString();
			return true;
		} else {
		    return false;
		}
	}

	bool readAllFileData(File * fp) {
		long size = fp->size();

		// reserve an extra byte for the null terminator
		char * data = (char *)calloc(sizeof(char), size + 1);
		assert(data);

		size_t bytesRead = fp->read(data, sizeof(char), size);
		if (bytesRead != size) {
			printlog("JsonFileReader: failed to read data (%d)", errno);
			free(data);
			return false;
		}

		// null terminate
		data[size] = 0;

		rapidjson::ParseResult result = doc.Parse(data);

		free(data);

		if (!result) {
			printlog("JsonFileReader: parse error: %s (%d)", rapidjson::GetParseError_En(result.Code()), result.Offset());
			return false;
		}

		return true;
	}

	rapidjson::Value::ConstValueIterator GetCurrentValue() {
		if (stack.empty()) {
		    if (propName == nullptr) {
			    return &doc;
		    } else {
		        return nullptr;
		    }
		}

		DocIterator& di = stack.back();
		if (di.it->IsArray()) {
			if (di.index >= 0) {
			    return &di.it->GetArray()[di.index++];
			} else {
			    return nullptr;
			}
		}

		if (propName != nullptr) {
		    rapidjson::Value::ConstValueIterator result;
		    if ((*di.it).HasMember(propName)) {
		        result = &(*di.it)[propName];
		    } else {
		        result = nullptr;
		    }
		    propName = nullptr;
		    return result;
		} else {
		    return nullptr;
		}
	}

	struct DocIterator {
		rapidjson::Value::ConstValueIterator it;
		Uint32 index;
	};

	rapidjson::Document doc;
	const char * propName = nullptr;
	std::vector<DocIterator> stack;
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
	// (sets len correctly; the std::string version's reserve+operator[]
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

bool FileInterface::beginObject() {
	if (format == EFileFormat::Binary) {
		return true;
	}
	return reading ? jsonReader->beginObject() : jsonWriter->beginObject();
}

void FileInterface::endObject() {
	if (format == EFileFormat::Binary) {
		return;
	}
	if (reading) jsonReader->endObject(); else jsonWriter->endObject();
}

bool FileInterface::beginArray(Uint32& size) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&size, sizeof(size), 1) == 1 : fp->write(&size, sizeof(size), 1) == 1;
	}
	return reading ? jsonReader->beginArray(size) : jsonWriter->beginArray(size);
}

void FileInterface::endArray() {
	if (format == EFileFormat::Binary) {
		return;
	}
	if (reading) jsonReader->endArray(); else jsonWriter->endArray();
}

void FileInterface::propertyName(const char* name) {
	if (format == EFileFormat::Binary) {
		return;
	}
	if (reading) jsonReader->propertyName(name); else jsonWriter->propertyName(name);
}

bool FileInterface::value(Uint32& v) {
	if (format == EFileFormat::Binary) {
		return reading ? fp->read(&v, sizeof(v), 1) == 1 : fp->write(&v, sizeof(v), 1) == 1;
	}
	return reading ? jsonReader->value(v) : jsonWriter->value(v);
}

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
