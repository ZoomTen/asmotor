/*  Copyright 2008-2017 Carsten Elton Sorensen

    This file is part of ASMotor.

    ASMotor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ASMotor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ASMotor.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include "xlink.h"

#define HUNK_UNIT    0x3E7u
#define HUNK_NAME    0x3E8u
#define HUNK_CODE    0x3E9u
#define HUNK_DATA    0x3EAu
#define HUNK_BSS     0x3EBu
#define HUNK_RELOC32 0x3ECu
#define HUNK_EXT     0x3EFu
#define HUNK_SYMBOL  0x3F0u
#define HUNK_END     0x3F2u
#define HUNK_HEADER  0x3F3u
#define HUNKF_CHIP   (1u << 30u)

#define EXT_DEF      0x01000000u
#define EXT_REF32    0x81000000u


static void writeInt32(FILE* f, uint32_t d)
{
    fputc((d >> 24u) & 0xFFu, f);
    fputc((d >> 16u) & 0xFFu, f);
    fputc((d >> 8u) & 0xFFu, f);
    fputc(d & 0xFFu, f);
}


static void writeBuffer(FILE* fileHandle, const void* buffer, size_t bufferSize)
{
    fwrite(buffer, 1, bufferSize, fileHandle);

    while ((bufferSize & 3u) != 0)
    {
        fputc(0, fileHandle);
        ++bufferSize;
    }
}


static uint32_t longSize(uint32_t size)
{
    return (size + 3) / 4;
}

static void writeString(FILE* fileHandle, char* string, uint32_t extFlags)
{
    uint32_t stringLength = (uint32_t) strlen(string);

    writeInt32(fileHandle, longSize(stringLength) | extFlags);
    writeBuffer(fileHandle, string, stringLength);
}


static uint32_t sectionSize(Section* section)
{
    uint32_t size = longSize(section->size);
    if (section->group->flags & GROUP_FLAG_CHIP)
        size |= HUNKF_CHIP;

    return size;
}


static void writeSectionNames(FILE* fileHandle, bool_t debugInfo)
{
    if (debugInfo)
    {
        for (Section* section = g_sections; section != NULL; section = section->nextSection)
        {
            if (section->used && section->group != NULL)
                writeString(fileHandle, section->name, 0);
        }
    }

    writeInt32(fileHandle, 0);
}


static void writeSectionSizes(FILE* fileHandle)
{
    for (Section* section = g_sections; section != NULL; section = section->nextSection)
    {
        if (section->used && section->group != NULL)
            writeInt32(fileHandle, sectionSize(section));
    }
}


static void writeStringHunk(FILE* fileHandle, uint32_t hunkType, char* hunkName)
{
    writeInt32(fileHandle, hunkType);
    writeString(fileHandle, hunkName != NULL ? hunkName : "", 0);
}


static void writeHunkUnit(FILE* fileHandle, char* hunkName)
{
    writeStringHunk(fileHandle, HUNK_UNIT, hunkName);
}


static void writeHunkName(FILE* fileHandle, char* hunkName)
{
    writeStringHunk(fileHandle, HUNK_NAME, hunkName);
}


static void writeHunkHeader(FILE* fileHandle, bool_t debugInfo, uint32_t totalSections)
{
    writeInt32(fileHandle, HUNK_HEADER);
    writeSectionNames(fileHandle, debugInfo);

    writeInt32(fileHandle, totalSections);     // total hunks
    writeInt32(fileHandle, 0);                 // first hunk number
    writeInt32(fileHandle, totalSections - 1); // last hunk number
    writeSectionSizes(fileHandle);
}


static void writeExtHunk(FILE* fileHandle, Section* section, Patches* importPatches, uint32_t codePos)
{
    // TODO: Implement when writing Amiga linker object
}


static uint32_t hunkType(Section* section)
{
    uint32_t hunkType;

    switch (section->group->type)
    {
        case GROUP_TEXT:
            if (section->group->flags & GROUP_FLAG_DATA)
                hunkType = HUNK_DATA;
            else
                hunkType = HUNK_CODE;

            break;
        default:
        case GROUP_BSS:
            hunkType = HUNK_BSS;

            break;
    }

    return hunkType;
}


typedef struct
{
    uint32_t capacity;
    uint32_t total;
    uint32_t offsets[];
} Offsets;


static Offsets** allocOffsets(uint32_t totalSections)
{
    Offsets** offsets = mem_Alloc(sizeof(Offsets*) * totalSections);
    for (uint32_t i = 0; i < totalSections; ++i)
    {
        Offsets* offset = mem_Alloc(sizeof(Offsets) + sizeof(uint32_t) * 4);
        offset->capacity = 4;
        offset->total = 0;
        offsets[i] = offset;
    }

    return offsets;
}


static void freeOffsets(Offsets** offsets, uint32_t totalSections)
{
    for (uint32_t i = 0; i < totalSections; ++i)
        mem_Free(offsets[i]);

    mem_Free(offsets);
}


static Offsets** getPatchSectionOffsets(Patches* patches, uint32_t totalSections)
{
    bool_t hasReloc = false;
    Offsets** offsets = allocOffsets(totalSections);
    Patch* patch = patches->patches;

    for (uint32_t index = 0; index < patches->totalPatches; ++index, ++patch)
    {
        if (patch->type == PATCH_RELOC)
        {
            if (patch->type == PATCH_RELOC && patch->valueSection != NULL)
            {
                Offsets** offset = &offsets[patch->valueSection->sectionId];
                if ((*offset)->total == (*offset)->capacity)
                {
                    (*offset)->capacity *= 2;
                    *offset = mem_Realloc(*offset, sizeof(Offsets) + sizeof(uint32_t) * (*offset)->capacity);
                }
                (*offset)->offsets[(*offset)->total++] = patch->offset;
                hasReloc = true;
            }
        }
    }

    if (!hasReloc)
    {
        freeOffsets(offsets, totalSections);
        return NULL;
    }

    return offsets;
}


static void writeReloc32(FILE* fileHandle, Section* section, uint32_t totalSections)
{
    Offsets** offsets = getPatchSectionOffsets(section->patches, totalSections);

    if (offsets != NULL)
    {
        writeInt32(fileHandle, HUNK_RELOC32);
        for (uint32_t sectionId = 0; sectionId < totalSections; ++sectionId)
        {
            Offsets* offset = offsets[sectionId];
            if (offset->total > 0)
            {
                writeInt32(fileHandle, offset->total);
                writeInt32(fileHandle, sectionId);

                for (uint32_t i = 0; i < offset->total; ++i)
                    writeInt32(fileHandle, offset->offsets[i]);
            }
        }
        writeInt32(fileHandle, 0);

        freeOffsets(offsets, totalSections);
    }
}


static void writeSymbolHunk(FILE* fileHandle, Section* section)
{
}


static void writeSection(FILE* fileHandle, Section* section, bool_t debugInfo, uint32_t totalSections, bool_t linkObject)
{
    if (linkObject)
        writeHunkName(fileHandle, section->name);

    writeInt32(fileHandle, hunkType(section));
    writeInt32(fileHandle, longSize(section->size));

    if (section->group->type == GROUP_TEXT)
    {
        writeBuffer(fileHandle, section->data, section->size);
        writeReloc32(fileHandle, section, totalSections);
    }

    if (linkObject)
        writeExtHunk(fileHandle, section, NULL, 0);

    if (debugInfo)
        writeSymbolHunk(fileHandle, section);

    writeInt32(fileHandle, HUNK_END);
}


static void writeSections(FILE* fileHandle, bool_t debugInfo, uint32_t totalSections, bool_t linkObject)
{
    for (Section* section = g_sections; section != NULL; section = section->nextSection)
    {
        if (section->used && section->group != NULL)
            writeSection(fileHandle, section, debugInfo, totalSections, linkObject);
    }
}


static uint32_t updateSectionIds()
{
    uint32_t sectionId = 0;

    for (Section* section = g_sections; section != NULL; section = section->nextSection)
    {
        if (section->used && section->group != NULL)
            section->sectionId = sectionId++;
        else
            section->sectionId = UINT32_MAX;
    }

    return sectionId;
}


static void writeExecutable(FILE* fileHandle, bool_t debugInfo)
{
    uint32_t totalSections = updateSectionIds();

    writeHunkHeader(fileHandle, debugInfo, totalSections);
    writeSections(fileHandle, debugInfo, totalSections, false);
}


static void writeLinkObject(FILE* fileHandle, bool_t debugInfo)
{
    uint32_t totalSections = updateSectionIds();

    writeHunkUnit(fileHandle, NULL);
    writeSections(fileHandle, debugInfo, totalSections, true);
}


static void openAndWriteFile(char* filename, void (*function)(FILE*, bool_t), bool_t debugInfo)
{
    FILE* fileHandle;

    if ((fileHandle = fopen(filename, "wb")) != NULL)
    {
        function(fileHandle, debugInfo);
        fclose(fileHandle);
    }
    else
    {
        Error("Unable to open file \"%s\" for writing", filename);
    }
}


void amiga_WriteExecutable(char* filename, bool_t debugInfo)
{
    openAndWriteFile(filename, writeExecutable, debugInfo);
}


void amiga_WriteLinkObject(char* filename, bool_t debugInfo)
{
    openAndWriteFile(filename, writeLinkObject, debugInfo);
}
