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


//NBTReader - mirrors NBTWriter for reading NBT files
//Made by TokiNoBug

#ifndef _NBTREADER_H
#define _NBTREADER_H

#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <stdexcept>

#include "NBTWriter.h"

#define TwinStackSize 128

namespace NBT{

class NBTReader
{
	private:
		//Vars
		bool isOpen;
		bool isBE;
        std::ifstream *File;
		unsigned long long ByteCount;
		short top;
		char CLA[TwinStackSize];
		int Size[TwinStackSize];

		//StackFun
		void pop();
		void push(char typeId,int size);
		bool isEmpty();
		bool isFull();
		char readType();
		int readSize();

		//ReaderFun
		void elementRead();
		void endList();
		bool typeMatch(char typeId);

		//Low-level reading
		template<typename T>
		T readValue();

		short readNameLength();
		std::string readName(short length);

	public:
		//Construct&deConstruct
		NBTReader(const char*path);
		~NBTReader();
        NBTReader();
        void open(const char*path);

		//Vars

		//ReaderFun
		bool isInList();
		bool isInCompound();
		void close();
		bool isListFinished();
		char CurrentType();

		//Tag header reading
		char peekTagType();
		char readTagType();
		std::string readTagName();

		//Skip operations
		void skipTag(char tagType);
		void skipCurrentTag();

		//Read compound tags
		void enterCompound(const char*expectedName = nullptr);
		void exitCompound();

		//Read list tags
		void readListHead(const char*expectedName, char* outElementType, int* outSize);

		//ReadRealSingleTags
		char readByte(const char*expectedName = nullptr);
		short readShort(const char*expectedName = nullptr);
		int readInt(const char*expectedName = nullptr);
		long long readLong(const char*expectedName = nullptr);
		float readFloat(const char*expectedName = nullptr);
		double readDouble(const char*expectedName = nullptr);
		std::string readString(const char*expectedName = nullptr);

		//ReadArrayHeads
		int readLongArrayHead(const char*expectedName = nullptr);
		int readByteArrayHead(const char*expectedName = nullptr);
		int readIntArrayHead(const char*expectedName = nullptr);

        unsigned long long getByteCount();
};



//NameSpace NBT ends here
}


#endif
