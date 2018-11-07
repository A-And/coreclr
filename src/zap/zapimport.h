// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// ZapImport.h
//

//
// Import is soft bound references to elements outside the current module
// 
// ======================================================================================

#ifndef __ZAPIMPORT_H__
#define __ZAPIMPORT_H__

class ZapGCRefMapTable;
class NibbleWriter;

//---------------------------------------------------------------------------------------
//
// ZapImport is the import cell itself
//
// Every import cell is uniquely identified by its ZapNodeType and two handles 
// (the second handle is optional and is often NULL)
//
// Actual implementations inherits from this abstract base class.
//
class ZapImport : public ZapNode
{
    COUNT_T m_index;
    DWORD m_offset;

    PVOID m_handle;
    PVOID m_handle2;

    ZapBlob * m_pBlob;

public:
    void SetHandle(PVOID handle)
    {
        _ASSERTE(m_handle == NULL);
        m_handle = handle;
    }

    void SetHandle2(PVOID handle2)
    {
        _ASSERTE(m_handle2 == NULL);
        m_handle2 = handle2;
    }

    PVOID GetHandle()
    {
        return m_handle;
    }

    PVOID GetHandle2()
    {
        return m_handle2;
    }

    void SetBlob(ZapBlob * pBlob)
    {
        _ASSERTE(m_pBlob == NULL);
        m_pBlob = pBlob;
    }

    ZapBlob * GetBlob()
    {
        _ASSERTE(m_pBlob != NULL);
        return m_pBlob;
    }

    BOOL HasBlob()
    {
        return m_pBlob != NULL;
    }

    virtual ZapImportSectionType ComputePlacement(ZapImage * pImage, BOOL * pfIsEager, BOOL * pfNeedsSignature)
    {
        *pfIsEager = FALSE;
        *pfNeedsSignature = TRUE;
        return ZapImportSectionType_Handle;
    }

    //// All subtypes have to override
    //virtual void EncodeSignature(ZapImportTable * pTable, SigBuilder * pSigBuilder) = 0;

    virtual DWORD GetSize()
    {
        return TARGET_POINTER_SIZE;
    }

    virtual UINT GetAlignment()
    {
        return TARGET_POINTER_SIZE;
    }

    virtual void Save(ZapWriter * pZapWriter);
    
    //
    // Offset of the fixup cell within its section
    //

    void SetSectionIndexAndOffset(COUNT_T index, DWORD offset)
    {
        m_index = index;
        m_offset = offset;
    }

    DWORD GetSectionIndex()
    {
        return m_index;
    }

    DWORD GetOffset()
    {
        return m_offset;
    }
};

//---------------------------------------------------------------------------------------
//
// ZapGenericSignature is signature of generic dictionary entry.
//
class ZapGenericSignature : public ZapBlob
{
public:
    ZapGenericSignature(SIZE_T cbSize)
        : ZapBlob(cbSize)
    {
    }

    virtual ZapNodeType GetType()
    {
        return ZapNodeType_GenericSignature;
    }
};

//
// CORCOMPILE_CODE_IMPORT_SECTION
//
class ZapImportSectionsTable : public ZapNode
{
    struct ImportSection
    {
        ZapVirtualSection * m_pSection;
        ZapNode * m_pSignatures;
        ZapNode * m_pAuxiliaryData;
        USHORT    m_Flags;
        BYTE      m_Type;
        BYTE      m_EntrySize;
    };

    SArray<ImportSection> m_ImportSectionsTable;

public:
    ZapImportSectionsTable(ZapImage * pImage)
    {
    }

    COUNT_T Append(BYTE Type, USHORT Flags, BYTE EntrySize, ZapVirtualSection * pSection, ZapNode * pSignatures = NULL, ZapNode * pAuxiliaryData = NULL);

    virtual UINT GetAlignment()
    {
        return sizeof(DWORD);
    }

    virtual DWORD GetSize();

    virtual ZapNodeType GetType()
    {
        return ZapNodeType_ImportSectionsTable;
    }

    virtual void Save(ZapWriter * pZapWriter);
};

//
// ZapImportSectionSignatures contains an array of signature RVAs for given import section.
//
class ZapImportSectionSignatures : public ZapNode
{
    ZapVirtualSection * m_pImportSection;
    ZapGCRefMapTable * m_pGCRefMapTable;

    DWORD m_dwIndex;

    ZapImage * m_pImage;

public:
    ZapImportSectionSignatures(ZapImage * pImage, ZapVirtualSection * pImportSection, ZapVirtualSection * pGCSection = NULL);
    ~ZapImportSectionSignatures();

    void PlaceExternalMethodThunk(ZapImport * pImport);

    virtual DWORD GetSize();

    virtual UINT GetAlignment()
    {
        return sizeof(DWORD);
    }

    virtual ZapNodeType GetType()
    {
        return ZapNodeType_ImportSectionSignatures;
    }

    virtual void Save(ZapWriter * pZapWriter);
};

#include "gcrefmap.h"

class ZapGCRefMapTable : public ZapNode
{
    ZapImage * m_pImage;
    GCRefMapBuilder m_GCRefMapBuilder;
    COUNT_T m_nCount;

public:
    ZapGCRefMapTable(ZapImage * pImage)
        : m_pImage(pImage)
    {
    }

    void Append(CORINFO_METHOD_HANDLE handle);

    virtual DWORD GetSize();

    virtual UINT GetAlignment()
    {
        return sizeof(DWORD);
    }

    virtual ZapNodeType GetType()
    {
        return ZapNodeType_GCRefMapTable;
    }

    virtual void Save(ZapWriter * pZapWriter);
};

#endif // __ZAPIMPORT_H__
