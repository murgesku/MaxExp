#ifndef __MAIN__H
#define __MAIN__H

// Edge=<0,1,2>     0-off 1-all 2-optimize

struct SVOHeader
{
    DWORD m_Id;  // 0x00006f76
    DWORD m_Ver;  // Версия
    DWORD m_Flags;  // {1-16 битный индекс иначе 32 битный} {2-default texture}
    DWORD r1;
    DWORD m_MaterialCnt;  // Список материалов(текстур) SMaterial
    DWORD m_MaterialSme;  // Положение от начала заголовка
    DWORD m_GroupCnt;  // Инвормация по группам (Смещение верши и треугольников)
    DWORD m_GroupSme;
    DWORD m_VerCnt;  // Список всех вершин SVertexNorTex
    DWORD m_VerSme;
    DWORD m_TriCnt;  // Список всех треугольников. Кол-во индексов (3 индкса для каждого трегольника по 2 или 4 байта взависимости от флага)
    DWORD m_TriSme;
    DWORD m_FrameCnt;  // Список кадров SVOFrame
    DWORD m_FrameSme;
    DWORD m_AnimCnt;  // Список анимаций SVOAnimHeader
    DWORD m_AnimSme;
    DWORD m_MatrixCnt;  // Список матриц SVOExpMatrixHeader
    DWORD m_MatrixSme;
    DWORD m_EdgeCnt;  // Список всех граней
    DWORD m_EdgeSme;
};

struct SVOGroup
{
    DWORD m_Material;
    DWORD m_Flags;  // 0-list
    DWORD m_VerCnt;
    DWORD m_VerStart;
    DWORD m_TriCnt;  // Кол-во индексов
    DWORD m_TriStart;
    DWORD r1;
    DWORD r2;
};

struct SVOFrame
{
    DWORD m_GroupIndexCnt;
    DWORD m_GroupIndexSme;  // Каждый индекс 4-байтный указатель
    float m_CenterX,m_CenterY,m_CenterZ;
    float m_RadiusCenter;
    float m_MinX,m_MinY,m_MinZ;
    float m_MaxX,m_MaxY,m_MaxZ;
    float m_RadiusBox;
    DWORD m_EdgeCnt;
    DWORD m_EdgeStart;
    DWORD r1;
};

struct SVOAnimHeader
{
    DWORD m_Id;
    wchar m_Name[32];
    DWORD m_UnitCnt;
    DWORD m_UnitSme;
    DWORD r1;
};

struct SVOAnimUnit
{
    DWORD m_FrameIndex;
    DWORD m_Time;  // in milisecond
};

struct SVOExpMatrixHeader
{
    DWORD m_Id;
    wchar m_Name[32];
    DWORD m_MatrixSme;  // Спиок SVOMatrix   (кол-во по количеству m_FrameCnt)
    DWORD r1;
    DWORD r2;
};

struct SVOMatrix
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;

        };
        float m[4][4];
    };
};

struct SVOEdge
{
    DWORD m_SideTri1;  // 0xf0000000 - triangle side, 0x0ff00000 - group in frame, 0x000fffff - triangle
    DWORD m_SideTri2;  // 0xf0000000 - triangle side, 0x0ff00000 - group in frame, 0x000fffff - triangle
};

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;
extern ClassDesc *GetParObjModClassDesc();
extern ClassDesc *GetParGroupModClassDesc();
extern ClassDesc *GetParCenterModClassDesc();
extern ClassDesc *GetParMatrixModClassDesc();

#define VERSION 200  // Version number * 100
#define CFGFILENAME _T("EGExp.CFG")  // Configuration file

class MtlKeeper
{
public:
    BOOL AddMtl(Mtl *mtl);
    int GetMtlID(Mtl *mtl);
    int Count();
    Mtl *GetMtl(int id);

    Tab<Mtl *> mtlTab;
};

struct SVertexNorTex
{
    float x, y, z;
    float nx, ny, nz;
    float tu, tv;
};

struct SMaterial
{
    DWORD m_Id;  // Pointer on material
    float dr, dg, db, da;
    float ar, ag, ab, aa;
    float sr, sg, sb, sa;
    float er, eg, eb, ea;
    float power;
    wchar tex_diffuse[32];
};

#define gt_Ver 1
#define gt_Material 2
#define gt_TriList 3
#define gt_Frame 4
#define gt_ExpMatrix 5
#define gt_Edge 6

using CTuple = std::pair<int, uintptr_t>;

class CGroup : public Base::CMain
{
public:
    uintptr_t m_User;
    uintptr_t m_User2;
    uintptr_t m_User3;
    uintptr_t m_User4;

    int m_No;
    int m_IdCnt;
    CTuple m_Id[4];
    Base::CBuf m_Buf;

    CGroup() : m_Id(), m_Buf()
    {
        m_User = 0;
        m_User2 = 0;
        m_User3 = 0;
        m_User4 = 0;

        m_IdCnt = 0;
        m_No = 0;
    }

    CGroup(const CGroup& other) : m_Id()
    {
        m_User = other.m_User;
        m_User2 = other.m_User2;
        m_User3 = other.m_User3;
        m_User4 = other.m_User4;

        m_No = other.m_No;
        m_IdCnt = other.m_IdCnt;

        std::copy(other.m_Id, other.m_Id + 4, m_Id);
        m_Buf = other.m_Buf;
    }

    ~CGroup() {}
};

class EGExp2
{
public:
    Base::CBlockPar m_Rename;
    Base::CBlockPar m_Anim;

    Interface *ip;

    int m_MaterialNextNo;
    std::list<CGroup> m_Groups;

    std::wstring m_FileName;
    std::wstring m_GroupName;
    bool m_TextureDefault;

    Base::CBuf m_Log;

    Matrix3 m_MatCenter;
    Point3 m_Scale;
    double m_MaxSizeX, m_MaxSizeY, m_MaxSizeZ;
    int m_EdgeExport;  // 0 - off, 1 - all, 2 - optimize
    double m_EdgeExportFactor;  // in gradus

    EGExp2();
    ~EGExp2();

    void GroupClear(void);
    void GroupDelete(CGroup *el);
    int GroupCount(void)
    {
        return m_Groups.size();
    }
    CGroup *GroupFind(CTuple &p1);
    CGroup *GroupFind(CTuple &p1, CTuple &p2);
    CGroup *GroupFind(CTuple &p1, CTuple &p2, CTuple &p3);
    CGroup *GroupAdd(CTuple &p1);
    CGroup *GroupAdd(CTuple &p1, CTuple &p2);
    CGroup *GroupAdd(CTuple &p1, CTuple &p2, CTuple &p3);
    CGroup *GroupFindAdd(CTuple &p1)
    {
        CGroup *g = GroupFind(p1);
        if (g)
            return g;
            
        return GroupAdd(p1);
    }
    CGroup *GroupFindAdd(CTuple &p1, CTuple &p2)
    {
        CGroup *g = GroupFind(p1, p2);
        if (g)
            return g;

        return GroupAdd(p1,p2);
    }
    CGroup *GroupFindAdd(CTuple &p1, CTuple &p2, CTuple &p3)
    {
        CGroup *g = GroupFind(p1, p2, p3);
        if (g)
            return g;

        return GroupAdd(p1, p2, p3);
    }

    void GroupSortByNo(void);

    void Log(const std::wstring &str);

    int GroupMaterialFindAdd(Mtl *ml, int sm, int &twosided);
    int AddVertex(Base::CBuf &buf, SVertexNorTex &v);

    void ExportSimpleGroup(void);
    void ExportSimple(void);
    BOOL ExportSimple_NodeEnum(INode *node, TimeValue timev, int frame);
    void ExportSimple_GeomObject(INode *node, TimeValue timev, int frame);
    void ExportSimple_Helper(INode *node, TimeValue timev, int frame);

    std::wstring RenameByCfg(const std::wstring &str);

    BOOL TMNegParity(Matrix3 &m);
    TriObject *GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
    Point3 GetVertexNormal(Mesh *mesh, int faceNo, RVertex *rv);
};

extern void ExportGroupSetRename(const std::wstring &value, INode *node = nullptr);
extern void ExportGroupGetRename(std::wstring &value, INode *node = nullptr);
extern int ExportGroupCount(INode *node = nullptr);
extern bool ExportGroupGetName(std::wstring &tstr, int no, int &nocur, INode *node = nullptr);
extern bool ExportGroupGetProp(int no, int &nocur, INode *node,
                               std::wstring &name, int &exp, std::wstring &file, bool &textureDefault, int &edgetype, double &edgefactor, std::wstring &anim);
extern bool ExportCenterGetProp(const std::wstring &group, TimeValue timev, INode *node,
                                double &maxsizex, double &maxsizey, double &maxsizez, float &scalex, float &scaley, float &scalez, Matrix3 &center);
extern bool ExportObjGetProp(INode *node, std::wstring &group, int &exp);
extern bool ExportMatrixGetProp(INode *node, const std::wstring & group, int &id, std::wstring &name);

#endif
