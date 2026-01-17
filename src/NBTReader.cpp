/*
 Copyright © 2021  TokiNoBug
This file is part of SlopeCraft.

    SlopeCraft is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SlopeCraft is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SlopeCraft.  If not, see <https://www.gnu.org/licenses/>.

    Contact with me:
    github:https://github.com/ToKiNoBug
    bilibili:https://space.bilibili.com/351429231
*/


#ifndef _NBTReader_Cpp
#define _NBTReader_Cpp
#include "NBTReader.h"
#include <iostream>

using namespace NBT;

// Constructor - read root compound header
NBTReader::NBTReader(const char*path)
{
    isBE=isSysBE();
    ByteCount=0;
    File=new std::ifstream(path,std::ios::in|std::ios::binary);

    if (!File->is_open()) {
        throw std::runtime_error("Failed to open file for reading");
    }

    // Read root compound: [10, 0, 0]
    char header[3];
    File->read(header, 3);
    ByteCount+=3;

    if (File->eof() || File->fail()) {
        throw std::runtime_error("Failed to read NBT header");
    }

    if (header[0] != idCompound || header[1] != 0 || header[2] != 0) {
        throw std::runtime_error("Invalid NBT file: missing root compound");
    }

    isOpen=true;
    for(top=0;top<TwinStackSize;top++)
    {
        CLA[top]=114;
        Size[top]=114514;
    }

    top=-1;
    push(idEnd, 0);  // We're now inside the root compound
}

NBTReader::NBTReader()
{
    isBE=isSysBE();
    ByteCount=0;
    File=NULL;
    isOpen=false;
    for(top=0;top<TwinStackSize;top++)
    {
        CLA[top]=114;
        Size[top]=114514;
    }

    top=-1;
}

void NBTReader::open(const char*path)
{
    if(isOpen)
    {
        return;
    }
    File=new std::ifstream(path,std::ios::in|std::ios::binary);

    if (!File->is_open()) {
        throw std::runtime_error("Failed to open file for reading");
    }

    // Read root compound: [10, 0, 0]
    char header[3];
    File->read(header, 3);
    ByteCount+=3;

    if (File->eof() || File->fail()) {
        throw std::runtime_error("Failed to read NBT header");
    }

    if (header[0] != idCompound || header[1] != 0 || header[2] != 0) {
        throw std::runtime_error("Invalid NBT file: missing root compound");
    }

    isOpen=true;
    push(idEnd, 0);  // We're now inside the root compound
}


NBTReader::~NBTReader()
{
    if(isOpen)close();
    delete File;
    return;
}

void NBTReader::close()
{
    if(isOpen)
    {
        File->close();
    }
}

bool NBTReader::isEmpty()
{
    return (top==-1);
}

bool NBTReader::isFull()
{
    return top>=TwinStackSize;
}

bool NBTReader::isListFinished()
{
    return (Size[top]<=0);
}

char NBTReader::readType()
{
    return CLA[top];
}

int NBTReader::readSize()
{
    return Size[top];
}

bool NBTReader::isInCompound()
{
    return isEmpty()||(readType()==0);
}

bool NBTReader::isInList()
{
    return !isInCompound();
}

bool NBTReader::typeMatch(char typeId)
{
    return(readType()==typeId);
}

void NBTReader::endList()
{
    if(isInList()&&isListFinished())
    {
        pop();
        elementRead();
    }
    return;
}

void NBTReader::pop()
{
    if(!isEmpty()){
    top--;
    }
    else
        ;
    return;
}

void NBTReader::push(char typeId,int size)
{
    if(!isFull())
    {
        top++;
        CLA[top]=typeId;
        Size[top]=size;
    }
    else
    return;
}

void NBTReader::elementRead()
{
    if(isInList()&&!isListFinished())
    Size[top]--;
    if(isListFinished())
    endList();
    return;
}

// Template for reading values with endianness conversion
template<typename T>
T NBTReader::readValue()
{
    T value;
    File->read(reinterpret_cast<char*>(&value), sizeof(T));
    ByteCount += sizeof(T);

    if (File->eof() || File->fail()) {
        throw std::runtime_error("Unexpected EOF while reading value");
    }

    if (!isBE && sizeof(T) > 1) {
        IE2BE(value);  // IE2BE and BE2IE are the same operation (byte swap)
    }
    return value;
}

char NBTReader::CurrentType()
{
    return readType();
}

unsigned long long NBTReader::getByteCount()
{
    return ByteCount;
}

// Peek at next tag type without consuming
char NBTReader::peekTagType()
{
    if (!isOpen) {
        throw std::runtime_error("File not open");
    }

    char type;
    File->read(&type, 1);
    if (File->eof() || File->fail()) {
        throw std::runtime_error("Unexpected EOF while peeking tag type");
    }

    // Seek back one byte
    File->seekg(-1, std::ios::cur);
    return type;
}

// Read tag type byte
char NBTReader::readTagType()
{
    char type;
    File->read(&type, 1);
    ByteCount += 1;

    if (File->eof() || File->fail()) {
        throw std::runtime_error("Unexpected EOF while reading tag type");
    }

    return type;
}

// Read tag name (length + string)
std::string NBTReader::readTagName()
{
    short nameLength = readValue<short>();
    std::string name(nameLength, '\0');
    File->read(&name[0], nameLength);
    ByteCount += nameLength;

    if (File->eof() || File->fail()) {
        throw std::runtime_error("Unexpected EOF while reading tag name");
    }

    return name;
}

// Skip a tag based on its type
void NBTReader::skipTag(char tagType)
{
    switch(tagType) {
        case idByte:
            File->seekg(1, std::ios::cur);
            ByteCount += 1;
            break;
        case idShort:
            File->seekg(2, std::ios::cur);
            ByteCount += 2;
            break;
        case idInt:
        case idFloat:
            File->seekg(4, std::ios::cur);
            ByteCount += 4;
            break;
        case idLong:
        case idDouble:
            File->seekg(8, std::ios::cur);
            ByteCount += 8;
            break;
        case idString: {
            short length = readValue<short>();
            File->seekg(length, std::ios::cur);
            ByteCount += length;
            break;
        }
        case idByteArray: {
            int length = readValue<int>();
            File->seekg(length, std::ios::cur);
            ByteCount += length;
            break;
        }
        case idIntArray: {
            int count = readValue<int>();
            File->seekg(count * 4, std::ios::cur);
            ByteCount += count * 4;
            break;
        }
        case idLongArray: {
            int count = readValue<int>();
            File->seekg(count * 8, std::ios::cur);
            ByteCount += count * 8;
            break;
        }
        case idList: {
            char elementType = readValue<char>();
            int count = readValue<int>();
            for (int i = 0; i < count; i++) {
                skipTag(elementType);
            }
            break;
        }
        case idCompound: {
            while (true) {
                char type = readTagType();
                if (type == idEnd) break;
                std::string name = readTagName();
                skipTag(type);
            }
            break;
        }
        default:
            throw std::runtime_error("Unknown tag type in skipTag");
    }
}

void NBTReader::skipCurrentTag()
{
    if (isInCompound()) {
        char type = readTagType();
        std::string name = readTagName();
        skipTag(type);
    } else if (isInList()) {
        skipTag(readType());
        elementRead();
    }
}

// Enter compound
void NBTReader::enterCompound(const char* expectedName)
{
    if (isInCompound() && expectedName != nullptr) {
        char type = readTagType();
        if (type != idCompound) {
            throw std::runtime_error("Expected COMPOUND tag");
        }

        std::string name = readTagName();
        if (name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList() && typeMatch(idCompound)) {
        // In list, no header to read
    }

    push(idEnd, 0);
}

// Exit compound (read TAG_END)
void NBTReader::exitCompound()
{
    if (!isInCompound()) {
        throw std::runtime_error("Not in compound");
    }

    char endTag = readTagType();
    if (endTag != idEnd) {
        throw std::runtime_error("Expected TAG_END");
    }

    pop();
    elementRead();
}

// Read list header
void NBTReader::readListHead(const char* expectedName, char* outElementType, int* outSize)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idList) {
            throw std::runtime_error("Expected LIST tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList() && typeMatch(idList)) {
        // List within list
    }

    char elementType = readValue<char>();
    int size = readValue<int>();

    *outElementType = elementType;
    *outSize = size;

    push(elementType, size);
    if (size == 0) {
        elementRead();
    }
}

// Read primitive tags
char NBTReader::readByte(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idByte) {
            throw std::runtime_error("Expected BYTE tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idByte)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    char value = readValue<char>();
    elementRead();
    return value;
}

short NBTReader::readShort(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idShort) {
            throw std::runtime_error("Expected SHORT tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idShort)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    short value = readValue<short>();
    elementRead();
    return value;
}

int NBTReader::readInt(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idInt) {
            throw std::runtime_error("Expected INT tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idInt)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    int value = readValue<int>();
    elementRead();
    return value;
}

long long NBTReader::readLong(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idLong) {
            throw std::runtime_error("Expected LONG tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idLong)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    long long value = readValue<long long>();
    elementRead();
    return value;
}

float NBTReader::readFloat(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idFloat) {
            throw std::runtime_error("Expected FLOAT tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idFloat)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    float value = readValue<float>();
    elementRead();
    return value;
}

double NBTReader::readDouble(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idDouble) {
            throw std::runtime_error("Expected DOUBLE tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idDouble)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    double value = readValue<double>();
    elementRead();
    return value;
}

// Read string
std::string NBTReader::readString(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idString) {
            throw std::runtime_error("Expected STRING tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    } else if (isInList()) {
        if (!typeMatch(idString)) {
            throw std::runtime_error("Type mismatch in list");
        }
    }

    short length = readValue<short>();
    std::string result(length, '\0');
    File->read(&result[0], length);
    ByteCount += length;

    if (File->eof() || File->fail()) {
        throw std::runtime_error("Unexpected EOF while reading string");
    }

    elementRead();
    return result;
}

// Read array heads
int NBTReader::readByteArrayHead(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idByteArray) {
            throw std::runtime_error("Expected BYTE_ARRAY tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    }

    int arraySize = readValue<int>();
    push(idByte, arraySize);
    if (arraySize == 0) {
        elementRead();
    }
    return arraySize;
}

int NBTReader::readIntArrayHead(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idIntArray) {
            throw std::runtime_error("Expected INT_ARRAY tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    }

    int arraySize = readValue<int>();
    push(idInt, arraySize);
    if (arraySize == 0) {
        elementRead();
    }
    return arraySize;
}

int NBTReader::readLongArrayHead(const char* expectedName)
{
    if (isInCompound()) {
        char type = readTagType();
        if (type != idLongArray) {
            throw std::runtime_error("Expected LONG_ARRAY tag");
        }

        std::string name = readTagName();
        if (expectedName != nullptr && name != expectedName) {
            throw std::runtime_error(
                std::string("Expected tag name '") + expectedName +
                "' but got '" + name + "'"
            );
        }
    }

    int arraySize = readValue<int>();
    push(idLong, arraySize);
    if (arraySize == 0) {
        elementRead();
    }
    return arraySize;
}

#endif
