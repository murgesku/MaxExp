//************************************************************************** 
//* Export.cpp	- Ascii File Exporter
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* January 20, 1997 CCJ Initial coding
//*
//* This module contains the main export functions.
//*
//* Copyright (c) 1997, All Rights Reserved. 
//***************************************************************************

#include "stdafx.h"

#include "main.hpp"

void ConvertMatrix(SVOMatrix &out, Matrix3 &in)
{
    Point3 pt;

    pt=in.GetRow(0);
    out._11 = pt.x; out._12 = pt.y; out._13 = pt.z; out._14 = 0.0f;

    pt=in.GetRow(1);
    out._21 = pt.x; out._22 = pt.y; out._23 = pt.z; out._24 = 0.0f;

    pt=in.GetRow(2);
    out._31 = pt.x; out._32 = pt.y; out._33 = pt.z; out._34 = 0.0f;

    pt=in.GetRow(3);
    out._41 = pt.x; out._42 = pt.y; out._43 = pt.z; out._44 = 1.0f;
}

void ConvertMatrix(Matrix3 &out, SVOMatrix &in)
{
    Point3 pt;

    pt.x = in._11; pt.y = in._12; pt.z = in._13;
    out.SetRow(0, pt);

    pt.x = in._21; pt.y = in._22; pt.z = in._23;
    out.SetRow(1, pt);

    pt.x = in._31; pt.y = in._32; pt.z = in._33;
    out.SetRow(2, pt);

    pt.x = in._41; pt.y = in._42; pt.z = in._43;
    out.SetRow(3, pt);
}

void MulMatrix(SVOMatrix &des, SVOMatrix &a, SVOMatrix &b)
{
    for (int y = 0; y < 4; y++)
        for(int x = 0; x < 4; x++)
            des.m[x][y] = 0.0f;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                des.m[i][j] += a.m[k][j] * b.m[i][k];
}

void IdentityMatrix(SVOMatrix &des)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            des.m[i][j] = 0.0f;
        des.m[i][i] = 1.0f;
    }
}

void ScaleMatrix(SVOMatrix &des, Point3 &v)
{
    IdentityMatrix(des);
    des.m[0][0] = v.x;
    des.m[1][1] = v.y;
    des.m[2][2] = v.z;
}

void TransformVector(Point3 &des, Point3 &v, SVOMatrix &m)
{
    float hvec[4];

    for (int i = 0; i < 4; i++)
    {
        hvec[i] = 0.0f;
        for (int j = 0; j < 4; j++)
            if (j == 3)
                hvec[i] += m.m[j][i];
            else
                hvec[i] += v[j] * m.m[j][i];
    }

    des.x = hvec[0] / hvec[3];
    des.y = hvec[1] / hvec[3];
    des.z = hvec[2] / hvec[3];
}

void ViewMatrix(Point3 &from, Point3 &at, Point3 &world_up, SVOMatrix &m)
{
    IdentityMatrix(m);

    Point3 view_dir = at - from;
    view_dir = view_dir.Normalize();

    Point3 right = world_up ^ view_dir;
    Point3 up = view_dir ^ right;

    right = right.Normalize();
    up = up.Normalize();

    m._11 = right.x; m._21 = right.y; m._31 = right.z;
    m._12 = up.x; m._22 = up.y; m._32 = up.z;
    m._13 = view_dir.x; m._23 = view_dir.y; m._33 = view_dir.z;

    m._41 = -DotProd(right, from);
    m._42 = -DotProd(up, from);
    m._43 = -DotProd(view_dir, from);
}

void StrToFrameList(utils::ParamParser &str, Base::CBuf &buf)
{
    buf.Clear();
    int cnt = str.GetCountPar(L"[]");
    utils::ParamParser st, sf;
    for (int i = 0; i < cnt; i++)
    {
        st = utils::trim(str.GetStrPar(i, L"[]"));
        if (st.empty())
            continue;

        int cntp = st.GetCountPar(L",");
        if (cntp < 2)
            continue;
        int time = st.GetStrPar(0, L",").GetInt();
        for (int u = 1; u < cntp; u++)
        {
            sf = utils::trim(st.GetStrPar(u, L","));
            if (sf.empty())
                continue;

            int cp = sf.GetCountPar(L"-:");
            if (cp == 0)
                continue;
            else if(cp == 1)
            {
                buf.Add<DWORD>(sf.GetStrPar(0, L"-:").GetDword());
                buf.Add<DWORD>(time);
            }
            else if(cp == 2)
            {
                int ifrom = sf.GetStrPar(0, L"-:").GetInt();
                int ito = sf.GetStrPar(1, L"-:").GetInt();
                int k = ifrom;

                while (true)
                {
                    buf.Add<DWORD>(k);
                    buf.Add<DWORD>(time);

                    if (ifrom < ito)
                        k++;
                    else
                        k--;

                    if(k==ito)
                        break;
                }
            }
            else
            {
                int ifrom = sf.GetStrPar(0, L"-:").GetInt();
                int ito = sf.GetStrPar(1, L"-:").GetInt();
                int istep = sf.GetStrPar(2, L"-:").GetInt();
                if (istep < 1)
                    continue;

                int k = ifrom;
                while (true)
                {
                    buf.Add<DWORD>(k);
                    buf.Add<DWORD>(time);

                    if (ifrom < ito)
                    {
                        k += istep;
                        if (k > ito)
                            break;
                    }
                    else
                    {
                        k -= istep;
                        if (k < ito)
                            break;
                    }
                }
            }
        }
    }
    buf.Pointer(0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EGExp2::EGExp2() : m_FileName(), m_GroupName(), m_Log(), m_Rename(), m_Anim()
{
    ip = nullptr;

    m_MaterialNextNo = 0;

    m_TextureDefault = true;

    m_MatCenter.IdentityMatrix();
    m_Scale = Point3(1.0, 1.0, 1.0);

    m_MaxSizeX = 0;
    m_MaxSizeY = 0;
    m_MaxSizeZ = 0;
    
    m_EdgeExport = 0;
    m_EdgeExportFactor = cos(0.1 * pi / 180.0);

    ip = GetCOREInterface();
}

EGExp2::~EGExp2()
{
    GroupClear();
}

void EGExp2::GroupClear()
{
    m_Groups.clear();
}

void EGExp2::GroupDelete(CGroup *el)
{
    auto it = std::find_if(m_Groups.begin(), m_Groups.end(), [el](CGroup& g) { return &g == el; });
    if (it != m_Groups.end())
        m_Groups.erase(it);
}

CGroup *EGExp2::GroupFind(CTuple &p1)
{
    auto it = std::find_if(m_Groups.rbegin(), m_Groups.rend(),
        [&p1](CGroup& g) {
            return (g.m_IdCnt == 1 && g.m_Id[0] == p1);
        });

    if (it != m_Groups.rend())
    {
        m_Groups.splice(m_Groups.end(), m_Groups, (++it).base());
        return &m_Groups.back();
    }

    return nullptr;
}

CGroup *EGExp2::GroupFind(CTuple &p1, CTuple &p2)
{
    auto it = std::find_if(m_Groups.rbegin(), m_Groups.rend(),
        [&p1, &p2](CGroup& g) {
            return (g.m_IdCnt == 2 && g.m_Id[0] == p1 && g.m_Id[1] == p2);
        });

    if (it != m_Groups.rend())
    {
        m_Groups.splice(m_Groups.end(), m_Groups, (++it).base());
        return &m_Groups.back();
    }

    return nullptr;
}

CGroup *EGExp2::GroupFind(CTuple &p1, CTuple &p2, CTuple &p3)
{
    auto it = std::find_if(m_Groups.rbegin(), m_Groups.rend(),
        [&p1, &p2, &p3](CGroup& g) {
            return (g.m_IdCnt == 3 && g.m_Id[0] == p1 && g.m_Id[1] == p2 && g.m_Id[2] == p3);
        });

    if (it != m_Groups.rend())
    {
        m_Groups.splice(m_Groups.end(), m_Groups, (++it).base());
        return &m_Groups.back();
    }

    return nullptr;
}

CGroup *EGExp2::GroupAdd(CTuple &p1)
{
    CGroup& el = m_Groups.emplace_back();

    el.m_IdCnt = 1;
    el.m_Id[0] = p1;

    return &m_Groups.back();
}

CGroup *EGExp2::GroupAdd(CTuple &p1, CTuple &p2)
{
    CGroup &el = m_Groups.emplace_back();

    el.m_IdCnt = 2;
    el.m_Id[0] = p1;
    el.m_Id[1] = p2;

    return &m_Groups.back();
}

CGroup *EGExp2::GroupAdd(CTuple &p1, CTuple &p2, CTuple &p3)
{
    CGroup& el = m_Groups.emplace_back();

    el.m_IdCnt = 3;
    el.m_Id[0] = p1;
    el.m_Id[1] = p2;
    el.m_Id[2] = p3;

    return &m_Groups.back();
}

void EGExp2::GroupSortByNo()
{
    m_Groups.sort([](CGroup& first, CGroup& second) { return first.m_No < second.m_No; });
}

void EGExp2::Log(const std::wstring &str)
{
    m_Log.WStrNZ(str);
    m_Log.Add<DWORD>(0x000a000d);
}

int EGExp2::GroupMaterialFindAdd(Mtl *ml, int sm, int &twosided)
{
    CGroup *gr;
    if (ml != nullptr && ml->NumSubMtls() > 0)
    {
        Mtl *ml2 = ml->GetSubMtl(sm % ml->NumSubMtls());
        if (ml2 != nullptr)
            ml = ml2;
    }

    if (ml && ml->ClassID() == Class_ID(DMTL_CLASS_ID, 0) && ((StdMat *)ml)->GetTwoSided())
        twosided = 1;
    else
        twosided = 0;

    gr = GroupFind(CTuple(gt_Material, (unsigned long long)ml));
    if (gr != nullptr)
        return gr->m_No;
    else
    {
        gr = GroupAdd(CTuple(gt_Material, (unsigned long long)ml));
        gr->m_No = m_MaterialNextNo;
        m_MaterialNextNo++;
    }

    SMaterial outm;
    ZeroMemory(&outm, sizeof(SMaterial));

    Color cl;

    if (ml != nullptr)
    {
        cl = ml->GetAmbient();
        outm.ar = cl.r;
        outm.ag = cl.g;
        outm.ab = cl.b;

        cl = ml->GetDiffuse();
        outm.dr = cl.r;
        outm.dg = cl.g;
        outm.db = cl.b;

        cl = ml->GetSpecular();
        outm.sr = cl.r;
        outm.sg = cl.g;
        outm.sb = cl.b;

        //if (!(m_Set->ParCount(L"TextureDefault") && m_Set->Par(L"TextureDefault").GetInt()))
        if (!m_TextureDefault)
            if (ml->ClassID() == Class_ID(DMTL_CLASS_ID, 0) && ID_DI < ml->NumSubTexmaps() && ml->GetSubTexmap(ID_DI))
            {
                Texmap *tex = ml->GetSubTexmap(ID_DI);
                if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
                {
                    MSTR mapName = ((BitmapTex *)tex)->GetMapName();
                    utils::ParamParser namestr(mapName.data());
                    if (!namestr.empty())
                    {
                        namestr = RenameByCfg(namestr.GetStrPar(namestr.GetCountPar(L"\\/") - 1, L"\\/"));
                        //int cnt = namestr.GetCountPar(L".");
                        //if (cnt>1)
                        //    namestr = namestr.GetStrPar(0, cnt-2, L".");

                        if (namestr.length() > 31)
                            namestr.resize(31);
                        if (namestr.length() > 0)
                            CopyMemory(&(outm.tex_diffuse), namestr.data(), namestr.length() * 2);
                    }
                }
            }
    }

    gr->m_Buf.Add(&outm, sizeof(SMaterial));

    return gr->m_No;
}

int EGExp2::AddVertex(Base::CBuf &buf, SVertexNorTex &v)
{
    int cnt = buf.Len() / sizeof(SVertexNorTex);
    for (int i = 0; i < cnt; i++)
    {
        SVertexNorTex &vc = *(((SVertexNorTex *)buf.Get()) + i);
        if (((double(v.x) - double(vc.x)) * (double(v.x) - double(vc.x)) +
             (double(v.y) - double(vc.y)) * (double(v.y) - double(vc.y)) +
             (double(v.z) - double(vc.z)) * (double(v.z) - double(vc.z))) >= 0.000001)
            continue;
        if (((double(v.nx) - double(vc.nx)) * (double(v.nx) - double(vc.nx)) +
             (double(v.ny) - double(vc.ny)) * (double(v.ny) - double(vc.ny)) +
             (double(v.nz) - double(vc.nz)) * (double(v.nz) - double(vc.nz))) >= 0.000001)
            continue;
        if (((double(v.tu) - double(vc.tu)) * (double(v.tu) - double(vc.tu)) +
             (double(v.tv) - double(vc.tv)) * (double(v.tv) - double(vc.tv))) >= 0.000001)
            continue;
        return i;
    }
    buf.Pointer(buf.Len());
    buf.Add(&v, sizeof(SVertexNorTex));
    return cnt;
}

std::wstring EGExp2::RenameByCfg(const std::wstring &str)
{
    if (m_Rename.ParCount(str) <= 0)
        return str;
    return m_Rename.ParGet(str);
}

void EGExp2::ExportSimpleGroup()
{
    int no, exp;
    std::wstring tstr;
    int cnt = ExportGroupCount();
    for (int i = 0; i < cnt; i++)
    {
        m_GroupName.clear();
        m_FileName.clear();
        m_Anim.Clear();
        tstr.clear();

        if (ExportGroupGetProp(i, no, nullptr, m_GroupName, exp, m_FileName, m_TextureDefault, m_EdgeExport, m_EdgeExportFactor, tstr) && exp)
        {
            m_Anim.Clear();
            m_Anim.LoadFromText(tstr);

            tstr.clear();
            ExportGroupGetRename(tstr);
            m_Rename.Clear();
            m_Rename.LoadFromText(tstr);

            m_EdgeExportFactor = cos(m_EdgeExportFactor * pi / 180.0);

            Log(L"=== Group ===");

            ExportSimple();
        }
    }
}

void EGExp2::ExportSimple()
{
    SVOHeader he;
    SVOGroup vogr;
    SVOFrame voframe;
    SVOFrame *voframep;
    SVOAnimHeader voanimh;
    SVOAnimHeader *voanimhp;

    utils::ParamParser tstr;
    std::wstring tstr2;
    Base::CBuf bufout;
    Base::CBuf bframes;
    std::wstring filename;

    m_MaterialNextNo = 0;

    std::vector<int> framelist;

    std::vector<std::pair<size_t, size_t>> vergrouplist;
    std::vector<std::pair<size_t, size_t>> trigrouplist;
    std::vector<size_t> matrixlist;

    GroupClear();

    filename = m_FileName;

    try
    {
        int animcnt = m_Anim.ParCount();
        if (animcnt > 0)
        {
            Interval range = ip->GetAnimRange();
            int framestart = range.Start() / GetTicksPerFrame();
            int frameend = range.End() / GetTicksPerFrame();
            int framemaxcnt = frameend - framestart + 1;

            framelist.clear();
            framelist.reserve(framemaxcnt);

            for (int i = 0; i < animcnt; i++)
            {
                tstr = m_Anim.ParGetName(i) + L"," + m_Anim.ParGet(i);

                int cnt = tstr.GetCountPar(L",");
                if (cnt < 3)
                    throw L"Anim format error";

                StrToFrameList(tstr.GetStrPar(2, cnt-1, L","), bframes);

                for (int u = 0; u < bframes.Len() / 8; u++)
                {
                    int curframe = bframes.Get<int>();
                    bframes.Get<int>();
                    if (curframe < framestart || curframe > frameend)
                        throw L"Anim error";
                    auto it = std::find(framelist.begin(), framelist.end(), curframe);
                    if (it == framelist.end())
                        framelist.push_back(curframe);
                }
            }
        }
        else
        {
            framelist.clear();
            framelist.push_back(ip->GetTime());
        }

        for (int i = 0; i < framelist.size(); i++)
        {
            int numChildren = ip->GetRootNode()->NumberOfChildren();

            if (!ExportCenterGetProp(m_GroupName, framelist[i] * GetTicksPerFrame(), nullptr,
                                     m_MaxSizeX, m_MaxSizeY, m_MaxSizeZ,
                                     m_Scale.x, m_Scale.y, m_Scale.z,
                                     m_MatCenter)) 
            {
                m_MatCenter.IdentityMatrix();
                m_Scale = Point3(1.0, 1.0, 1.0);
                m_MaxSizeX = 0;
                m_MaxSizeY = 0;
                m_MaxSizeZ = 0;
            }

            m_MatCenter.Scale(m_Scale, FALSE);

            for (int idx = 0; idx < numChildren; idx++)
            {
                if (ip->GetCancel())
                    throw L"Terminate";
                ExportSimple_NodeEnum(ip->GetRootNode()->GetChildNode(idx), framelist[i] * GetTicksPerFrame(), i);
            }
        }

        GroupSortByNo();

        if (!m_GroupName.empty())
            Log(L"Group=" + m_GroupName);
        Log(L"OutFile=" + filename);

        int cntvergroup = 0;
        int cnttrigroup = 0;

        for (auto &gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_Ver)
                cntvergroup++;
            else if(gr.m_Id[0].first == gt_Frame && gr.m_Id[1].first == gt_TriList)
                cnttrigroup++;
        }

        // header
        ZeroMemory(&he, sizeof(SVOHeader));
        he.m_Id = 0x00006f76;
        he.m_Ver = 0;

        if (m_TextureDefault)
            he.m_Flags |= 2;

        bufout.Add(&he, sizeof(SVOHeader));

        // material
        he.m_MaterialSme = bufout.Pointer();
        for (auto &gr : m_Groups)
        {
            if(gr.m_Id[0].first == gt_Material)
            {
                gr.m_User = he.m_MaterialCnt;

                he.m_MaterialCnt += gr.m_Buf.Len() / sizeof(SMaterial);
                bufout.Add(gr.m_Buf.Get(), gr.m_Buf.Len());

                SMaterial *mat = reinterpret_cast<SMaterial *>(gr.m_Buf.Get());
                if (mat->tex_diffuse[0] != 0)
                    Log(std::wstring(L"Texture=") + mat->tex_diffuse);
            }
        }

        // vertex scale
        double minx = 1e30, miny = 1e30, minz = 1e30;
        double maxx = -1e30, maxy = -1e30, maxz = -1e30;

        for (auto &gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_Ver)
            {
                int cnt = gr.m_Buf.Len() / sizeof(SVertexNorTex);
                SVertexNorTex *cv = reinterpret_cast<SVertexNorTex *>(gr.m_Buf.Get());
                for (;cnt > 0; --cnt, ++cv)
                {
                    minx = std::min((double)cv->x, minx);
                    miny = std::min((double)cv->y, miny);
                    minz = std::min((double)cv->z, minz);
                    maxx = std::max((double)cv->x, maxx);
                    maxy = std::max((double)cv->y, maxy);
                    maxz = std::max((double)cv->z, maxz);
                }
            }
        }

        double koffx = 1e30, koffy = 1e30, koffz = 1e30;

        if (m_MaxSizeX > 0)
            koffx = m_MaxSizeX / (maxx - minx);
        if (m_MaxSizeY > 0)
            koffy = m_MaxSizeY / (maxy  -miny);
        if (m_MaxSizeZ > 0)
            koffz = m_MaxSizeZ / (maxz - minz);

        double kofscale = std::min(std::min(koffx,koffy), koffz);
        if (kofscale > 1e20)
            kofscale = 1.0;

        if (kofscale != 1.0)
        {
            for (auto &gr : m_Groups)
            {
                if (gr.m_Id[0].first == gt_Ver)
                {
                    int cnt = gr.m_Buf.Len() / sizeof(SVertexNorTex);
                    SVertexNorTex *cv = reinterpret_cast<SVertexNorTex *>(gr.m_Buf.Get());
                    for (;cnt > 0; --cnt, ++cv)
                    {
                        cv->x = float(cv->x * kofscale);
                        cv->y = float(cv->y * kofscale);
                        cv->z = float(cv->z * kofscale);
                    }
                }
            }
        }

        for (auto& gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_ExpMatrix)
            {
                BYTE *buf = reinterpret_cast<BYTE *>(gr.m_Buf.Get());
                SVOMatrix *em = reinterpret_cast<SVOMatrix *>(buf + sizeof(SVOExpMatrixHeader));

                Matrix3 mm;
                ConvertMatrix(mm, *em);
                mm.Scale(Point3(kofscale, kofscale, kofscale), true);
                mm.Orthogonalize();
                mm.NoScale();
                ConvertMatrix(*em, mm);
            }
        }

        // vertex
        vergrouplist.clear();
        vergrouplist.reserve(cntvergroup);
        he.m_VerSme = bufout.Pointer();
        for (auto &gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_Ver)
            {
                BYTE *buf = reinterpret_cast<BYTE *>(bufout.Get());
                auto it = std::find_if(vergrouplist.begin(), vergrouplist.end(),
                    [&gr, buf](auto &t) {
                        void *ptr = buf + t.second;
                        return t.first == gr.m_Buf.Len() && memcmp(ptr, gr.m_Buf.Get(), gr.m_Buf.Len()) == 0;
                    });
                if (it == vergrouplist.end())
                {
                    gr.m_User = gr.m_Buf.Len(); // size
                    gr.m_User2 = bufout.Pointer(); // sme
                    bufout.Add(gr.m_Buf.Get(), gr.m_Buf.Len());
                    vergrouplist.push_back(std::make_pair(gr.m_User, gr.m_User2));
                    he.m_VerCnt += gr.m_Buf.Len() / sizeof(SVertexNorTex);
                }
                else
                {
                    gr.m_User = it->first; // size
                    gr.m_User2 = it->second; // sme
                }
            }
        }
        Log(utils::format(L"Vertex=%i size=%i", he.m_VerCnt, he.m_VerCnt * sizeof(SVertexNorTex)));


        // triangle
        trigrouplist.clear();
        trigrouplist.reserve(cnttrigroup);
        he.m_TriSme = bufout.Pointer();
        for (auto &gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_Frame && gr.m_Id[1].first == gt_TriList)
            {
                BYTE *buf = reinterpret_cast<BYTE *>(bufout.Get());
                auto it = std::find_if(trigrouplist.begin(), trigrouplist.end(),
                    [&gr, buf](auto &t) {
                        void *ptr = buf + t.second;
                        return t.first == gr.m_Buf.Len() && memcmp(ptr, gr.m_Buf.Get(), gr.m_Buf.Len()) == 0;
                    });
                if (it == trigrouplist.end())
                {
                    gr.m_User = gr.m_Buf.Len(); // size
                    gr.m_User2 = bufout.Pointer(); // sme
                    bufout.Add(gr.m_Buf.Get(), gr.m_Buf.Len());
                    trigrouplist.push_back(std::make_pair(gr.m_User, gr.m_User2));
                    he.m_TriCnt += gr.m_Buf.Len() / 4;
                }
                else
                {
                    gr.m_User = it->first; // size
                    gr.m_User2 = it->second; // sme
                }
            }
        }

        // triangle index to 16 bit
        if (he.m_TriCnt < 65536)
        {
            he.m_Flags |= 1;

            BYTE *buf = reinterpret_cast<BYTE *>(bufout.Get());
            DWORD *sou = reinterpret_cast<DWORD *>(buf + he.m_TriSme);
            WORD *des = reinterpret_cast<WORD *>(sou);

            std::transform(sou, sou + he.m_TriCnt, des, [](auto i) { return static_cast<WORD>(i); });

            bufout.Len(he.m_TriSme + he.m_TriCnt * sizeof(WORD));
            bufout.Pointer(bufout.Len());
        }
        Log(utils::format(L"Triangles=%i size=%i", he.m_TriCnt / 3, he.m_TriCnt * ((he.m_Flags & 1) ? sizeof(WORD) : sizeof(DWORD))));

        // group
        he.m_GroupSme = bufout.Pointer();
        for (auto &gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_Frame && gr.m_Id[1].first == gt_TriList)
            {
                ZeroMemory(&vogr, sizeof(SVOGroup));

                gr.m_User3 = he.m_GroupCnt;

                CGroup *grpoint = GroupFind(CTuple(gt_Ver, gr.m_Id[0].second));

                vogr.m_Material = gr.m_Id[1].second;
                vogr.m_VerCnt = grpoint->m_User / sizeof(SVertexNorTex);
                vogr.m_VerStart = (grpoint->m_User2 - he.m_VerSme) / sizeof(SVertexNorTex);
                vogr.m_TriCnt = gr.m_User / 4;
                vogr.m_TriStart = (gr.m_User2 - he.m_TriSme) / 4;

                bufout.Add(&vogr, sizeof(SVOGroup));

                he.m_GroupCnt++;
            }
        }
        Log(utils::format(L"Groups=%i", he.m_GroupCnt));

        // edge
        if (m_EdgeExport)
        {
            he.m_EdgeSme = bufout.Pointer();

            for (auto &gr : m_Groups)
            {
                if (gr.m_Id[0].first == gt_Edge)
                {
                    gr.m_User = (bufout.Pointer() - he.m_EdgeSme) / sizeof(SVOEdge);
                    gr.m_User2 = gr.m_Buf.Len() / sizeof(SVOEdge);

                    bufout.Add(gr.m_Buf.Get(), gr.m_Buf.Len());
                    he.m_EdgeCnt += gr.m_Buf.Len() / sizeof(SVOEdge);
                }
            }
            Log(utils::format(L"Edges=%i", he.m_EdgeCnt));
        }

        // frame
        ZeroMemory(&voframe, sizeof(SVOFrame));
        he.m_FrameSme = bufout.Pointer();
        he.m_FrameCnt = framelist.size();

        voframe.m_MinX = 1e30f;
        voframe.m_MinY = 1e30f;
        voframe.m_MinZ = 1e30f;

        voframe.m_MaxX = -1e30f;
        voframe.m_MaxY = -1e30f;
        voframe.m_MaxZ = -1e30f;

        for (int i = 0; i < framelist.size(); i++)
            bufout.Add(&voframe, sizeof(SVOFrame));

        for (int i = 0; i < framelist.size(); i++)
        {
            int sme = bufout.Pointer();
            int cnt = 0;
            for (auto& gr : m_Groups)
            {
                if (gr.m_Id[0].first == gt_Frame && gr.m_Id[0].second == i && gr.m_Id[1].first == gt_TriList)
                {
                    bufout.Add<DWORD>(gr.m_User3);
                    gr.m_User4 = cnt;
                    cnt++;
                }
            }
            BYTE *buf = reinterpret_cast<BYTE *>(bufout.Get());
            voframep = reinterpret_cast<SVOFrame *>(buf + he.m_FrameSme + i * sizeof(SVOFrame));
            voframep->m_GroupIndexCnt = cnt;
            voframep->m_GroupIndexSme = sme;

            CGroup *gr = GroupFind(CTuple(gt_Ver, i));

            cnt = gr->m_Buf.Len() / sizeof(SVertexNorTex);
            SVertexNorTex *cv = reinterpret_cast<SVertexNorTex *>(gr->m_Buf.Get());
            for (;cnt > 0; --cnt, ++cv)
            {
                voframep->m_MinX = std::min(cv->x, voframep->m_MinX);
                voframep->m_MinY = std::min(cv->y, voframep->m_MinY);
                voframep->m_MinZ = std::min(cv->z, voframep->m_MinZ);
                voframep->m_MaxX = std::max(cv->x, voframep->m_MaxX);
                voframep->m_MaxY = std::max(cv->y, voframep->m_MaxY);
                voframep->m_MaxZ = std::max(cv->z, voframep->m_MaxZ);
                voframep->m_CenterX += cv->x;
                voframep->m_CenterY += cv->y;
                voframep->m_CenterZ += cv->z;
            }

            cnt = gr->m_Buf.Len() / sizeof(SVertexNorTex);
            voframep->m_CenterX /= cnt;
            voframep->m_CenterY /= cnt;
            voframep->m_CenterZ /= cnt;
            cv = reinterpret_cast<SVertexNorTex *>(gr->m_Buf.Get());
            float mr = 0, mr2 = 0;
            for (;cnt > 0; --cnt, ++cv)
            {
                mr = std::max(mr, (voframep->m_CenterX - cv->x) * (voframep->m_CenterX - cv->x) +
                                  (voframep->m_CenterY - cv->y) * (voframep->m_CenterY - cv->y) +
                                  (voframep->m_CenterZ - cv->z) * (voframep->m_CenterZ - cv->z));
                mr2 = std::max(mr2, ((voframep->m_MaxX + voframep->m_MinX) / 2 - cv->x) * ((voframep->m_MaxX + voframep->m_MinX) / 2 - cv->x) +
                                    ((voframep->m_MaxY + voframep->m_MinY) / 2 - cv->y) * ((voframep->m_MaxY + voframep->m_MinY) / 2 - cv->y) +
                                    ((voframep->m_MaxZ + voframep->m_MinZ) / 2 - cv->z) * ((voframep->m_MaxZ + voframep->m_MinZ) / 2 - cv->z));
            }
            voframep->m_RadiusCenter = float(sqrt(mr));
            voframep->m_RadiusBox = float(sqrt(mr2));

            if (m_EdgeExport)
            {
                gr = GroupFind(CTuple(gt_Edge, i));
                if (gr)
                {
                    voframep->m_EdgeStart = gr->m_User;
                    voframep->m_EdgeCnt = gr->m_User2;

                    int lastmat1 = -1;
                    int lastmat2 = -1;
                    int lastgroupinframe1 = -1;
                    int lastgroupinframe2 = -1;

                    BYTE *buf = reinterpret_cast<BYTE *>(bufout.Get());
                    SVOEdge *edg = reinterpret_cast<SVOEdge *>(buf + he.m_EdgeSme + voframep->m_EdgeStart * sizeof(SVOEdge));
                    for (int ei = 0; ei < int(voframep->m_EdgeCnt); ++ei, ++edg)
                    {
                        int cmat = ((edg->m_SideTri1) >> 20) & 0x0ff;
                        if (lastmat1 != cmat)
                        {
                            lastmat1 = cmat;
                            CGroup *gr = GroupFind(CTuple(gt_Frame, i), CTuple(gt_TriList, cmat));
                            lastgroupinframe1 = gr->m_User4;
                        }
                        edg->m_SideTri1 = (edg->m_SideTri1 & (~(0x0ff << 20))) | ((lastgroupinframe1) << 20);

                        cmat = ((edg->m_SideTri2) >> 20) & 0x0ff;
                        if (lastmat2 != cmat)
                        {
                            lastmat2 = cmat;
                            CGroup *gr = GroupFind(CTuple(gt_Frame, i), CTuple(gt_TriList, cmat));
                            lastgroupinframe2 = gr->m_User4;
                        }
                        edg->m_SideTri2 = (edg->m_SideTri2 & (~(0x0ff << 20))) | ((lastgroupinframe2) << 20);
                    }
                }
            }
        }
        Log(utils::format(L"Frames=%i", he.m_FrameCnt));

        // anim
        ZeroMemory(&voanimh, sizeof(SVOAnimHeader));
        he.m_AnimSme = bufout.Pointer();
        he.m_AnimCnt = animcnt;
        for (int i = 0; i < animcnt; i++)
            bufout.Add(&voanimh, sizeof(SVOAnimHeader));
        for (int i = 0; i < animcnt; i++)
        {
            ZeroMemory(&voanimh, sizeof(SVOAnimHeader));

            tstr = m_Anim.ParGetName(i) + L"," + m_Anim.ParGet(i);
            int cnt = tstr.GetCountPar(L",");

            voanimh.m_Id = tstr.GetStrPar(0, L",").GetInt();

            tstr2 = RenameByCfg(tstr.GetStrPar(1, L","));
            if (tstr2.length() > 31)
                tstr2.resize(31);
            if (tstr2.length() > 0)
                CopyMemory(voanimh.m_Name, tstr2.data(), tstr2.length() * 2);

            StrToFrameList(tstr.GetStrPar(2, cnt-1, L","), bframes);

            voanimh.m_UnitCnt = bframes.Len() / 8;
            voanimh.m_UnitSme = bufout.Pointer();

            tstr = utils::format(L"Anim=%i,%s", voanimh.m_Id, voanimh.m_Name);
            bframes.Pointer(0);
            for (int u = 0; u < bframes.Len() / 8; u++)
            {
                int cf = bframes.Get<int>();
                int ct = bframes.Get<int>();

                int t = 0;
                for (;t < framelist.size(); t++)
                    if (framelist[t] == cf)
                    {
                        cf = t;
                        break;
                    }
                if (t >= framelist.size())
                    throw L"Error in anim";

                tstr += L",";
                tstr += cf;

                bufout.Add<int>(cf);
                bufout.Add<int>(ct);
            }
            Log(tstr);

            tstr = utils::format(L"Anim=%i,%s", voanimh.m_Id, voanimh.m_Name);
            bframes.Pointer(0);
            for (int u = 0; u < bframes.Len() / 8; u++)
            {
                bframes.Get<int>();
                tstr += L",";
                tstr += bframes.Get<int>();
            }
            Log(tstr);

            voanimhp = (SVOAnimHeader *)(((BYTE *)bufout.Get()) + he.m_AnimSme + i * sizeof(SVOAnimHeader));
            CopyMemory(voanimhp, &voanimh, sizeof(SVOAnimHeader));
        }

        // ExportMatrix
        matrixlist.clear();
        matrixlist.reserve(GroupCount());
        he.m_MatrixSme = bufout.Pointer();
        he.m_MatrixCnt = 0;
        for (auto &gr : m_Groups)
        {
            if (gr.m_Id[0].first == gt_ExpMatrix && gr.m_Id[0].second == 0 && gr.m_Id[1].first == gt_ExpMatrix)
            {
                he.m_MatrixCnt++;

                SVOExpMatrixHeader *emh = (SVOExpMatrixHeader *)gr.m_Buf.Get();
                Log(utils::format(L"Matrix=%s", emh->m_Name));

                bufout.Add(gr.m_Buf.Get(), sizeof(SVOExpMatrixHeader));
                matrixlist.push_back(gr.m_Id[1].second);
            }
        }
        for (int i = 0; i < matrixlist.size(); i++)
        {
            for (int u = 0; u < framelist.size(); u++)
            {
                CGroup *gr = GroupFind(CTuple(gt_ExpMatrix, u), CTuple(gt_ExpMatrix, matrixlist[i]));

                if (u == 0)
                {
                    BYTE *buf = reinterpret_cast<BYTE *>(bufout.Get());
                    SVOExpMatrixHeader *emh = reinterpret_cast<SVOExpMatrixHeader *>(buf + he.m_MatrixSme + i * sizeof(SVOExpMatrixHeader));
                    emh->m_MatrixSme = bufout.Pointer();
                }

                BYTE *buf = reinterpret_cast<BYTE *>(gr->m_Buf.Get());
                bufout.Add(buf + sizeof(SVOExpMatrixHeader), sizeof(SVOMatrix));
            }
        }

        // header again
        CopyMemory(bufout.Get(), &he, sizeof(SVOHeader));

        Log(utils::format(L"File Size=%i", bufout.Len()));

        // Save
        Base::CFile fileout(filename);
        fileout.Create();
        fileout.Write(bufout.Get(), bufout.Len());
        fileout.Close();
    }
    catch(wchar *estr)
    {
        Log(std::wstring(estr));
    }
}

BOOL EGExp2::ExportSimple_NodeEnum(INode *node, TimeValue timev, int frame)
{
    if (ip->GetCancel())
        return FALSE;

    const ObjectState &os = node->EvalWorldState(timev); 
    if (os.obj)
    {
        switch (os.obj->SuperClassID())
        {
            case GEOMOBJECT_CLASS_ID:
                ExportSimple_GeomObject(node, timev, frame); 
                break;
            case HELPER_CLASS_ID:
                ExportSimple_Helper(node, timev, frame); 
                break;
        }
    }

    for (int c = 0; c < node->NumberOfChildren(); c++)
    {
        if (!ExportSimple_NodeEnum(node->GetChildNode(c), timev, frame))
            return FALSE;
    }

    return TRUE;
}

void EGExp2::ExportSimple_GeomObject(INode *node, TimeValue timev, int frame)
{
    SVertexNorTex vdes, vdess[3];
    Point3 v, v0, v1, v2;

    std::wstring tstr;
    int temp;
    if (ExportObjGetProp(node, tstr, temp))
    {
        if (!temp)
            return;
        if (!m_GroupName.empty() && !tstr.empty() && m_GroupName != tstr)
            return;
    }

    Matrix3 tm = node->GetObjTMAfterWSM(timev) * m_MatCenter;

    int vx[3];
    if (TMNegParity(tm))
    { 
       vx[0] = 2;
       vx[1] = 1;
       vx[2] = 0;
    }
    else
    {
        vx[0] = 0;
        vx[1] = 1;
        vx[2] = 2;
    }

    BOOL needDel;
    TriObject *tri = GetTriObjectFromNode(node, timev, needDel);
    if (!tri)
        return;

    Mtl *nodeMtl = node->GetMtl();

    Mesh *mesh = &tri->GetMesh();
    mesh->buildRenderNormals();

    CGroup *gv = GroupFindAdd(CTuple(gt_Ver, frame));
    CGroup *gf = nullptr;
    if (m_EdgeExport)
        gf = GroupFindAdd(CTuple(gt_Edge, frame));

    Matrix3 normalObjToWorld;
    for (int it = 0; it < 4; it++)
    {
        Point3 p = tm.GetRow(it);
        normalObjToWorld.SetRow(it, p);
    }

    std::vector<size_t> edgeindexlist;
    if (m_EdgeExport)
    {
        edgeindexlist.clear();
        edgeindexlist.reserve(mesh->getNumFaces());
    }

    for (int i = 0; i < mesh->getNumFaces(); i++)
    {
        v0 = tm * mesh->verts[mesh->faces[i].v[vx[0]]];
        v1 = tm * mesh->verts[mesh->faces[i].v[vx[1]]];
        v2 = tm * mesh->verts[mesh->faces[i].v[vx[2]]];

        Point3 nfn = ((v1 - v0) ^ (v2 - v0)).Normalize();
        mesh->setFaceNormal(i, nfn);
    }

    mesh->buildNormals();

    if (mesh->tVerts == nullptr)
    {
        MSTR str;
        node->EvalWorldState(timev).obj->GetClassName(str);
        Log(utils::format(L"Mapping coords not found: %s  Class name: %s",
                node->NodeName().data(),
                str.data() 
            ));
    }

    for (int i = 0; i < mesh->getNumVerts(); i++)
    {
        v = mesh->verts[i];
        int cnt = 0;
        for (int u = i+1; u < mesh->getNumVerts(); u++)
            if (i != u)
                if ((mesh->verts[u] - v).Length() <= 0.01)
                {
                    mesh->VertSel().Set(u);
                    mesh->VertSel().Set(i);
                    cnt++;
                }
        if (cnt >= 1)
            mesh->VertSel().Set(i);
        else
            mesh->VertSel().Clear(i);
    }

    for (int i = 0; i < mesh->getNumFaces(); i++)
    {
        int twosided = 0;
        int mno = GroupMaterialFindAdd(nodeMtl, mesh->faces[i].getMatID(), twosided);
        CGroup *gtl = GroupFindAdd(CTuple(gt_Frame, frame), CTuple(gt_TriList, mno));

        if (m_EdgeExport)
            edgeindexlist.push_back((mno << 20) | (gtl->m_Buf.Pointer() / (4 * 3)));

        for (int u = 0; u < 3; u++)
        {
            int vi = mesh->faces[i].v[vx[u]];

            v = tm * mesh->verts[vi];
            vdes.x = v.x;
            vdes.y = v.y;
            vdes.z = v.z;

            v = Normalize(GetVertexNormal(mesh, i, mesh->getRVertPtr(vi)));
            vdes.nx = v.x;
            vdes.ny = v.y;
            vdes.nz = v.z;

            if (mesh->tVerts != nullptr)
            {
                v = mesh->tVerts[mesh->tvFace[i].t[vx[u]]];
                vdes.tu = v.x;
                vdes.tv = 1.0f - v.y;
            }
            else
            {
                vdes.tu = 0;
                vdes.tv = 0;
            }

            vdess[u] = vdes;

            gtl->m_Buf.Add<DWORD>(AddVertex(gv->m_Buf, vdes));

        }
        if (twosided)
        {
            vdess[0].nx = -vdess[0].nx;
            vdess[0].ny = -vdess[0].ny;
            vdess[0].nz = -vdess[0].nz;

            vdess[1].nx = -vdess[1].nx;
            vdess[1].ny = -vdess[1].ny;
            vdess[1].nz = -vdess[1].nz;

            vdess[2].nx = -vdess[2].nx;
            vdess[2].ny = -vdess[2].ny;
            vdess[2].nz = -vdess[2].nz;

            gtl->m_Buf.Add<DWORD>(AddVertex(gv->m_Buf, vdess[0]));
            gtl->m_Buf.Add<DWORD>(AddVertex(gv->m_Buf, vdess[2]));
            gtl->m_Buf.Add<DWORD>(AddVertex(gv->m_Buf, vdess[1]));
        }
    }

    if (m_EdgeExport)
    {
        for(int i = 0; i < edgeindexlist.size(); i++)
        {
            for(int u = 0; u < 3; u++)
            {
                int vi1 = mesh->faces[i].v[vx[u]];
                int vi2;
                if (u < 2)
                    vi2 = mesh->faces[i].v[vx[u+1]];
                else
                    vi2 = mesh->faces[i].v[vx[0]];

                int cnttri = 0;

                for (int p = 0; p < edgeindexlist.size(); p++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        int vu1=mesh->faces[p].v[vx[k]];
                        int vu2;
                        if (k < 2)
                            vu2 = mesh->faces[p].v[vx[k+1]];
                        else
                            vu2 = mesh->faces[p].v[vx[0]];

                        if (((vi1 == vu1) && (vi2 == vu2)) || ((vi1 == vu2) && (vi2 == vu1)))
                        {
                            cnttri++;

                            if (cnttri==2)
                            {
                                double dp = DotProd(mesh->getFaceNormal(i).Normalize(), mesh->getFaceNormal(p).Normalize());
                                if ((m_EdgeExport == 1) || (dp < m_EdgeExportFactor/*0.000000001*/))
                                {
                                    gf->m_Buf.Add<DWORD>((u << 28) | edgeindexlist[i]);
                                    gf->m_Buf.Add<DWORD>((k << 28) | edgeindexlist[p]);
                                }
                            }
                        }
                    }
                }

                if (cnttri != 2)
                {
                    mesh->FaceSel().Set(i);
                    MSTR str;
                    node->EvalWorldState(timev).obj->GetClassName(str);

                    Log(utils::format(L"У грани не 2 треугольника. Face:%i Cnt: %i  Name: %s  Class name: %s",
                        i+1,
                        cnttri,
                        node->NodeName().data(),
                        str.data() 
                    ));
                }

            }
        }
    }

    if (needDel)
        delete tri;
}

void EGExp2::ExportSimple_Helper(INode *node, TimeValue timev, int frame)
{
    SVOExpMatrixHeader emh;

    int id;
    std::wstring name;
    if (!ExportMatrixGetProp(node, m_GroupName, id, name))
        return;

    Matrix3 tm = node->GetObjTMAfterWSM(timev) * m_MatCenter;

    CGroup *gv = GroupFindAdd(CTuple(gt_ExpMatrix, frame), CTuple(gt_ExpMatrix, (uintptr_t)node));

    ZeroMemory(&emh, sizeof(SVOExpMatrixHeader));
    emh.m_Id = id;
    name = RenameByCfg(name);
    if (name.length() > 31)
        name.resize(31);
    CopyMemory(emh.m_Name, name.data(), name.length() * 2);

    gv->m_Buf.Clear();
    gv->m_Buf.Add(&emh, sizeof(SVOExpMatrixHeader));

    SVOMatrix em;
    tm.Orthogonalize();
    tm.NoScale();
    ConvertMatrix(em, tm);
    gv->m_Buf.Add(&em, sizeof(SVOMatrix));
}

BOOL EGExp2::TMNegParity(Matrix3 &m)
{
    return (DotProd(CrossProd(m.GetRow(0), m.GetRow(1)), m.GetRow(2)) < 0.0) ? 1 : 0;
}

TriObject *EGExp2::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
    deleteIt = FALSE;
    Object *obj = node->EvalWorldState(t).obj;
    if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
    { 
        TriObject *tri = (TriObject *) obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
        // Note that the TriObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (obj != tri)
            deleteIt = TRUE;
        return tri;
    }
    else
        return nullptr;
}

Point3 EGExp2::GetVertexNormal(Mesh *mesh, int faceNo, RVertex *rv)
{
    Face *f = &mesh->faces[faceNo];
    DWORD smGroup = f->smGroup;
    int numNormals;
    Point3 vertexNormal;

    // Is normal specified
    // SPCIFIED is not currently used, but may be used in future versions.
    if (rv->rFlags & SPECIFIED_NORMAL) {
        vertexNormal = rv->rn.getNormal();
    }
    // If normal is not specified it's only available if the face belongs
    // to a smoothing group
    else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
        // If there is only one vertex is found in the rn member.
        if (numNormals == 1) {
            vertexNormal = rv->rn.getNormal();
        } else {
            // If two or more vertices are there you need to step through them
            // and find the vertex with the same smoothing group as the current face.
            // You will find multiple normals in the ern member.
            for (int i = 0; i < numNormals; i++) {
                if (rv->ern[i].getSmGroup() & smGroup) {
                    vertexNormal = rv->ern[i].getNormal();
                }
            }
        }
    } else {
        // Get the normal from the Face if no smoothing groups are there
        vertexNormal = mesh->getFaceNormal(faceNo);
    }

    return vertexNormal;
}
