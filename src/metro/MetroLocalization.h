#pragma once
#include "mycommon.h"

class MetroLocalization {
public:
    MetroLocalization();
    ~MetroLocalization();

    bool                LoadFromPath(const CharString& path);
    bool                LoadFromData(MemStream stream);
    bool                LoadFromExcel2003(const fs::path& path);

    bool                SaveToExcel2003(const fs::path& path);
    bool                Save(const fs::path& path);

    size_t              GetNumStrings() const;
    const CharString&   GetKey(const size_t idx) const;
    const WideString&   GetValue(const size_t idx) const;

    size_t              GetCharsCount() const;
    wchar_t             GetChar(const size_t idx) const;

private:
    void                DecodeString(const CharString& encodedStr, WideString& resultStr);
    void                EncodeString(const WideString& srcStr, BytesArray& encodedStr);

    MyArray<wchar_t>    CollectUniqueChars() const;
    size_t              CalculateDataSize() const;

private:
    struct LocPair {
        CharString  key;
        WideString  value;
    };

private:
    MyArray<wchar_t>    mCharsTable;
    MyArray<LocPair>    mStrings;
};
