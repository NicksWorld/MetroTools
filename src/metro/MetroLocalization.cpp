#include "MetroLocalization.h"
#include "MetroContext.h"

#include "pugixml.hpp"

#include <fstream>


static CharString WideToUtf8(const WideString& wstr) {
    CharString result;

    for (wchar_t wc : wstr) {
        if (wc < 0x80) {            // U+0000..U+007F
            result.push_back(scast<char>(wc));
        } else if (wc < 0x800) {    // U+0080..U+07FF
            result.push_back(scast<char>(0xC0 | (wc >> 6)));
            result.push_back(scast<char>(0x80 | (wc & 0x3F)));
        } else {                    // U+0800..U+FFFF
            result.push_back(scast<char>(0xE0 | (wc >> 12)));
            result.push_back(scast<char>(0x80 | ((wc >> 6) & 0x3F)));
            result.push_back(scast<char>(0x80 | (wc & 0x3F)));
        }
    }

    return std::move(result);
}

static WideString Utf8ToWide(const CharString& u8str) {
    WideString result;

    const uint8_t* src = rcast<const uint8_t*>(u8str.data());
    const uint8_t* end = src + u8str.length();
    while (src < end) {
        const uint8_t lead = *src;
        wchar_t cp = scast<wchar_t>(lead);
        if (lead < 0x80) {
            // nothing
        } else if ((lead >> 5) == 0x6) {
            ++src;
            cp = ((cp << 6) & 0x7FF) + ((*src) & 0x3F);
        } else if ((lead >> 4) == 0xE) {
            ++src;
            cp = ((cp << 12) & 0xFFFF) + (((*src) << 6) & 0xFFF);
            ++src;
            cp += (*src) & 0x3F;
        } else if ((lead >> 3) == 0x1E) {
            ++src;
            cp = ((cp << 18) & 0x1FFFFF) + (((*src) << 12) & 0x3FFFF);
            ++src;
            cp += ((*src) << 6) & 0xFFF;
            ++src;
            cp += (*src) & 0x3F;
        } else {
            break;
        }

        result.push_back(cp);
        ++src;
    }

    return std::move(result);
}


enum LocalizationChunk : size_t {
    LC_CharsTable   = 0x00000001,
    LC_StringsTable = 0x00000002
};

MetroLocalization::MetroLocalization() {
}
MetroLocalization::~MetroLocalization() {
}

bool MetroLocalization::LoadFromPath(const CharString& path) {
    bool result = false;

    MetroFileSystem& mfs = MetroContext::Get().GetFilesystem();
    MyHandle file = mfs.FindFile(path);
    if (file != kInvalidHandle) {
        MemStream stream = mfs.OpenFileStream(file);
        if (stream.Good()) {
            result = this->LoadFromData(stream);
        }
    }

    return result;
}

bool MetroLocalization::LoadFromData(MemStream stream) {
    while (!stream.Ended()) {
        const size_t chunkId = stream.ReadTyped<uint32_t>();
        const size_t chunkSize = stream.ReadTyped<uint32_t>();
        const size_t chunkEnd = stream.GetCursor() + chunkSize;

        switch (chunkId) {
            case LC_CharsTable: {
                mCharsTable.resize(chunkSize / 2);
                stream.ReadToBuffer(mCharsTable.data(), chunkSize);
            } break;

            case LC_StringsTable: {
                assert(!mCharsTable.empty());
                if (mCharsTable.empty()) {
                    break;
                }

                MemStream subStream = stream.Substream(chunkSize);
                while (!subStream.Ended()) {
                    LocPair p;
                    p.key = subStream.ReadStringZ();

                    CharString encodedValue = subStream.ReadStringZ();
                    this->DecodeString(encodedValue, p.value);

                    mStrings.emplace_back(p);
                }
            } break;
        }

        stream.SetCursor(chunkEnd);
    }

    return !mStrings.empty();
}

bool MetroLocalization::LoadFromExcel2003(const fs::path& path) {
    bool result = false;

    mStrings.clear();

    std::ifstream file(path);
    if (file.good()) {
        pugi::xml_document doc;
        const uint32_t options = pugi::parse_pi | pugi::parse_ws_pcdata | pugi::parse_default;
        pugi::xml_parse_result parseResult = doc.load(file, options);
        if (parseResult) {
            pugi::xml_node decl = doc.child("mso-application");
            if (decl && decl.value() == CharString(R"(progid="Excel.Sheet")")) {
                pugi::xml_node workbook, worksheet, table, charMap;
                workbook = doc.child("Workbook");
                if (workbook) {
                    worksheet = workbook.child("Worksheet");
                    charMap = workbook.child("ss:MetroCharmap");
                }
                if (worksheet) {
                    table = worksheet.child("Table");
                }
                if (table && charMap) {
                    static CharString kRowStr = "Row";
                    static CharString kCellStr = "Cell";

                    bool foundErrors = false;

                    for (pugi::xml_node child = table.first_child(); child; child = child.next_sibling()) {
                        if (child.type() == pugi::node_element && child.name() == kRowStr) {
                            LocPair lp;

                            pugi::xml_node cell, data;
                            cell = child.first_child();
                            if (cell.type() == pugi::node_pcdata) {
                                cell = cell.next_sibling();
                            }

                            if (cell && cell.name() == kCellStr) {
                                data = cell.child("Data");
                            } else {
                                foundErrors = true;
                                break;
                            }

                            if (data) {
                                lp.key = data.text().get();
                            }

                            cell = cell.next_sibling();
                            if (cell.type() == pugi::node_pcdata) {
                                cell = cell.next_sibling();
                            }

                            if (cell && cell.name() == kCellStr) {
                                data = cell.child("Data");
                            } else {
                                foundErrors = true;
                                break;
                            }

                            if (data) {
                                lp.value = Utf8ToWide(data.text().get());
                            }

                            if (!lp.key.empty() /*&& !lp.value.empty()*/) { //#NOTE_SK: could be just empty string
                                mStrings.emplace_back(lp);
                            } else {
                                foundErrors = true;
                                break;
                            }
                        }
                    }

                    CharString charMapStr = charMap.text().get();
                    StringArray charsList = StrSplit(charMapStr, ',');
                    mCharsTable.reserve(charsList.size());
                    for (const CharString& charStr : charsList) {
                        mCharsTable.push_back(scast<wchar_t>(std::stoul(charStr)));
                    }

                    result = !foundErrors && !mCharsTable.empty() && !mStrings.empty();
                }
            }
        }
    }

    return result;
}

bool MetroLocalization::SaveToExcel2003(const fs::path& path) {
    pugi::xml_document doc;
    // add a custom declaration nodes
    // <?xml version="1.0" encoding="UTF-8"?>
    pugi::xml_node xmlDecl = doc.append_child(pugi::node_declaration);
    xmlDecl.set_name("xml");
    xmlDecl.append_attribute("version") = "1.0";
    xmlDecl.append_attribute("encoding") = "UTF-8";
    // <?mso-application progid="Excel.Sheet"?>
    pugi::xml_node excelDecl = doc.append_child(pugi::node_declaration);
    excelDecl.set_name("mso-application");
    excelDecl.append_attribute("progid") = "Excel.Sheet";

    pugi::xml_node workbook = doc.append_child("Workbook");
    workbook.append_attribute("xmlns") = "urn:schemas-microsoft-com:office:spreadsheet";
    workbook.append_attribute("xmlns:o") = "urn:schemas-microsoft-com:office:office";
    workbook.append_attribute("xmlns:x") = "urn:schemas-microsoft-com:office:excel";
    workbook.append_attribute("xmlns:ss") = "urn:schemas-microsoft-com:office:spreadsheet";
    workbook.append_attribute("xmlns:html") = R"(http://www.w3.org/TR/REC-html40)";

    pugi::xml_node worksheet = workbook.append_child("Worksheet");
    worksheet.append_attribute("ss:Name") = "Metro localization";

    pugi::xml_node table = worksheet.append_child("Table");

    pugi::xml_node col1 = table.append_child("Column");
    col1.append_attribute("ss:AutoFitWidth") = "0";
    col1.append_attribute("ss:Width") = "480";
    pugi::xml_node col2 = table.append_child("Column");
    col2.append_attribute("ss:AutoFitWidth") = "0";
    col2.append_attribute("ss:Width") = "650";

    for (const auto& p : mStrings) {
        pugi::xml_node row = table.append_child("Row");

        pugi::xml_node nodeKey = row.append_child("Cell").append_child("Data");
        nodeKey.append_attribute("ss:Type") = "String";
        nodeKey.text() = p.key.c_str();

        pugi::xml_node nodeValue = row.append_child("Cell").append_child("Data");
        nodeValue.append_attribute("ss:Type") = "String";
        nodeValue.text() = WideToUtf8(p.value).c_str();
    }

    pugi::xml_node charMap = workbook.append_child("ss:MetroCharmap");
    CharString charMapStr;
    for (const wchar_t c : mCharsTable) {
        charMapStr += std::to_string(scast<uint16_t>(c)) + ',';
    }
    charMapStr.pop_back();
    charMap.text() = charMapStr.c_str();

    return doc.save_file(path.native().c_str(), "  ", pugi::format_default, pugi::encoding_utf8);
}

static void WriteU32(std::ofstream& s, const uint32_t v) {
    s.write(rcast<const char*>(&v), sizeof(v));
}

bool MetroLocalization::Save(const fs::path& path) {
    bool result = false;

    if (!mCharsTable.empty()) {
        std::ofstream file(path, std::ofstream::binary);
        if (file.good()) {
            // first chunk, dunno why it's there
            WriteU32(file, 0);
            WriteU32(file, 4);
            WriteU32(file, 0);

            // chars table
            WriteU32(file, scast<uint32_t>(LC_CharsTable));
            WriteU32(file, scast<uint32_t>(mCharsTable.size() * sizeof(wchar_t)));
            file.write(rcast<const char*>(mCharsTable.data()), mCharsTable.size() * sizeof(wchar_t));

            // pairs
            const size_t dataSize = this->CalculateDataSize();
            WriteU32(file, scast<uint32_t>(LC_StringsTable));
            WriteU32(file, scast<uint32_t>(dataSize));

            for (const LocPair& lp : mStrings) {
                file.write(lp.key.c_str(), lp.key.length() + 1);
                BytesArray encoded;
                this->EncodeString(lp.value, encoded);
                file.write(rcast<const char*>(encoded.data()), encoded.size());
            }

            file.flush();
            file.close();

            result = true;
        }
    }

    return result;
}


size_t MetroLocalization::GetNumStrings() const {
    return mStrings.size();
}

const CharString& MetroLocalization::GetKey(const size_t idx) const {
    return mStrings[idx].key;
}

const WideString& MetroLocalization::GetValue(const size_t idx) const {
    return mStrings[idx].value;
}

size_t MetroLocalization::GetCharsCount() const {
    return mCharsTable.size();
}

wchar_t MetroLocalization::GetChar(const size_t idx) const {
    return mCharsTable[idx];
}

void MetroLocalization::DecodeString(const CharString& encodedStr, WideString& resultStr) {
    const uint8_t* codes = rcast<const uint8_t*>(encodedStr.data());
    const size_t len = encodedStr.length();
    for (size_t i = 0; i < len; ++i) {
        const size_t charIdx = scast<size_t>(codes[i]);
        if (charIdx < 224) {
            resultStr.push_back(scast<WideString::value_type>(mCharsTable[charIdx]));
        } else {
            const size_t nextCharIdx = scast<size_t>(codes[++i]);
            const size_t idx = charIdx * 255 + nextCharIdx - 0xDE41;
            resultStr.push_back(scast<WideString::value_type>(mCharsTable[idx]));
        }
    }
}

void MetroLocalization::EncodeString(const WideString& srcStr, BytesArray& encodedStr) {
    for (wchar_t c : srcStr) {
        auto it = std::find(mCharsTable.begin(), mCharsTable.end(), c);
        const size_t pos = (it == mCharsTable.end()) ? 0 : std::distance(mCharsTable.begin(), it);
        if (pos < 224) {
            encodedStr.push_back(scast<uint8_t>(pos));
        } else {
            const size_t p0 = (pos - 224) / 255;
            const size_t p1 = pos + 32 + p0 + 1;
            encodedStr.push_back(scast<uint8_t>((p0 - 32) & 0xFF));
            encodedStr.push_back(scast<uint8_t>(p1 & 0xFF));
        }
    }
    encodedStr.push_back(0);
}

MyArray<wchar_t> MetroLocalization::CollectUniqueChars() const {
    MyDict<wchar_t, bool> dict;
    MyArray<wchar_t> result;

    MyArray<wchar_t> baseTable;
    for (size_t i = 1; i < 256; ++i) {
        if (i != 32 && (i < 127 || i > 191)) {
            baseTable.push_back(scast<wchar_t>(i));
        }
    }
    baseTable.push_back(0xF8FF);

    result.push_back(0);
    result.push_back(32);
    for (const LocPair& lp : mStrings) {
        for (const wchar_t c : lp.value) {
            if (c != 32) {
                auto it = dict.find(c);
                if (it == dict.end()) {
                    dict.insert({ c, true });
                    result.push_back(c);

                    auto baseIt = std::find(baseTable.begin(), baseTable.end(), c);
                    if (baseIt != baseTable.end() && *baseIt == c) {
                        baseTable.erase(baseIt);
                    }
                }
            }
        }
    }

    if (!baseTable.empty()) {
        result.insert(result.end(), baseTable.begin(), baseTable.end());
    }

    return std::move(result);
}

size_t MetroLocalization::CalculateDataSize() const {
    size_t result = 0;

    for (const LocPair& lp : mStrings) {
        result += lp.key.length() + 1;
        result += lp.value.length() + 1;
    }

    return result;
}
