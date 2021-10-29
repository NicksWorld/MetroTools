#include "MetroReflection.h"
#include "jansson.h"

void MetroReflectionJsonWriteStream::SerializeBool(bool& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, v ? json_true() : json_false());
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), v ? json_true() : json_false());
    }
}

void MetroReflectionJsonWriteStream::SerializeI8(int8_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeU8(uint8_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeI16(int16_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeU16(uint16_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeI32(int32_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeU32(uint32_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeI64(int64_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeU64(uint64_t& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_integer(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_integer(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeF32(float& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_real(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_real(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeF64(double& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_real(v));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_real(v));
    }
}

void MetroReflectionJsonWriteStream::SerializeString(CharString& v) {
    if (mIsArray) {
        json_array_append_new(mCurrentObj, json_string_nocheck(v.c_str()));
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), json_string_nocheck(v.c_str()));
    }
}

// Advanced
void MetroReflectionJsonWriteStream::SerializeFloats(float* fv, const size_t numFloats) {
    json_t* a = json_array();
    for (size_t i = 0; i < numFloats; ++i) {
        json_array_append_new(a, json_real(fv[i]));
    }

    if (mIsArray) {
        json_array_append_new(mCurrentObj, a);
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), a);
    }
}

void MetroReflectionJsonWriteStream::SerializeInts(int* iv, const size_t numInts) {
    json_t* a = json_array();
    for (size_t i = 0; i < numInts; ++i) {
        json_array_append_new(a, json_integer(iv[i]));
    }

    if (mIsArray) {
        json_array_append_new(mCurrentObj, a);
    } else {
        json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), a);
    }
}

// Advanced
void MetroReflectionJsonWriteStream::FlushSectionTail(BytesArray& data) {
    CharString savedName = mCurrentMemberName;
    mCurrentMemberName = "_tail_blob_";
    CharString str = Encode_BytesToBase64(data.data(), data.size());
    this->SerializeString(str);
}

// Need to override this as we just need the name, not a type
bool MetroReflectionJsonWriteStream::SerializeTypeInfo(const CharString& propName, const CharString&) {
    mCurrentMemberName = propName;
    return true;
}

MetroReflectionStream* MetroReflectionJsonWriteStream::OpenSection(const CharString& sectionName, const bool /*nameUnknown*/) {
    mCurrentMemberName = sectionName;
    json_t* o = json_object();
    json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), o);

    mTopObjectsStack.push_front({ mCurrentObj, mIsArray });
    mCurrentObj = o;
    mIsArray = false;

    return this;
}

void MetroReflectionJsonWriteStream::CloseSection(MetroReflectionStream* section, const bool) {
    assert(this == section);
    if (this == section) {
        ObjMode& om = mTopObjectsStack.front();
        mCurrentObj = om.j;
        mIsArray = om.isArray;
        mTopObjectsStack.pop_front();
    }
}

void MetroReflectionJsonWriteStream::BeginArray16(uint16_t&) {
    json_t* a = json_array();
    json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), a);

    mTopObjectsStack.push_front({ mCurrentObj, mIsArray });
    mCurrentObj = a;
    mIsArray = true;
}

void MetroReflectionJsonWriteStream::BeginArray32(uint32_t&) {
    json_t* a = json_array();
    json_object_set_new_nocheck(mCurrentObj, mCurrentMemberName.c_str(), a);

    mTopObjectsStack.push_front({ mCurrentObj, mIsArray });
    mCurrentObj = a;
    mIsArray = true;
}

void MetroReflectionJsonWriteStream::EndArray() {
    ObjMode& om = mTopObjectsStack.front();
    mCurrentObj = om.j;
    mIsArray = om.isArray;
    mTopObjectsStack.pop_front();
}

// Json specific
CharString MetroReflectionJsonWriteStream::WriteToString() const {
    CharString result;

    char* str = json_dumps(mJson, JSON_INDENT(2));
    if (str) {
        result = str;
        free(str);
    }

    return result;
}

void MetroReflectionJsonWriteStream::Initialize() {
    mJson = json_object();
    mCurrentObj = mJson;
}

void MetroReflectionJsonWriteStream::Shutdown() {
    json_decref(mJson);
    mJson = nullptr;
    mCurrentObj = nullptr;

    mTopObjectsStack.clear();
}



// READ

// Specialize these
void MetroReflectionJsonReadStream::SerializeBool(bool& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_boolean(j));
    v = json_is_true(j);
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeI8(int8_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<int8_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeU8(uint8_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<uint8_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeI16(int16_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<int16_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeU16(uint16_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<uint16_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeI32(int32_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<int32_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeU32(uint32_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<uint32_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeI64(int64_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<int64_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeU64(uint64_t& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_integer(j));
    v = scast<uint64_t>(json_integer_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeF32(float& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_real(j));
    v = scast<float>(json_real_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeF64(double& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_real(j));
    v = scast<double>(json_real_value(j));
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeString(CharString& v) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_string(j));
    v = json_string_value(j);
    this->Iterate();
}

// Advanced
void MetroReflectionJsonReadStream::SerializeFloats(float* fv, const size_t numFloats) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_array(j) && json_array_size(j) == numFloats);
    if (json_is_array(j) && json_array_size(j) == numFloats) {
        size_t idx;
        json_t* elem;
        json_array_foreach(j, idx, elem) {
            fv[idx] = scast<float>(json_real_value(elem));
        }
    }
    this->Iterate();
}

void MetroReflectionJsonReadStream::SerializeInts(int* iv, const size_t numInts) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    assert(json_is_array(j) && json_array_size(j) == numInts);
    if (json_is_array(j) && json_array_size(j) == numInts) {
        size_t idx;
        json_t* elem;
        json_array_foreach(j, idx, elem) {
            iv[idx] = scast<int>(json_integer_value(elem));
        }
    }
    this->Iterate();
}

// Advanced
void MetroReflectionJsonReadStream::FlushSectionTail(BytesArray& data) {
    CharString base64;
    this->SerializeString(base64);
    data = Encode_Base64ToBytes(base64);
}

// Need to override this as we just need the name, not a type
bool MetroReflectionJsonReadStream::SerializeTypeInfo(const CharString& propName, const CharString&) {
    return mCurrentMemberName == propName;
}

MetroReflectionStream* MetroReflectionJsonReadStream::OpenSection(const CharString& sectionName, const bool nameUnknown) {
    MetroReflectionStream* result = nullptr;

    if (nameUnknown || mCurrentMemberName == sectionName) {
        json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
        if (json_is_object(j)) {
            mTopObjectsStack.push_front({ mCurrentObj, mCurrentIter, mIsArray });
            mCurrentIter = json_object_iter(j);
            mCurrentObj = j;
            mIsArray = false;
            mCurrentMemberName = json_object_iter_key(mCurrentIter);
            result = this;
        }
    }

    return result;
}

void MetroReflectionJsonReadStream::CloseSection(MetroReflectionStream* section, const bool) {
    if (section == this) {
        ObjIter& oi = mTopObjectsStack.front();
        mCurrentObj = oi.j;
        mCurrentIter = oi.i;
        mIsArray = oi.isArray;
        mTopObjectsStack.pop_front();
        this->Iterate();
    }
}

void MetroReflectionJsonReadStream::BeginArray16(uint16_t& size) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    if (json_is_array(j)) {
        mTopObjectsStack.push_front({ mCurrentObj, mCurrentIter, mIsArray });
        mCurrentIndex = 0;
        mCurrentObj = j;
        mIsArray = true;

        size = scast<uint16_t>(json_array_size(j));
    } else {
        assert(false);
    }
}

void MetroReflectionJsonReadStream::BeginArray32(uint32_t& size) {
    json_t* j = mIsArray ? json_array_get(mCurrentObj, mCurrentIndex) : json_object_iter_value(mCurrentIter);
    if (json_is_array(j)) {
        mTopObjectsStack.push_front({ mCurrentObj, mCurrentIter, mIsArray });
        mCurrentIndex = 0;
        mCurrentObj = j;
        mIsArray = true;

        size = scast<uint32_t>(json_array_size(j));
    } else {
        assert(false);
    }
}

void MetroReflectionJsonReadStream::EndArray() {
    if (json_is_array(mCurrentObj)) {
        ObjIter& oi = mTopObjectsStack.front();
        mCurrentObj = oi.j;
        mCurrentIter = oi.i;
        mIsArray = oi.isArray;
        mTopObjectsStack.pop_front();
        this->Iterate();
    } else {
        assert(false);
    }
}

void MetroReflectionJsonReadStream::Initialize(const CharString& jsonStr) {
    json_error_t jsonErr = {};
    json_t* json = json_loads(jsonStr.c_str(), JSON_DISABLE_EOF_CHECK, &jsonErr);
    this->Initialize(json);
}

void MetroReflectionJsonReadStream::Initialize(json_t* json) {
    if (json) {
        mJson = json;
        mCurrentObj = mJson;
        mCurrentIter = json_object_iter(mJson);
        mCurrentMemberName = json_object_iter_key(mCurrentIter);
    }
}

void MetroReflectionJsonReadStream::Shutdown() {
    json_decref(mJson);
    mJson = nullptr;
    mCurrentObj = nullptr;
    mCurrentIter = nullptr;
}

void MetroReflectionJsonReadStream::Iterate() {
    if (mIsArray) {
        ++mCurrentIndex;
    } else if (mCurrentIter) {
        void* next = json_object_iter_next(mCurrentObj, mCurrentIter);
        mCurrentIter = next;
        if (next) {
            mCurrentMemberName = json_object_iter_key(next);
        }
    }
}
