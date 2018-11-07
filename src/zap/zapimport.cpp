// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// ZapImport.cpp
//

//
// Zapping of soft bound references to elements outside the current module
//
// ======================================================================================

#include "common.h"

#include "zapimport.h"

#include "nibblestream.h"
#include "sigbuilder.h"

#if defined(FEATURE_READYTORUN_COMPILER)
// A flag to indicate that a helper call uses VSD
const DWORD READYTORUN_HELPER_FLAG_VSD = 0x10000000;
#endif

//
// ZapImportTable
//

static const struct ImportSectionProperties 
{
    BYTE                    Type;
    BYTE                    EntrySize;
    WORD                    Flags;
}
c_ImportSectionProperties[ZapImportSectionType_Count] =
{
    { /* ZapImportSectionType_Handle,       */ CORCOMPILE_IMPORT_TYPE_UNKNOWN,         0,                    0                               },
    { /* ZapImportSectionType_TypeHandle,   */ CORCOMPILE_IMPORT_TYPE_TYPE_HANDLE,     TARGET_POINTER_SIZE,  0                               },
    { /* ZapImportSectionType_MethodHandle, */ CORCOMPILE_IMPORT_TYPE_METHOD_HANDLE,   TARGET_POINTER_SIZE,  0                               },
#ifdef _TARGET_ARM_
    { /* ZapImportSectionType_PCode,        */ CORCOMPILE_IMPORT_TYPE_UNKNOWN,         0,                    CORCOMPILE_IMPORT_FLAGS_PCODE   },
#endif
    { /* ZapImportSectionType_StringHandle, */ CORCOMPILE_IMPORT_TYPE_STRING_HANDLE,   TARGET_POINTER_SIZE,  0                               },
};

// Sort ZapImport* by CorCompileTokenTable as primary key and offset within the table as secondary key
static int __cdecl fixupCmp(const void* a_, const void* b_)
{
    ZapImport *a = *(ZapImport **)a_;
    ZapImport *b = *(ZapImport **)b_;

    int tableDiff = a->GetSectionIndex() - b->GetSectionIndex();
    if (tableDiff != 0)
        return tableDiff;

    // Sort by offset within the table
    return (a->GetOffset() - b->GetOffset());
}
void ZapImport::Save(ZapWriter * pZapWriter)
{
    if (IsReadyToRunCompilation())
    {
        TARGET_POINTER_TYPE value = 0;
        pZapWriter->Write(&value, sizeof(value));
        return;
    }

    TARGET_POINTER_TYPE token = CORCOMPILE_TAG_TOKEN(GetBlob()->GetRVA());
    pZapWriter->Write(&token, sizeof(token));
}

//
// CORCOMPILE_CODE_IMPORT_SECTION
//

COUNT_T ZapImportSectionsTable::Append(BYTE Type, USHORT Flags, BYTE EntrySize, ZapVirtualSection * pSection, ZapNode * pSignatures, ZapNode * pAuxiliaryData)
{
    ImportSection entry;

    entry.m_pSection = pSection;
    entry.m_pSignatures = pSignatures;
    entry.m_pAuxiliaryData = pAuxiliaryData;
    entry.m_Flags = Flags;
    entry.m_Type = Type;
    entry.m_EntrySize = EntrySize;

    m_ImportSectionsTable.Append(entry);

    return m_ImportSectionsTable.GetCount() - 1;
}

DWORD ZapImportSectionsTable::GetSize()
{
    return m_ImportSectionsTable.GetCount() * sizeof(CORCOMPILE_IMPORT_SECTION);
}

void ZapImportSectionsTable::Save(ZapWriter * pZapWriter)
{
    COUNT_T nSections = m_ImportSectionsTable.GetCount();
    for (COUNT_T iSection = 0; iSection < nSections; iSection++)
    {
        ImportSection * p = &m_ImportSectionsTable[iSection];

        CORCOMPILE_IMPORT_SECTION entry;

        ZapWriter::SetDirectoryData(&entry.Section, p->m_pSection);

        entry.Flags = p->m_Flags;
        entry.Type = p->m_Type;
        entry.EntrySize = p->m_EntrySize;

        entry.Signatures = (p->m_pSignatures != NULL) ? p->m_pSignatures->GetRVA() : NULL;
        entry.AuxiliaryData = (p->m_pAuxiliaryData != NULL) ? p->m_pAuxiliaryData->GetRVA() : NULL;

        pZapWriter->Write(&entry, sizeof(entry));
    }
}


ZapImportSectionSignatures::ZapImportSectionSignatures(ZapImage * pImage, ZapVirtualSection * pImportSection, ZapVirtualSection * pGCSection)
    : m_pImportSection(pImportSection), m_pImage(pImage)
{
    if (pGCSection != NULL)
    {
        m_pGCRefMapTable = new (pImage->GetHeap()) ZapGCRefMapTable(pImage);
        pGCSection->Place(m_pGCRefMapTable);
    }
}

ZapImportSectionSignatures::~ZapImportSectionSignatures()
{
    if (m_pGCRefMapTable != NULL)
        m_pGCRefMapTable->~ZapGCRefMapTable();
}

DWORD ZapImportSectionSignatures::GetSize()
{
    return m_pImportSection->GetNodeCount() * sizeof(DWORD);
}

void ZapImportSectionSignatures::Save(ZapWriter * pZapWriter)
{
    COUNT_T nCount = m_pImportSection->GetNodeCount();
    for (COUNT_T i = 0; i < nCount; i++)
    {
        ZapNode * pNode = m_pImportSection->GetNode(i);
        DWORD dwRVA = ((ZapImport *)pNode)->GetBlob()->GetRVA();
        pZapWriter->Write(&dwRVA, sizeof(dwRVA));
    }
}

// ======================================================================================
//
// Special lazy imports for lazily resolved method calls
//

//
// External method thunk is a patchable thunk used for cross-module direct calls
//
class ZapExternalMethodThunk : public ZapImport
{
public:
    ZapExternalMethodThunk()
    {
    }

    CORINFO_METHOD_HANDLE GetMethod()
    {
        return (CORINFO_METHOD_HANDLE)GetHandle();
    }

    virtual DWORD GetSize()
    {
        return sizeof(CORCOMPILE_EXTERNAL_METHOD_THUNK);
    }

    virtual ZapNodeType GetType()
    {
        return ZapNodeType_ExternalMethodThunk;
    }

    virtual void Save(ZapWriter * pZapWriter);
};

void ZapExternalMethodThunk::Save(ZapWriter * pZapWriter)
{
    ZapImage *             pImage  = ZapImage::GetImage(pZapWriter);
    ZapNode *              helper  = pImage->GetHelperThunk(CORINFO_HELP_EE_EXTERNAL_FIXUP);

    CORCOMPILE_EXTERNAL_METHOD_THUNK thunk;
    memset(&thunk, DEFAULT_CODE_BUFFER_INIT, sizeof(thunk));
#if defined(_TARGET_X86_) || defined(_TARGET_AMD64_)
    thunk.callJmp[0]  = 0xE8;  // call rel32
    pImage->WriteReloc(&thunk, 1, helper, 0, IMAGE_REL_BASED_REL32);
    thunk.precodeType = _PRECODE_EXTERNAL_METHOD_THUNK;
#elif defined(_TARGET_ARM_) 
    // Setup the call to ExternalMethodFixupStub
    //
    // mov r12, pc
    //
    // Per ARM architecture reference manual section A2.3,
    // reading the value of PC register will read the address
    // of the current instruction plus 4. In this case,
    // R12 will containing the address of "F004" below once
    // the "mov" is executed.
    //
    // Since this is 4 bytes ahead of the start of the thunk,
    // the assembly helper we will call into will adjust this
    // so that we point to the start of the thunk correctly.
    thunk.m_rgCode[0] = 0x46fc;

    // ldr pc, [pc, #4]
    thunk.m_rgCode[1] = 0xf8df;
    thunk.m_rgCode[2] = 0xf004;

    // Setup the initial target to be our assembly helper.
    pImage->WriteReloc(&thunk, offsetof(CORCOMPILE_EXTERNAL_METHOD_THUNK, m_pTarget), helper, 0, IMAGE_REL_BASED_PTR);
#elif defined(_TARGET_ARM64_)
    
    thunk.m_rgCode[0] = 0x1000000C; //adr       x12, #0
    thunk.m_rgCode[1] = 0xF940098A; //ldr       x10, [x12, #16]
    thunk.m_rgCode[2] = 0xD61F0140; //br        x10
        
    pImage->WriteReloc(&thunk, offsetof(CORCOMPILE_EXTERNAL_METHOD_THUNK, m_pTarget), helper, 0, IMAGE_REL_BASED_PTR);
#else
    PORTABILITY_ASSERT("ZapExternalMethodThunk::Save");

#endif

    pZapWriter->Write(&thunk,  sizeof(thunk));
    _ASSERTE(sizeof(thunk) == GetSize());
}

void ZapImportSectionSignatures::PlaceExternalMethodThunk(ZapImport * pImport)
{
    ZapExternalMethodThunk * pThunk = (ZapExternalMethodThunk *)pImport;

    if (m_pImportSection->GetNodeCount() == 0)
    {
        m_dwIndex = m_pImage->GetImportSectionsTable()->Append(CORCOMPILE_IMPORT_TYPE_EXTERNAL_METHOD, CORCOMPILE_IMPORT_FLAGS_CODE,
            sizeof(CORCOMPILE_EXTERNAL_METHOD_THUNK), m_pImportSection, this, m_pGCRefMapTable);

        // Make sure the helper created
        m_pImage->GetHelperThunk(CORINFO_HELP_EE_EXTERNAL_FIXUP);
    }

    // Add entry to both the the cell and data sections
    m_pImportSection->Place(pThunk);

    //m_pImage->GetImportTable()->PlaceImportBlob(pThunk);

    m_pGCRefMapTable->Append(pThunk->GetMethod());
}

//
// Stub dispatch cell is lazily initialized indirection used for virtual stub dispatch
//
class ZapStubDispatchCell : public ZapImport
{
    ZapNode * m_pDelayLoadHelper;

public:
    void SetDelayLoadHelper(ZapNode * pDelayLoadHelper)
    {
        _ASSERTE(m_pDelayLoadHelper == NULL);
        m_pDelayLoadHelper = pDelayLoadHelper;
    }

    CORINFO_METHOD_HANDLE GetMethod()
    {
        return (CORINFO_METHOD_HANDLE)GetHandle();
    }

    CORINFO_CLASS_HANDLE GetClass()
    {
        return (CORINFO_CLASS_HANDLE)GetHandle2();
    }

    virtual DWORD GetSize()
    {
        return TARGET_POINTER_SIZE;
    }

    virtual UINT GetAlignment()
    {
        return TARGET_POINTER_SIZE;
    }

    virtual ZapNodeType GetType()
    {
        return ZapNodeType_StubDispatchCell;
    }

    virtual void Save(ZapWriter * pZapWriter)
    {
        ZapImage * pImage = ZapImage::GetImage(pZapWriter);

        TARGET_POINTER_TYPE cell;
        pImage->WriteReloc(&cell, 0, m_pDelayLoadHelper, 0, IMAGE_REL_BASED_PTR);
        pZapWriter->Write(&cell, sizeof(cell));
    }
};

// ======================================================================================
//
// GCRefMapTable is used to encode for GC references locations for lazily resolved calls
//

void ZapGCRefMapTable::Append(CORINFO_METHOD_HANDLE handle)
{
    m_pImage->GetCompileInfo()->GetCallRefMap(handle, &m_GCRefMapBuilder);
    m_nCount++;
}

DWORD ZapGCRefMapTable::GetSize()
{
    if (m_nCount == 0) return 0;

    COUNT_T nLookupEntries = (1 + m_nCount / GCREFMAP_LOOKUP_STRIDE);

    return (nLookupEntries * sizeof(DWORD)) + m_GCRefMapBuilder.GetBlobLength();
}

void ZapGCRefMapTable::Save(ZapWriter * pZapWriter)
{
    if (m_nCount == 0) return;

    COUNT_T nLookupEntries = (1 + m_nCount / GCREFMAP_LOOKUP_STRIDE);

    DWORD dwBlobLength;
    BYTE * pBlob = (BYTE *)m_GCRefMapBuilder.GetBlob(&dwBlobLength);

    DWORD pos = 0;
    COUNT_T iLookupEntry = 0;
    for (;;)
    {
        DWORD relOfs = (nLookupEntries * sizeof(DWORD)) + pos;
        pZapWriter->Write(&relOfs, sizeof(relOfs));
        iLookupEntry++;

        if (iLookupEntry >= nLookupEntries)
            break;
        
        for (int i = 0; i < GCREFMAP_LOOKUP_STRIDE; i++)
        {
            while ((*(pBlob + pos) & 0x80) != 0)
                pos++;
            pos++;

            _ASSERTE(pos <= dwBlobLength);
        }
    }

    pZapWriter->Write(pBlob, dwBlobLength);
}

class ZapClassHandleImport : public ZapImport
{
public:
    virtual ZapNodeType GetType()
    {
        return ZapNodeType_Import_ClassHandle;
    }

    virtual ZapImportSectionType ComputePlacement(ZapImage * pImage, BOOL * pfIsEager, BOOL * pfNeedsSignature)
    {
        ZapImport::ComputePlacement(pImage, pfIsEager, pfNeedsSignature);

        if (IsReadyToRunCompilation())
            return ZapImportSectionType_Handle;

        CORINFO_CLASS_HANDLE handle = (CORINFO_CLASS_HANDLE)GetHandle();
        if (pImage->m_pPreloader->CanEmbedClassHandle(handle))
        {
            // We may have entries pointing to our module that exist in the handle table to trigger restore.
            if (pImage->GetCompileInfo()->GetLoaderModuleForEmbeddableType(handle) == pImage->GetModuleHandle())
            {
                *pfNeedsSignature = FALSE;
            }
            else
            {
                *pfIsEager = TRUE;
            }
         }

        return ZapImportSectionType_TypeHandle;
    }

    virtual void Save(ZapWriter * pZapWriter)
    {
        ZapImage * pImage = ZapImage::GetImage(pZapWriter);

        // We may have entries pointing to our module that exist in the handle table to trigger restore.
        if (!HasBlob())
        {
            PVOID cell;
            ZapNode handle(pImage->m_pPreloader->MapClassHandle((CORINFO_CLASS_HANDLE)GetHandle()));
            pImage->WriteReloc(&cell, 0, &handle, 0, IMAGE_REL_BASED_PTR);
            pZapWriter->Write(&cell, sizeof(cell));
        }
        else
        {
            ZapImport::Save(pZapWriter);
        }
    }
};
