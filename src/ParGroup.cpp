#include "stdafx.h"

#include "main.hpp"
#include "resource.hpp"
#include "iparamm2.h"
#include "modstack.h"

#define PARGROUP_CLASS_ID Class_ID(0x4b9331f5, 0x75c944d3);

#define PBLOCK_REF_NO 0

class ParGroupMod : public Modifier
{
    public:
        IParamBlock2 *m_PBlock;
        std::wstring m_ExportFile;
        std::wstring m_Anim;
        std::wstring m_Rename;

    public:
        ParGroupMod();

        // From Animatable
        void DeleteThis() override { delete this; }
        void GetClassName(MSTR &s, bool localized) const override { s = _M("EG.Exp.Group"); }
        Class_ID ClassID() override { return PARGROUP_CLASS_ID; }
        RefTargetHandle Clone(RemapDir &remap = DefaultRemapDir()) override;
        const MCHAR *GetObjectName(bool localized) const override { return _M("EG.Exp.Group"); }
        void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) override;
        void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) override;

        // From modifier
        ChannelMask ChannelsUsed() override { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_TEXMAP|PART_VERTCOLOR; }
        ChannelMask ChannelsChanged() override { return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
        Class_ID InputType() override { return defObjectClassID; }
        void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) override {}
        Interval LocalValidity(TimeValue t) override { return FOREVER; }
        MSTR GetName(bool localized) const override { return _M("EG.Exp.Group"); };

        // From BaseObject
        CreateMouseCallBack *GetCreateMouseCallBack() override { return nullptr; }

        int NumRefs() override { return 1; }
        RefTargetHandle GetReference(int i) override { if (i == 0) return m_PBlock; else return nullptr; }
        void SetReference(int i, RefTargetHandle rtarg) override { if (i == PBLOCK_REF_NO) m_PBlock = (IParamBlock2 *)rtarg; }

        int NumSubs() override { return 0; }
        Animatable *SubAnim(int i) override { return nullptr; }
        MSTR SubAnimName(int i, bool localized) override { return _M(""); }

        int	NumParamBlocks() override { return 1; }
        IParamBlock2 *GetParamBlock(int i) override { if (i == 0) return m_PBlock; else return nullptr; }
        IParamBlock2 *GetParamBlockByID(BlockID id) override { if (m_PBlock->ID() == id) return m_PBlock; else return nullptr; }

        RefResult NotifyRefChanged(const Interval &changeInt, RefTargetHandle hTarget, PartID &partID,
            RefMessage message, BOOL propagate) override { return REF_SUCCEED; }

        IOResult Load(ILoad *iload) override;
        IOResult Save(ISave *isave) override;

        void SetExportFile(const MCHAR *file);
        void UpdateCaption(void);
};

class ParGroupModClassDesc : public ClassDesc2
{
    public:
        int IsPublic() override { return 1; }
        void *Create(BOOL loading = FALSE) override { return new ParGroupMod; }
        const MCHAR *ClassName() override { return _M("EG.Exp.Group"); }
        SClass_ID SuperClassID() override { return OSM_CLASS_ID; }
        Class_ID ClassID() override { return PARGROUP_CLASS_ID; }
        const MCHAR *Category() override { return _M("EG"); }
        const MCHAR *NonLocalizedClassName() override { return _M("EG.Exp.Group"); }

        const MCHAR *InternalName() override { return _M("EG.Exp.Group"); }
        HINSTANCE HInstance() override { return hInstance; }
};

static ParGroupModClassDesc g_ParGroupModClassDesc;
extern ClassDesc *GetParGroupModClassDesc() { return &g_ParGroupModClassDesc; }

enum
{
    egpb_Export,
    egpb_Name,
    egpb_File,
    egpb_TextureDefault,
    egpb_EdgeType,
    egpb_EdgeFactor
};

class PEFG_Accessor : public PBAccessor
{
    void Set(PB2Value &v, ReferenceMaker *owner, ParamID id, int tabIndex, TimeValue t) override
    {
        ParGroupMod *map = (ParGroupMod *)owner;
        switch (id)
        {
            case egpb_File:
                map->SetExportFile(v.s);
                break;
            default:
                break;
        }
    }
};
PEFG_Accessor pefg_accessor;

static INT_PTR CALLBACK DlgExt(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK ExportDlgProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ParGroupDlgProc : public ParamMap2UserDlgProc
{
    public:
        ParGroupMod *ob;

        ParGroupDlgProc(ParGroupMod *o) { ob = o; }
        INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
        {
            if (msg == WM_COMMAND && LOWORD(wParam) == IDC_PARGROUP_MORE)
            {
                DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PARGROUP_EXT), GetCOREInterface()->GetMAXHWnd(), DlgExt, (LPARAM)ob);
                return TRUE;
            }
            else if (msg == WM_COMMAND && LOWORD(wParam) == IDC_PARGROUP_EXPORTBEGIN)
            {
                DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EGEXPORT_DLG2), GetCOREInterface()->GetMAXHWnd(), ExportDlgProc2, (LPARAM)ob);
                return TRUE;
            }
            return FALSE;
        }
        void DeleteThis() override { delete this; }
};

static ParamBlockDesc2 descP (0, _M("Parameters"), 0, &g_ParGroupModClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI,
    PBLOCK_REF_NO,  // auto_construct_block_refno
    IDD_PARGROUP, IDS_PARGROUP_PAR, 0, 0, NULL,  // auto_ui_parammap_specs

    egpb_Export, "Export", TYPE_INT, 0, IDS_PARGROUP_EXPORT,
        p_ui, TYPE_SINGLECHEKBOX, IDC_PARGROUP_EXPORT,
        p_end,

    egpb_Name, "Name", TYPE_STRING, 0, IDS_PARGROUP_NAME,
        p_ui, TYPE_EDITBOX, IDC_PARGROUP_NAME,
        p_end,

    egpb_File, "File", TYPE_FILENAME, 0, IDS_PARGROUP_FILE,
        p_ui, TYPE_FILESAVEBUTTON, IDC_PARGROUP_FILE,
        p_file_types, IDS_PARGROUP_FILETYPE,
        p_caption, IDS_PARGROUP_FILENAME,
        p_accessor, &pefg_accessor,
        p_end,

    egpb_TextureDefault, "TextureDefault", TYPE_INT, 0, IDS_PARGROUP_TEXTUREDEFAULT,
        p_default, 1,
        p_ui, TYPE_SINGLECHEKBOX, IDC_PARGROUP_TEXTUREDEFAULT,
        p_end,

    egpb_EdgeType, "EdgeType", TYPE_INT, 0, IDS_PARGROUP_EDGETYPE,
        p_default, 0,
        p_ui, TYPE_RADIO, 3, IDC_PARGROUP_EDGEOFF, IDC_PARGROUP_EDGEALL, IDC_PARGROUP_EDGEOPTIMIZE,
        p_end,

    egpb_EdgeFactor, "EdgeFactor", TYPE_FLOAT, 0, IDS_PARGROUP_EDGEFACTOR,
        p_default, 0.1f,
        p_range, 0.0f, 180.0f,
        p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARGROUP_EDGEFACTOR_EDIT, IDC_PARGROUP_EDITFACTOR_SPINNER, 0.01f,
        p_end,

    p_end
);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ParGroupMod::ParGroupMod() : m_ExportFile(), m_Anim(), m_Rename()
{
    m_PBlock = nullptr;

    GetParGroupModClassDesc()->MakeAutoParamBlocks(this);
    m_PBlock->SetValue(egpb_Export, 0, 1, 0);
    m_PBlock->SetValue(egpb_Name, 0, _M(""), 0);
    // m_PBlock->SetValue(egpb_File, 0, _M(""), 0);

    ExportGroupGetRename(m_Rename);
}

RefTargetHandle ParGroupMod::Clone(RemapDir &remap)
{
    ParGroupMod *mod = new ParGroupMod();
    mod->ReplaceReference(PBLOCK_REF_NO, m_PBlock->Clone(remap));
    mod->m_ExportFile = m_ExportFile;
    mod->m_Anim = m_Anim;
    mod->m_Rename = m_Rename;
    BaseClone(this, mod, remap);
    return mod;
}


void ParGroupMod::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
    static MCHAR sr[MAX_PATH];
    if (m_ExportFile.empty())
        sr[0] = 0;
    else
    {
        MSTR mstr = m_ExportFile.c_str();
        CopyMemory(sr, mstr.data(), mstr.ByteCount() + sizeof(MCHAR));
    }
    descP.ParamOption(egpb_File, p_init_file, sr);

    g_ParGroupModClassDesc.BeginEditParams(ip, this, flags, prev);

    g_ParGroupModClassDesc.SetUserDlgProc(&descP, new ParGroupDlgProc(this));

    UpdateCaption();
}

void ParGroupMod::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
    g_ParGroupModClassDesc.EndEditParams(ip, this, flags, next);
}

#define FILEEXPORT_CHUNK_1 0x1000
#define FILEEXPORT_CHUNK_2 0x1001
#define FILEEXPORT_CHUNK_3 0x1002

IOResult ParGroupMod::Load(ILoad *iload)
{
    wchar_t *buf;
    IOResult res = Modifier::Load(iload);
    if (res != IO_OK)
        return res;

    while (IO_OK == (res=iload->OpenChunk()))
    {
        switch (iload->CurChunkID())
        {
            case FILEEXPORT_CHUNK_1:
            {
                res = iload->ReadWStringChunk(&buf);
                if (res != IO_OK)
                    break;
                m_ExportFile = buf;
                break;
            }

            case FILEEXPORT_CHUNK_2:
            {
                res = iload->ReadWStringChunk(&buf);
                if (res != IO_OK)
                    break;
                m_Anim = buf;
                break;
            }

            case FILEEXPORT_CHUNK_3:
            {
                res = iload->ReadWStringChunk(&buf);
                if (res!=IO_OK)
                    break;
                m_Rename = buf;
                break;
            };
        }
        iload->CloseChunk();
        if (res != IO_OK)
            return res;
    }

    return IO_OK;
}

IOResult ParGroupMod::Save(ISave *isave)
{
    IOResult res = Modifier::Save(isave);
    if (res != IO_OK)
        return res;

    isave->BeginChunk(FILEEXPORT_CHUNK_1);
    if(m_ExportFile.empty())
        isave->WriteWString(L"");
    else
        isave->WriteWString(m_ExportFile.c_str());
    isave->EndChunk();

    isave->BeginChunk(FILEEXPORT_CHUNK_2);
    if(m_Anim.empty())
        isave->WriteWString(L"");
    else
        isave->WriteWString(m_Anim.c_str());
    isave->EndChunk();

    isave->BeginChunk(FILEEXPORT_CHUNK_3);
    if(m_Rename.empty())
        isave->WriteWString(L"");
    else
        isave->WriteWString(m_Rename.c_str());
    isave->EndChunk();

    return IO_OK;
}

void ParGroupMod::SetExportFile(const MCHAR *file)
{
    m_ExportFile = file;

    descP.ParamOption(egpb_File, p_init_file, file);

    UpdateCaption();
}

void ParGroupMod::UpdateCaption()
{
    IParamMap2 *map = m_PBlock->GetMap();
    if (map)
    {
        MSTR p, f, e, nameStr;

        nameStr = m_ExportFile.c_str();
        SplitFilename(nameStr, &p, &f, &e);
        nameStr = f + e;
        map->SetText(egpb_File, nameStr.data());
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern void ExportGroupSetRename(const std::wstring &value, INode *node)
{
    if (node == nullptr)
        node = GetCOREInterface()->GetRootNode();

    Object *obj = node->GetObjectRef();
    if (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
    {
        IDerivedObject *dobj = (IDerivedObject *)obj;
        int cnt = dobj->NumModifiers();
        for (int i = 0; i < cnt; i++)
        {
            Modifier *mod = dobj->GetModifier(i);
            if (!mod)
                continue;

            int idok = mod->ClassID() == PARGROUP_CLASS_ID;
            if (!idok)
                continue;

            ((ParGroupMod *)mod)->m_Rename = value;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++) {
        ExportGroupSetRename(value, node->GetChildNode(c));
    }
}

extern void ExportGroupGetRename(std::wstring &value, INode *node)
{
    if (node == nullptr)
        node = GetCOREInterface()->GetRootNode();

    Object *obj = node->GetObjectRef();
    if (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
    {
        IDerivedObject *dobj = (IDerivedObject *)obj;
        int cnt = dobj->NumModifiers();
        for (int i = 0; i < cnt; i++)
        {
            Modifier *mod = dobj->GetModifier(i);
            if (!mod)
                continue;
            
            int idok = mod->ClassID() == PARGROUP_CLASS_ID;
            if (!idok)
                continue;

            value = ((ParGroupMod *)mod)->m_Rename;
            if (!value.empty())
                return;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++)
    {
        ExportGroupGetRename(value, node->GetChildNode(c));
        if (!value.empty())
            return;
    }
}

extern int ExportGroupCount(INode *node)
{
    int r = 0;

    if (node == nullptr)
        node = GetCOREInterface()->GetRootNode();

    Object *obj = node->GetObjectRef();
    if (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
    {
        IDerivedObject *dobj = (IDerivedObject *)obj;
        int cnt = dobj->NumModifiers();
        for (int i=0; i < cnt; i++)
        {
            Modifier *mod = dobj->GetModifier(i);
            if (!mod)
                continue;
            
            int idok = mod->ClassID() == PARGROUP_CLASS_ID;
            if (!idok)
                continue;

            r++;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++)
        r += ExportGroupCount(node->GetChildNode(c));

    return r;
}

extern bool ExportGroupGetName(std::wstring &tstr, int no, int &nocur, INode *node)
{
    if (node == nullptr)
    {
        node = GetCOREInterface()->GetRootNode();
        nocur = 0;
    }

    Object *obj = node->GetObjectRef();
    if (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
    {
        IDerivedObject *dobj = (IDerivedObject *)obj;
        int cnt = dobj->NumModifiers();
        for (int i = 0; i < cnt; i++)
        {
            Modifier *mod = dobj->GetModifier(i);
            if (!mod)
                continue;
            
            int idok = mod->ClassID() == PARGROUP_CLASS_ID;
            if (!idok)
                continue;

            if (no == nocur)
            {
                const MCHAR *ach;
                ((ParGroupMod *)mod)->m_PBlock->GetValue(egpb_Name, 0, ach, FOREVER);
                tstr = ach;
                return true;
            }
            else
                nocur++;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++)
        if (ExportGroupGetName(tstr, no, nocur, node->GetChildNode(c)))
            return true;

    return false;
}

extern bool ExportGroupGetProp(int no, int &nocur, INode *node,
    std::wstring &name, int &exp, std::wstring &file, bool &textureDefault, int &edgetype, double &edgefactor, std::wstring &anim)
{
    if (node == nullptr)
    {
        node = GetCOREInterface()->GetRootNode();
        nocur = 0;
    }

    Object *obj = node->GetObjectRef();
    if (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
    {
        IDerivedObject *dobj = (IDerivedObject *)obj;
        int cnt = dobj->NumModifiers();
        for (int i=0; i<cnt; i++)
        {
            Modifier *mod = dobj->GetModifier(i);
            if (!mod)
                continue;
            
            int idok = mod->ClassID() == PARGROUP_CLASS_ID;
            if (!idok)
                continue;

            if (no == nocur)
            {
                const MCHAR *ach;

                ((ParGroupMod *)mod)->m_PBlock->GetValue(egpb_Name, 0, ach, FOREVER);
                name = ach;

                ((ParGroupMod *)mod)->m_PBlock->GetValue(egpb_Export, 0, exp, FOREVER);

                file = ((ParGroupMod *)mod)->m_ExportFile;

                int iv;
                ((ParGroupMod *)mod)->m_PBlock->GetValue(egpb_TextureDefault, 0, iv, FOREVER);
                textureDefault = iv != 0;

                ((ParGroupMod *)mod)->m_PBlock->GetValue(egpb_EdgeType, 0, edgetype, FOREVER);

                float fv;
                ((ParGroupMod *)mod)->m_PBlock->GetValue(egpb_EdgeFactor, 0, fv, FOREVER);
                edgefactor = fv;

                anim = ((ParGroupMod *)mod)->m_Anim;

                return true;
            }
            else
                nocur++;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++)
        if (ExportGroupGetProp(no, nocur, node->GetChildNode(c), name, exp, file, textureDefault, edgetype, edgefactor, anim))
            return true;
    
    return false;
}

static INT_PTR CALLBACK DlgExt(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HFONT m_font = 0;
    static ParGroupMod *m_Group = nullptr;
    static bool m_InitOk = false;

    switch (msg)
    {
        case WM_INITDIALOG:
        {
            utils::ParamParser tstr;

            m_InitOk = false;
            m_Group = (ParGroupMod *)lParam;

            if (!m_font)
                m_font = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, 0, 0, 0,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, _T("Courier New"));

            SendMessage(GetDlgItem(hWnd, IDC_PARGROUP_EXT_ANIM), WM_SETFONT, (WPARAM)m_font, 0);
            SendMessage(GetDlgItem(hWnd, IDC_PARGROUP_EXT_RENAME), WM_SETFONT, (WPARAM)m_font, 0);

            SETTEXTEX ste;
            ste.codepage = 1200;
            ste.flags = ST_DEFAULT;

            if (!m_Group->m_Anim.empty())
                SendMessage(GetDlgItem(hWnd, IDC_PARGROUP_EXT_ANIM), EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)m_Group->m_Anim.data());
            ExportGroupGetRename(tstr);
            if (!tstr.empty())
                SendMessage(GetDlgItem(hWnd, IDC_PARGROUP_EXT_RENAME), EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)tstr.data());

            tstr = Base::Reg_GetString(0, L"HKCU\\Software\\Elemental Games\\MaxExp", L"DlgGroupPos", L"");
            if (tstr.GetCountPar(L",") >= 2)
            {
                int x = tstr.GetStrPar(0,L",").GetInt();
                int y = tstr.GetStrPar(1,L",").GetInt();

                SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
            }

            m_InitOk = true;

            return TRUE;
        }

        case WM_DESTROY:
        {
            m_InitOk = false;
            m_Group = nullptr;
            if (m_font)
            {
                DeleteObject(m_font);
                m_font=0;
            }
            return TRUE;
        }

        case WM_MOVE: {
            if(m_InitOk) {
                Base::Reg_SetString(0, L"HKCU\\Software\\Elemental Games\\MaxExp", L"DlgGroupPos",
                    utils::format(L"%i,%i", (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)).c_str());
            }
            break;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    HWND cw = GetDlgItem(hWnd, IDC_PARGROUP_EXT_ANIM);
                    GETTEXTLENGTHEX gtl; gtl.flags = GTL_DEFAULT|GTL_USECRLF|GTL_NUMBYTES; gtl.codepage = 1200;
                    int len = SendMessage(cw, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
                    if (len != E_INVALIDARG)
                    {
                        if ((len > 0) && !(len & 1))
                        {
                            GETTEXTEX gte;
                            gte.cb = len + 2;
                            gte.codepage = 1200;
                            gte.flags = GT_USECRLF;
                            gte.lpDefaultChar = NULL;
                            gte.lpUsedDefChar = NULL;

                            std::wstring str;
                            str.resize(len >> 1);
                            SendMessage(cw, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)str.data());
                            m_Group->m_Anim = str;
                        }
                        else
                            m_Group->m_Anim.clear();
                    }

                    std::wstring str;
                    cw = GetDlgItem(hWnd, IDC_PARGROUP_EXT_RENAME);
                    len = SendMessage(cw, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
                    if (len != E_INVALIDARG)
                    {
                        if ((len > 0) && !(len & 1))
                        {
                            GETTEXTEX gte;
                            gte.cb = len + 2;
                            gte.codepage = 1200;
                            gte.flags = GT_USECRLF;
                            gte.lpDefaultChar = NULL;
                            gte.lpUsedDefChar = NULL;

                            str.resize(len >> 1);
                            SendMessage(cw, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)str.data());
                        }
                    }
                    ExportGroupSetRename(str);

                    EndDialog(hWnd, 1);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hWnd, 0);
                    return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}       

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static INT_PTR CALLBACK ExportDlgProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            EGExp2 exp;

            exp.ExportSimpleGroup();

            if (exp.m_Log.Len() > 0)
            {
                exp.m_Log.Add<WORD>(0);

                SETTEXTEX ste;
                ste.codepage = 1200;
                ste.flags = ST_DEFAULT;

                HWND cw = GetDlgItem(hWnd, IDC_LOG);
                SendMessage(cw, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)exp.m_Log.Get());
            }
            return TRUE;
        }
        case WM_CLOSE:
        {
            EndDialog(hWnd, 0);
            return TRUE;
        }
        default:
            return FALSE;
    }
    return FALSE;
}
