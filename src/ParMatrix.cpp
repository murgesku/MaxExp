#include "stdafx.h"

#include "main.hpp"
#include "resource.hpp"
#include "iparamm2.h"
#include "modstack.h"

#define PARMATRIX_CLASS_ID Class_ID(0x31764fc2, 0x761b1e85);

#define PBLOCK_REF_NO 0

class ParMatrixMod : public Modifier
{
    public:
        IParamBlock2 *m_PBlock;
        std::wstring m_Group;

    public:
        ParMatrixMod();

        // From Animatable
        void DeleteThis() override { delete this; }
        void GetClassName(MSTR &s, bool localized) const override { s = _M("EG.Exp.Matrix"); }
        Class_ID ClassID() override { return PARMATRIX_CLASS_ID; }
        RefTargetHandle Clone(RemapDir &remap = DefaultRemapDir()) override;
        const MCHAR *GetObjectName(bool localized) const override { return _M("EG.Exp.Matrix"); }
        void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) override;
        void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) override;		

        // From modifier
        ChannelMask ChannelsUsed() override { return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_TEXMAP|PART_VERTCOLOR; }
        ChannelMask ChannelsChanged() override { return PART_GEOM|PART_TOPO|PART_SELECT|PART_TEXMAP|PART_VERTCOLOR; }
        Class_ID InputType() override { return defObjectClassID; }
        void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) override {}
        Interval LocalValidity(TimeValue t) override { return FOREVER; }
        MSTR GetName(bool localized) const override { return _M("EG.Exp.Matrix"); };

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
};

class ParMatrixModClassDesc : public ClassDesc2
{
    public:
        int IsPublic() override { return 1; }
        void *Create(BOOL loading = FALSE) override { return new ParMatrixMod; }
        const MCHAR *ClassName() override { return _M("EG.Exp.Matrix"); }
        SClass_ID SuperClassID() override { return OSM_CLASS_ID; }
        Class_ID ClassID() override { return PARMATRIX_CLASS_ID; }
        const MCHAR *Category() override { return _M("EG"); }
        const MCHAR *NonLocalizedClassName() override { return _M("EG.Exp.Matrix"); }

        const MCHAR *InternalName() override { return _M("EG.Exp.Matrix"); }
        HINSTANCE HInstance() override { return hInstance; }
};

static ParMatrixModClassDesc g_ParMatrixModClassDesc;
extern ClassDesc* GetParMatrixModClassDesc() { return &g_ParMatrixModClassDesc; }

enum
{
    egpb_Export,
    egpb_Group,
    egpb_Name,
    egpb_Id
};

class ParMatrixDlgProc : public ParamMap2UserDlgProc
{
    public:
        ParMatrixMod *ob;

        ParMatrixDlgProc(ParMatrixMod *o) { ob = o; }
        INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
        {
            switch(msg)
            {
                case WM_INITDIALOG:
                {
                    HWND win = GetDlgItem(hWnd, IDC_PAROBJ_GROUP);
                    SendMessage(win, CB_RESETCONTENT, 0, 0);
                    SendMessage(win, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T(""));
                    int cnt = ExportGroupCount();
                    int no;
                    for (int i = 0; i < cnt; i++)
                    {
                        std::wstring tstr;
                        if (ExportGroupGetName(tstr, i, no))
                            SendMessage(win, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)tstr.data());
                    }
                    SendMessage(win, CB_SETCURSEL, (WPARAM)-1, 0);
                    if (!ob->m_Group.empty())
                        SetWindowText(win, ob->m_Group.c_str());

                    return TRUE;
                }

                case WM_COMMAND:
                {
                    if (HIWORD(wParam) == CBN_SETFOCUS && LOWORD(wParam) == IDC_PAROBJ_GROUP)
                        DisableAccelerators();

                    else if (HIWORD(wParam) == CBN_KILLFOCUS && LOWORD(wParam) == IDC_PAROBJ_GROUP)
                        EnableAccelerators();

                    else if (HIWORD(wParam) == CBN_EDITCHANGE && LOWORD(wParam) == IDC_PAROBJ_GROUP)
                    {
                        HWND win = GetDlgItem(hWnd, IDC_PAROBJ_GROUP);
                        int cnt = SendMessage(win, WM_GETTEXTLENGTH, 0, 0);
                        std::wstring tstr;
                        if (cnt > 0)
                        {
                            tstr.resize(cnt);
                            SendMessage(win, WM_GETTEXT, cnt + 1, (LPARAM)tstr.data());
                        }
                        ob->m_Group = tstr;
                    }

                    else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PAROBJ_GROUP)
                    {
                        HWND win = GetDlgItem(hWnd, IDC_PAROBJ_GROUP);
                        int cur = SendMessage(win, CB_GETCURSEL, 0, 0);
                        int cnt = SendMessage(win, CB_GETLBTEXTLEN, cur, 0);
                        std::wstring tstr;
                        if (cnt > 0)
                        {
                            tstr.resize(cnt);
                            SendMessage(win, CB_GETLBTEXT, cur, (LPARAM)tstr.data());
                        }
                        ob->m_Group = tstr;
                    }

                    return TRUE;
                }
            }
            return FALSE;
        }
        void DeleteThis() override { delete this; }
};

static ParamBlockDesc2 descP (0, _M("Parameters"), 0, &g_ParMatrixModClassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI,
    PBLOCK_REF_NO,  // auto_construct_block_refno
    IDD_PARMATRIX, IDS_PARGROUP_PAR, 0, 0, NULL,  // auto_ui_parammap_specs

    egpb_Export, "Export", TYPE_INT, 0, IDS_PARGROUP_EXPORT,
        p_ui, TYPE_SINGLECHEKBOX, IDC_PAROBJ_EXPORT,
        p_end,

    egpb_Name, "Name", TYPE_STRING, 0, IDS_PARMATRIX_NAME,
        p_ui, TYPE_EDITBOX, IDC_PARMATRIX_NAME,
        p_end,

    egpb_Id, "MId", TYPE_INT, 0, IDS_PARMATRIX_ID,
        p_default, 0,
        p_range, 0, 2000000000,
        p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_PARMATRIX_ID_EDIT, IDC_PARMATRIX_ID_SPINNER, 1.0f,
        p_end,

    p_end
);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ParMatrixMod::ParMatrixMod() : m_Group()
{
    m_PBlock = nullptr;

    GetParMatrixModClassDesc()->MakeAutoParamBlocks(this);
    m_PBlock->SetValue(egpb_Export, 0, 1, 0);
}

RefTargetHandle ParMatrixMod::Clone(RemapDir &remap)
{
    ParMatrixMod *mod = new ParMatrixMod();
    mod->ReplaceReference(PBLOCK_REF_NO, m_PBlock->Clone(remap));
    mod->m_Group = m_Group;
    BaseClone(this, mod, remap);
    return mod;
}

void ParMatrixMod::BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev)
{
    g_ParMatrixModClassDesc.BeginEditParams(ip, this, flags, prev);
    g_ParMatrixModClassDesc.SetUserDlgProc(&descP, new ParMatrixDlgProc(this));
}

void ParMatrixMod::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
    g_ParMatrixModClassDesc.EndEditParams(ip, this, flags, next);
}

#define FILEEXPORT_CHUNK_1 0x1000

IOResult ParMatrixMod::Load(ILoad *iload)
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
                m_Group = buf;
                break;
            }
        }
        iload->CloseChunk();
        if (res != IO_OK)
            return res;
    }

    return IO_OK;
}

IOResult ParMatrixMod::Save(ISave *isave)
{
    IOResult res = Modifier::Save(isave);
    if (res != IO_OK)
        return res;

    isave->BeginChunk(FILEEXPORT_CHUNK_1);
    if (m_Group.empty())
        isave->WriteWString(L"");
    else
        isave->WriteWString(m_Group.c_str());
    isave->EndChunk();

    return IO_OK;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern bool ExportMatrixGetProp(INode *node, const std::wstring &group, int &id, std::wstring &name)
{
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

            int idok = mod->ClassID() == PARMATRIX_CLASS_ID;
            if (!idok)
                continue;

            int iv;
            const MCHAR *ach;

            if (!group.empty() && !((ParMatrixMod *)mod)->m_Group.empty() && (group!=((ParMatrixMod *)mod)->m_Group))
                continue;

            ((ParMatrixMod *)mod)->m_PBlock->GetValue(egpb_Export, 0, iv, FOREVER);
            if (!iv)
                continue;

            ((ParMatrixMod *)mod)->m_PBlock->GetValue(egpb_Id, 0, id, FOREVER);

            ((ParMatrixMod *)mod)->m_PBlock->GetValue(egpb_Name, 0, ach, FOREVER);
            name = ach;

            return true;
        }
    }

    return false;
}
