#ifndef __UNCORE_H__
#define __UNCORE_H__


#if RENDERING
#	include "CoreGL.h"		//?? for materials only
#endif


// forward declarations
class FArchive;
class UObject;
class UnPackage;

// empty guard macros, if not defined
#ifndef guard
#define guard(x)
#endif

#ifndef unguard
#define unguard
#endif

#ifndef unguardf
#define unguardf(...)
#endif


// field offset macros
// get offset of the field in struc
//#ifdef offsetof
#	define FIELD2OFS(struc, field)		(offsetof(struc, field))				// more compatible
//#else
//#	define FIELD2OFS(struc, field)		((unsigned) &((struc *)NULL)->field)	// just in case
//#endif
// get field of type by offset inside struc
#define OFS2FIELD(struc, ofs, type)	(*(type*) ((byte*)(struc) + ofs))


#define INDEX_NONE			-1


#define FVECTOR_ARG(v)		(v).X, (v).Y, (v).Z
#define FQUAT_ARG(v)		(v).X, (v).Y, (v).Z, (v).W
#define FROTATOR_ARG(r)		(r).Yaw, (r).Pitch, (r).Roll
#define FCOLOR_ARG(v)		(v).R, (v).G, (v).B, (v).A


// detect engine version by package
#define PACKAGE_V2			100
#define PACKAGE_V3			180


#define PACKAGE_FILE_TAG		0x9E2A83C1
#define PACKAGE_FILE_TAG_REV	0xC1832A9E

#if PROFILE
extern int GNumSerialize;
extern int GSerializeBytes;

void appResetProfiler();
void appPrintProfiler();

#define PROFILE_POINT(Label)	appPrintProfiler(); appPrintf("PROFILE: " #Label "\n");

#endif

/*-----------------------------------------------------------------------------
	Game compatibility
-----------------------------------------------------------------------------*/

// BIOSHOCK requires UNREAL3
#if !UNREAL3
#undef BIOSHOCK
#endif

/*-----------------------------------------------------------------------------
	Game directory support
-----------------------------------------------------------------------------*/

void appSetRootDirectory(const char *dir, bool recurse = true);
void appSetRootDirectory2(const char *filename);
const char *appGetRootDirectory();

struct CGameFileInfo
{
	char		RelativeName[256];		// relative to RootDirectory
	const char *ShortFilename;			// without path
	const char *Extension;				// points to extension part (after '.')
	bool		IsPackage;
	int			SizeInKb;				// file size, in kilobytes
	class FObbVFS* FileSystem;			// virtual file system
};

extern int GNumGameFiles;
extern int GNumPackageFiles;
extern int GNumForeignFiles;

// Ext = NULL -> use any package extension
// Filename can contain extension, but should not contain path
const CGameFileInfo *appFindGameFile(const char *Filename, const char *Ext = NULL);
const char *appSkipRootDir(const char *Filename);
FArchive *appCreateFileReader(const CGameFileInfo *info);

typedef bool (*EnumGameFilesCallback_t)(const CGameFileInfo*, void*);
void appEnumGameFilesWorker(EnumGameFilesCallback_t, const char *Ext = NULL, void *Param = NULL);

template<class T>
FORCEINLINE void appEnumGameFiles(bool (*Callback)(const CGameFileInfo*, T&), const char* Ext, T& Param)
{
	appEnumGameFilesWorker((EnumGameFilesCallback_t)Callback, Ext, &Param);
}

template<class T>
FORCEINLINE void appEnumGameFiles(bool (*Callback)(const CGameFileInfo*, T&), T& Param)
{
	appEnumGameFilesWorker((EnumGameFilesCallback_t)Callback, NULL, &Param);
}

FORCEINLINE void appEnumGameFiles(bool (*Callback)(const CGameFileInfo*), const char* Ext = NULL)
{
	appEnumGameFilesWorker((EnumGameFilesCallback_t)Callback, Ext, NULL);
}

#if UNREAL3
extern const char *GStartupPackage;
#endif


/*-----------------------------------------------------------------------------
	FName class
-----------------------------------------------------------------------------*/

const char* appStrdupPool(const char* str);

class FName
{
public:
	int			Index;
#if UNREAL3
	int			ExtraIndex;
	bool		NameGenerated;
#endif
	const char	*Str;

	FName()
	:	Index(0)
	,	Str("None")
#if UNREAL3
	,	ExtraIndex(0)
	,	NameGenerated(false)
#endif
	{}

#if UNREAL3
	void AppendIndex()
	{
		if (NameGenerated || !ExtraIndex) return;
		NameGenerated = true;
		Str = appStrdupPool(va("%s_%d", Str, ExtraIndex-1));
	}
#endif // UNREAL3

#if BIOSHOCK
	void AppendIndexBio()
	{
		if (NameGenerated || !ExtraIndex) return;
		NameGenerated = true;
		Str = appStrdupPool(va("%s%d", Str, ExtraIndex-1));	// without "_" char
	}
#endif // BIOSHOCK

	inline FName& operator=(const FName &Other)
	{
		Index = Other.Index;
		Str   = Other.Str;
#if UNREAL3
		NameGenerated = Other.NameGenerated;
		if (Other.NameGenerated)
		{
			// should duplicate generated names to avoid crash in destructor
			Str = appStrdupPool(Other.Str);
		}
#endif // UNREAL3
		return *this;
	}

	inline bool operator==(const FName& Other) const
	{
		return (Str == Other.Str) || (stricmp(Str, Other.Str) == 0);
	}

	FORCEINLINE const char *operator*() const
	{
		return Str;
	}
	FORCEINLINE operator const char*() const
	{
		return Str;
	}
};


/*-----------------------------------------------------------------------------
	FCompactIndex class for serializing objects in a compactly, mapping
	small values to fewer bytes.
-----------------------------------------------------------------------------*/

class FCompactIndex
{
public:
	int		Value;
	friend FArchive& operator<<(FArchive &Ar, FCompactIndex &I);
};

#define AR_INDEX(intref)	(*(FCompactIndex*)&(intref))


/*-----------------------------------------------------------------------------
	FArchive class
-----------------------------------------------------------------------------*/

enum EGame
{
	GAME_UNKNOWN   = 0,			// should be 0

	GAME_UE1       = 0x01000,
		GAME_Undying,

	GAME_UE2       = 0x02000,
		GAME_UT2,
		GAME_Pariah,
		GAME_SplinterCell,
		GAME_SplinterCellConv,
		GAME_Lineage2,
		GAME_Exteel,
		GAME_Ragnarok2,
		GAME_RepCommando,
		GAME_Loco,
		GAME_BattleTerr,
		GAME_UC1,				// note: not UE2X
		GAME_XIII,
		GAME_Vanguard,
		GAME_AA2,

	GAME_VENGEANCE = 0x02100,	// variant of UE2
		GAME_Tribes3,
		GAME_Swat4,				// not autodetected, overlaps with Tribes3
		GAME_Bioshock,

	GAME_LEAD      = 0x02200,

	GAME_UE2X      = 0x04000,
		GAME_UC2,

	GAME_UE3       = 0x08000,
		GAME_EndWar,
		GAME_MassEffect,
		GAME_MassEffect2,
		GAME_MassEffect3,
		GAME_R6Vegas2,
		GAME_MirrorEdge,
		GAME_TLR,
		GAME_Huxley,
		GAME_Turok,
		GAME_Fury,
		GAME_XMen,
		GAME_MagnaCarta,
		GAME_ArmyOf2,
		GAME_CrimeCraft,
		GAME_50Cent,
		GAME_AVA,
		GAME_Frontlines,
		GAME_Batman,
		GAME_Batman2,
		GAME_Batman3,
		GAME_Borderlands,
		GAME_AA3,
		GAME_DarkVoid,
		GAME_Legendary,
		GAME_Tera,
		GAME_BladeNSoul,
		GAME_APB,
		GAME_AlphaProtocol,
		GAME_Transformers,
		GAME_MortalOnline,
		GAME_Enslaved,
		GAME_MOHA,
		GAME_MOH2010,
		GAME_Berkanix,
		GAME_DOH,
		GAME_DCUniverse,
		GAME_Bulletstorm,
		GAME_Undertow,
		GAME_Singularity,
		GAME_Tron,
		GAME_Hunted,
		GAME_DND,
		GAME_ShadowsDamned,
		GAME_Argonauts,
		GAME_SpecialForce2,
		GAME_GunLegend,
		GAME_TaoYuan,
		GAME_Tribes4,
		GAME_Dishonored,
		GAME_Hawken,
		GAME_Fable,
		GAME_DmC,
		GAME_PLA,
		GAME_AliensCM,
		GAME_GoWJ,
		GAME_Bioshock3,
		GAME_RememberMe,
		GAME_MarvelHeroes,
		GAME_LostPlanet3,
		GAME_XcomB,
		GAME_Thief4,
		GAME_Murdered,
		GAME_SOV,
		GAME_VEC,
		GAME_Dust514,

	GAME_MIDWAY3   = 0x08100,	// variant of UE3
		GAME_A51,
		GAME_Wheelman,
		GAME_MK,
		GAME_Strangle,
		GAME_TNA,

	GAME_UE4       = 0x10000,
		// engine versions
		GAME_UE4_0,
		GAME_UE4_1,
		GAME_UE4_2,
		GAME_UE4_3,
		GAME_UE4_4,
		GAME_UE4_5,
		// games

	GAME_ENGINE    = 0xFFF00	// mask for game engine
};

enum EPlatform
{
	PLATFORM_UNKNOWN = 0,
	PLATFORM_PC,
	PLATFORM_XBOX360,
	PLATFORM_PS3,
	PLATFORM_IOS,

	PLATFORM_COUNT,
};


class FArchive
{
public:
	bool	IsLoading;
	int		ArVer;
	int		ArLicenseeVer;
	bool	ReverseBytes;

protected:
	int		ArPos;
	int		ArStopper;

public:
	// game-specific flags
	int		Game;				// EGame
	int		Platform;			// EPlatform

	FArchive()
	:	ArPos(0)
	,	ArStopper(0)
	,	ArVer(100000)			//?? something large
	,	ArLicenseeVer(0)
	,	ReverseBytes(false)
	,	Game(GAME_UNKNOWN)
	,	Platform(PLATFORM_PC)
	{}

	virtual ~FArchive()
	{}

	void DetectGame();
	void OverrideVersion();

	inline int Engine() const
	{
		return (Game & GAME_ENGINE);
	}

	void SetupFrom(const FArchive &Other)
	{
		ArVer         = Other.ArVer;
		ArLicenseeVer = Other.ArLicenseeVer;
		ReverseBytes  = Other.ReverseBytes;
		Game          = Other.Game;
		Platform      = Other.Platform;
	}

	virtual void Seek(int Pos) = 0;
	void Seek64(int64 Pos)
	{
		//!! change when support large files; perhaps Seek() should call Seek64
		if (Pos >= (1LL << 31)) appError("Seek64 %I64X", Pos);
		Seek((int)Pos);
	}
	virtual bool IsEof() const
	{
		return false;
	}

	virtual int Tell() const
	{
		return ArPos;
	}

	virtual void Serialize(void *data, int size) = 0;
	void ByteOrderSerialize(void *data, int size);

	void Printf(const char *fmt, ...);

	virtual void SetStopper(int Pos)
	{
		ArStopper = Pos;
	}

	virtual int GetStopper() const
	{
		return ArStopper;
	}

	bool IsStopper()
	{
		return Tell() == GetStopper();
	}

	virtual int GetFileSize() const
	{
		return 0;
	}

	virtual bool IsOpen() const
	{
		return true;
	}
	virtual bool Open()
	{
		return true;
	}
	virtual void Close()
	{
	}

	virtual FArchive& operator<<(FName &N)
	{
		return *this;
	}
	virtual FArchive& operator<<(UObject *&Obj)
	{
		return *this;
	}

	// some typeinfo

	static const char* StaticGetName()
	{
		return "FArchive";
	}

	virtual const char* GetName() const
	{
		return StaticGetName();
	}

	virtual bool IsA(const char* type)
	{
		return !strcmp(type, "FArchive");
	}

	template<class T> T* CastTo()
	{
		if (IsA(T::StaticGetName()))
			return static_cast<T*>(this);
		else
			return NULL;
	}
};

#define DECLARE_ARCHIVE(Class,Base)		\
	typedef Class	ThisClass;			\
	typedef Base	Super;				\
public:									\
	static const char* StaticGetName()	\
	{									\
		return #Class;					\
	}									\
	virtual const char* GetName() const	\
	{									\
		return StaticGetName();			\
	}									\
	virtual bool IsA(const char* type)	\
	{									\
		return !strcmp(type, StaticGetName()) || Super::IsA(type); \
	}									\
private:


// Booleans in UE are serialized as int32
FORCEINLINE FArchive& operator<<(FArchive &Ar, bool &B)
{
	int b32 = B;
	Ar.Serialize(&b32, 4);
	if (Ar.IsLoading) B = (b32 != 0);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, char &B)
{
	Ar.Serialize(&B, 1);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, byte &B)
{
	Ar.Serialize(&B, 1);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, short &B)
{
	Ar.ByteOrderSerialize(&B, 2);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, word &B)
{
	Ar.ByteOrderSerialize(&B, 2);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, int &B)
{
	Ar.ByteOrderSerialize(&B, 4);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, unsigned &B)
{
	Ar.ByteOrderSerialize(&B, 4);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, int64 &B)
{
	Ar.ByteOrderSerialize(&B, 8);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, uint64 &B)
{
	Ar.ByteOrderSerialize(&B, 8);
	return Ar;
}
FORCEINLINE FArchive& operator<<(FArchive &Ar, float &B)
{
	Ar.ByteOrderSerialize(&B, 4);
	return Ar;
}


enum EFileReaderOptions
{
	FRO_NoOpenError = 1,
};


class FFileArchive : public FArchive
{
	DECLARE_ARCHIVE(FFileArchive, FArchive);
public:
	FFileArchive(const char *Filename, unsigned InOptions);
	virtual ~FFileArchive();

	virtual void Seek(int Pos);
	virtual bool IsEof() const;
	virtual FArchive& operator<<(FName &N);
	virtual FArchive& operator<<(UObject *&Obj);
	virtual bool IsOpen() const;
	virtual void Close();

protected:
	FILE		*f;
	unsigned	Options;
	const char	*FullName;		// allocared with appStrdup
	const char	*ShortName;		// points to FullName[N]
	int			FileSize;

	byte*		Buffer;
	int			BufferSize;
	int			BufferPos;
	int			FilePos;

	bool OpenFile(const char *Mode);
};


inline bool appFileExists(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	if (f)
	{
		fclose(f);
		return true;
	}
	return false;
}


class FFileReader : public FFileArchive
{
	DECLARE_ARCHIVE(FFileReader, FFileArchive);
public:
	FFileReader(const char *Filename, unsigned InOptions = 0);
	virtual ~FFileReader();

	virtual void Serialize(void *data, int size);
	virtual bool Open();
	virtual int GetFileSize() const;
};


class FFileWriter : public FFileArchive
{
	DECLARE_ARCHIVE(FFileWriter, FFileArchive);
public:
	FFileWriter(const char *Filename, unsigned Options = 0);
	virtual ~FFileWriter();

	virtual void Serialize(void *data, int size);
	virtual bool Open();
	virtual void Close();
	virtual int GetFileSize() const;

protected:
	void FlushBuffer();
};


// NOTE: this class should work well as a writer too!
class FReaderWrapper : public FArchive
{
	DECLARE_ARCHIVE(FReaderWrapper, FArchive);
public:
	FArchive	*Reader;
	int			ArPosOffset;

	FReaderWrapper(FArchive *File, int Offset = 0)
	:	Reader(File)
	,	ArPosOffset(Offset)
	{}
	virtual ~FReaderWrapper()
	{
		delete Reader;
	}
	virtual void Seek(int Pos)
	{
		Reader->Seek(Pos + ArPosOffset);
	}
	virtual int Tell() const
	{
		return Reader->Tell() - ArPosOffset;
	}
	virtual int GetFileSize() const
	{
		return Reader->GetFileSize() - ArPosOffset;
	}
	virtual void SetStopper(int Pos)
	{
		Reader->SetStopper(Pos + ArPosOffset);
	}
	virtual int GetStopper() const
	{
		return Reader->GetStopper() - ArPosOffset;
	}
	virtual bool IsOpen() const
	{
		return Reader->IsOpen();
	}
	virtual bool Open()
	{
		return Reader->Open();
	}
	virtual void Close()
	{
		Reader->Close();
	}
};


class FMemReader : public FArchive
{
	DECLARE_ARCHIVE(FMemReader, FArchive);
public:
	FMemReader(const void *data, int size)
	:	DataPtr((const byte*)data)
	,	DataSize(size)
	{
		IsLoading = true;
		ArStopper = size;
	}

	virtual void Seek(int Pos)
	{
		guard(FMemReader::Seek);
		assert(Pos >= 0 && Pos <= DataSize);
		ArPos = Pos;
		unguard;
	}

	virtual bool IsEof() const
	{
		return ArPos >= DataSize;
	}

	virtual void Serialize(void *data, int size)
	{
		guard(FMemReader::Serialize);
		if (ArStopper > 0 && ArPos + size > ArStopper)
			appError("Serializing behind stopper (%X+%X > %X)", ArPos, size, ArStopper);
		if (ArPos + size > DataSize)
			appError("Serializing behind end of buffer");
		memcpy(data, DataPtr + ArPos, size);
		ArPos += size;
		unguard;
	}

	virtual int GetFileSize() const
	{
		return DataSize;
	}

protected:
	const byte *DataPtr;
	int		DataSize;
};


// drop remaining object data (until stopper)
#define DROP_REMAINING_DATA(Ar)							\
	Ar.Seek(Ar.GetStopper());

// research helper
inline void DUMP_ARC_BYTES(FArchive &Ar, int NumBytes)
{
	int OldPos = Ar.Tell();
	for (int i = 0; i < NumBytes; i++)
	{
		if (Ar.IsStopper() || Ar.IsEof()) break;
		if (!(i & 31)) appPrintf("\n%06X :", OldPos+i);
		if (!(i & 3)) appPrintf(" ");
		byte b;
		Ar << b;
		appPrintf(" %02X", b);
	}
	appPrintf("\n");
	Ar.Seek(OldPos);
}

inline void DUMP_MEM_BYTES(const void* Data, int NumBytes)
{
	const byte* b = (byte*)Data;
	for (int i = 0; i < NumBytes; i++)
	{
		if (!(i & 31)) appPrintf("\n%08X :", b);
		if (!(i & 3)) appPrintf(" ");
		appPrintf(" %02X", *b++);
	}
	appPrintf("\n");
}


// Reverse byte order for data array, inplace
void appReverseBytes(void *Block, int NumItems, int ItemSize);


/*-----------------------------------------------------------------------------
	Math classes
-----------------------------------------------------------------------------*/

struct FVector
{
	float	X, Y, Z;

	void Set(float _X, float _Y, float _Z)
	{
		X = _X; Y = _Y; Z = _Z;
	}

	void Scale(float value)
	{
		X *= value; Y *= value; Z *= value;
	}

	friend FArchive& operator<<(FArchive &Ar, FVector &V)
	{
		Ar << V.X << V.Y << V.Z;
#if ENDWAR
		if (Ar.Game == GAME_EndWar) Ar.Seek(Ar.Tell() + 4);	// skip W, at ArVer >= 290
#endif
		return Ar;
	}
};

#if 1

FORCEINLINE bool operator==(const FVector &V1, const FVector &V2)
{
	return V1.X == V2.X && V1.Y == V2.Y && V1.Z == V2.Z;
}

#else

FORCEINLINE bool operator==(const FVector &V1, const FVector &V2)
{
	const int* p1 = (int*)&V1;
	const int* p2 = (int*)&V2;
	return ( (p1[0] ^ p2[0]) | (p1[1] ^ p2[1]) | (p1[2] ^ p2[2]) ) == 0;
}

#endif


struct FRotator
{
	int		Pitch, Yaw, Roll;

	void Set(int _Yaw, int _Pitch, int _Roll)
	{
		Pitch = _Pitch; Yaw = _Yaw; Roll = _Roll;
	}

	friend FArchive& operator<<(FArchive &Ar, FRotator &R)
	{
		Ar << R.Pitch << R.Yaw << R.Roll;
#if TNA_IMPACT
		if (Ar.Game == GAME_TNA && Ar.ArVer >= 395)
		{
			// WWE All Stars: this game has strange FRotator values; found formulas below experimentally
			R.Pitch = R.Pitch / 65536;
			R.Yaw   = R.Yaw   / 65536 - 692;
			R.Roll  = R.Roll  / 65536 - 692;
		}
#endif // TNA_IMPACT
		return Ar;
	}
};


struct FQuat
{
	float	X, Y, Z, W;

	void Set(float _X, float _Y, float _Z, float _W)
	{
		X = _X; Y = _Y; Z = _Z; W = _W;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuat &F)
	{
		return Ar << F.X << F.Y << F.Z << F.W;
	}
};


struct FCoords
{
	FVector	Origin;
	FVector	XAxis;
	FVector	YAxis;
	FVector	ZAxis;

	friend FArchive& operator<<(FArchive &Ar, FCoords &F)
	{
		return Ar << F.Origin << F.XAxis << F.YAxis << F.ZAxis;
	}
};


struct FBox
{
	FVector	Min;
	FVector	Max;
	byte	IsValid;

	friend FArchive& operator<<(FArchive &Ar, FBox &Box)
	{
#if UC2
		if (Ar.Engine() == GAME_UE2X && Ar.ArVer >= 146)
			return Ar << Box.Min << Box.Max;
#endif // UC2
		return Ar << Box.Min << Box.Max << Box.IsValid;
	}
};


struct FSphere : public FVector
{
	float	R;

	friend FArchive& operator<<(FArchive &Ar, FSphere &S)
	{
		Ar << (FVector&)S;
		if (Ar.ArVer >= 61)
			Ar << S.R;
		return Ar;
	};
};


struct FPlane : public FVector
{
	float	W;

	friend FArchive& operator<<(FArchive &Ar, FPlane &S)
	{
		return Ar << (FVector&)S << S.W;
	};
};


struct FMatrix
{
	FPlane	XPlane;
	FPlane	YPlane;
	FPlane	ZPlane;
	FPlane	WPlane;

	friend FArchive& operator<<(FArchive &Ar, FMatrix &F)
	{
		return Ar << F.XPlane << F.YPlane << F.ZPlane << F.WPlane;
	}
};


class FScale
{
public:
	FVector	Scale;
	float	SheerRate;
	byte	SheerAxis;	// ESheerAxis

	// Serializer.
	friend FArchive& operator<<(FArchive &Ar, FScale &S)
	{
		return Ar << S.Scale << S.SheerRate << S.SheerAxis;
	}
};


struct FColor
{
	byte	R, G, B, A;

	FColor()
	{}
	FColor(byte r, byte g, byte b, byte a = 255)
	:	R(r), G(g), B(b), A(a)
	{}
	friend FArchive& operator<<(FArchive &Ar, FColor &C)
	{
		return Ar << C.R << C.G << C.B << C.A;
	}
};


// UNREAL3
struct FLinearColor
{
	float	R, G, B, A;

	void Set(float _R, float _G, float _B, float _A)
	{
		R = _R; G = _G; B = _B; A = _A;
	}

	friend FArchive& operator<<(FArchive &Ar, FLinearColor &C)
	{
		return Ar << C.R << C.G << C.B << C.A;
	}
};


/*-----------------------------------------------------------------------------
	Typeinfo for fast array serialization
-----------------------------------------------------------------------------*/

// Default typeinfo
template<class T> struct TTypeInfo
{
	enum { FieldSize = sizeof(T) };
	enum { NumFields = 1         };
	enum { IsSimpleType = 0      };		// type consists of NumFields fields of integral type, sizeof(type) == FieldSize
	enum { IsRawType = 0         };		// type memory layour is the same as archive layout
	enum { IsPod = IS_POD(T)     };		// type has no constructor/destructor
};


template<class T1, class T2> struct IsSameType
{
	enum { Value = 0 };
};

template<class T> struct IsSameType<T,T>
{
	enum { Value = 1 };
};


// Declare type, consists from fields of the same type length
// (e.g. from ints and floats, or from chars and bytes etc), and
// which memory layout is the same as disk layout (if endian of
// package and platform is the same)
#define SIMPLE_TYPE(Type,BaseType)			\
template<> struct TTypeInfo<Type>			\
{											\
	enum { FieldSize = sizeof(BaseType) };	\
	enum { NumFields = sizeof(Type) / sizeof(BaseType) }; \
	enum { IsSimpleType = 1 };				\
	enum { IsRawType = 1 };					\
	enum { IsPod = 1 };						\
};


// Declare type, which memory layout is the same as disk layout
#define RAW_TYPE(Type)						\
template<> struct TTypeInfo<Type>			\
{											\
	enum { FieldSize = sizeof(Type) };		\
	enum { NumFields = 1 };					\
	enum { IsSimpleType = 0 };				\
	enum { IsRawType = 1 };					\
	enum { IsPod = 1 };						\
};


// Note: SIMPLE and RAW types does not need constructors for loading
//!! add special mode to check SIMPLE/RAW types (serialize twice and compare results)

#if 0
//!! testing
#undef  SIMPLE_TYPE
#undef  RAW_TYPE
#define SIMPLE_TYPE(x,y)
#define RAW_TYPE(x)
#endif


// Declare fundamental types
SIMPLE_TYPE(bool,     bool)
SIMPLE_TYPE(byte,     byte)
SIMPLE_TYPE(char,     char)
SIMPLE_TYPE(short,    short)
SIMPLE_TYPE(word,     word)
SIMPLE_TYPE(int,      int)
SIMPLE_TYPE(unsigned, unsigned)
SIMPLE_TYPE(float,    float)
SIMPLE_TYPE(int64,    int64)
SIMPLE_TYPE(uint64,   uint64)

// Aggregates
SIMPLE_TYPE(FVector, float)
SIMPLE_TYPE(FQuat,   float)
SIMPLE_TYPE(FCoords, float)
SIMPLE_TYPE(FColor,  byte)


/*-----------------------------------------------------------------------------
	TArray/TLazyArray templates
-----------------------------------------------------------------------------*/

/*
 * NOTES:
 *	- FArray/TArray should not contain objects with virtual tables (no
 *	  constructor/destructor support)
 *	- should not use new[] and delete[] here, because compiler will alloc
 *	  additional 'count' field for correct delete[], but we uses appMalloc/
 *	  appFree calls.
 */

class FArray
{
	friend struct CTypeInfo;

public:
	FORCEINLINE FArray()
	:	DataCount(0)
	,	MaxCount(0)
	,	DataPtr(NULL)
	{}
	~FArray();

	FORCEINLINE void *GetData()
	{
		return DataPtr;
	}
	FORCEINLINE const void *GetData() const
	{
		return DataPtr;
	}
	FORCEINLINE int Num() const
	{
		return DataCount;
	}

	void RawCopy(const FArray &Src, int elementSize);

	// serializers
	FArchive& SerializeSimple(FArchive &Ar, int NumFields, int FieldSize);
	FArchive& SerializeRaw(FArchive &Ar, void (*Serializer)(FArchive&, void*), int elementSize);

protected:
	void	*DataPtr;
	int		DataCount;
	int		MaxCount;

	// helper for TStaticArray and FStaticString
	FORCEINLINE bool IsStatic() const
	{
		return DataPtr == (void*)(this + 1);
	}

	// serializers
	FArchive& Serialize(FArchive &Ar, void (*Serializer)(FArchive&, void*), int elementSize);

	void Empty (int count, int elementSize);
	void Add   (int count, int elementSize);
	void Insert(int index, int count, int elementSize);
	// remove items and then move next items to the position of removed items
	void Remove(int index, int count, int elementSize);
	// remove items and then fill the hole with items from array end
	void FastRemove(int index, int count, int elementSize);

	void* GetItem(int index, int elementSize) const;
};

#if DECLARE_VIEWER_PROPS
#define ARRAY_COUNT_FIELD_OFFSET	( sizeof(void*) )	// offset of DataCount field inside TArray structure
#endif


FArchive& SerializeLazyArray(FArchive &Ar, FArray &Array, FArchive& (*Serializer)(FArchive&, void*));
#if UNREAL3
FArchive& SerializeRawArray(FArchive &Ar, FArray &Array, FArchive& (*Serializer)(FArchive&, void*));
#endif


// NOTE: this container cannot hold objects, required constructor/destructor
// (at least, Add/Insert/Remove functions are not supported, but can serialize
// such data)
template<class T> class TArray : public FArray
{
public:
	TArray()
	:	FArray()
	{}
	~TArray()
	{
		// destruct all array items
		if (!TTypeInfo<T>::IsPod) Destruct(0, DataCount);
	}
	// data accessors

#if !DO_ASSERT
	// version without verifications, very compact
	FORCEINLINE T& operator[](int index)
	{
		return *((T*)DataPtr + index);
	}
	FORCEINLINE const T& operator[](int index) const
	{
		return *((T*)DataPtr + index);
	}
#elif DO_GUARD_MAX
	// version with guardfunc instead of guard
	T& operator[](int index)
	{
		guardfunc;
		assert(index >= 0 && index < DataCount);
		return *((T*)DataPtr + index);
		unguardf("%d/%d", index, DataCount);
	}
	const T& operator[](int index) const
	{
		guardfunc;
		assert(index >= 0 && index < DataCount);
		return *((T*)DataPtr + index);
		unguardf("%d/%d", index, DataCount);
	}
#else // DO_ASSERT && !DO_GUARD_MAX
	// common implementation for all types
	FORCEINLINE T& operator[](int index)
	{
		return *(T*)GetItem(index, sizeof(T));
	}
	FORCEINLINE const T& operator[](int index) const
	{
		return *(T*)GetItem(index, sizeof(T));
	}
#endif

	FORCEINLINE int Add(int count = 1)
	{
		int index = DataCount;
		Insert(index, count);
		return index;
	}

	FORCEINLINE void Insert(int index, int count = 1)
	{
		FArray::Insert(index, count, sizeof(T));
		if (!TTypeInfo<T>::IsPod) Construct(index, count);
	}

	FORCEINLINE void Remove(int index, int count = 1)
	{
		// destruct specified array items
		if (!TTypeInfo<T>::IsPod) Destruct(index, count);
		// remove items from array
		FArray::Remove(index, count, sizeof(T));
	}

	// Remove an item and copy last array's item(s) to the removed item position,
	// so no array shifting performed. Could be used when order of array elements
	// is not important.
	FORCEINLINE void FastRemove(int index, int count = 1)
	{
		// destruct specified array items
		if (!TTypeInfo<T>::IsPod) Destruct(index, count);
		// remove items from array
		FArray::FastRemove(index, count, sizeof(T));
	}

	int AddItem(const T& item)
	{
		int index = Add();
		Item(index) = item;
		return index;
	}

	FORCEINLINE T& AddItem()
	{
		int index = Add();
		return Item(index);
	}

	int FindItem(const T& item, int startIndex = 0) const
	{
#if 1
		const T *P;
		int i;
		for (i = startIndex, P = (const T*)DataPtr + startIndex; i < DataCount; i++, P++)
			if (*P == item)
				return i;
#else
		for (int i = startIndex; i < DataCount; i++)
			if (*((T*)DataPtr + i) == item)
				return i;
#endif
		return INDEX_NONE;
	}

	FORCEINLINE void Empty(int count = 0)
	{
		// destruct all array items
		if (!TTypeInfo<T>::IsPod) Destruct(0, DataCount);
		// remove data array (count=0) or preallocate memory (count>0)
		FArray::Empty(count, sizeof(T));
	}

	FORCEINLINE void FastEmpty()
	{
		// destruct all array items
		if (!TTypeInfo<T>::IsPod) Destruct(0, DataCount);
		// set DataCount to 0 without reallocation
		DataCount = 0;
	}

	FORCEINLINE void Sort(int (*cmpFunc)(const T*, const T*))
	{
		QSort<T>((T*)DataPtr, DataCount, cmpFunc);
	}

	// serializer
	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, TArray &A)
	{
#if DO_GUARD_MAX
		guardfunc;
#endif
		// special case for SIMPLE_TYPE
		if (TTypeInfo<T>::IsSimpleType)
		{
			staticAssert(sizeof(T) == TTypeInfo<T>::NumFields * TTypeInfo<T>::FieldSize, Error_In_TypeInfo);
			return A.SerializeSimple(Ar, TTypeInfo<T>::NumFields, TTypeInfo<T>::FieldSize);
		}

		// special case for RAW_TYPE
		if (TTypeInfo<T>::IsRawType)
			return A.SerializeRaw(Ar, TArray<T>::SerializeItem, sizeof(T));

		// generic case
		// erase previous data before loading in a case of non-POD data
		if (!TTypeInfo<T>::IsPod && Ar.IsLoading)
			A.Destruct(0, A.Num());		// do not call Empty() - data will be freed anyway in FArray::Serialize()
		return A.Serialize(Ar, TArray<T>::SerializeItem, sizeof(T));
#if DO_GUARD_MAX
		unguard;
#endif
	}

	// serializer helper; used from 'operator<<(FArchive, TArray<>)' only
	static void SerializeItem(FArchive &Ar, void *item)
	{
		if (!TTypeInfo<T>::IsPod && Ar.IsLoading)
			new (item) T;		// construct item before reading
		Ar << *(T*)item;		// serialize item
	}

private:
	// disable array copying
	TArray(const TArray &Other)
	:	FArray()
	{}
	TArray& operator=(const TArray &Other)
	{
		return this;
	}
	// fast version of operator[] without assertions (may be used in safe code)
	FORCEINLINE T& Item(int index)
	{
		return *((T*)DataPtr + index);
	}
	FORCEINLINE const T& Item(int index) const
	{
		return *((T*)DataPtr + index);
	}
	void Construct(int index, int count)
	{
		for (int i = 0; i < count; i++)
			new ((T*)DataPtr + index + i) T;
	}
	void Destruct(int index, int count)
	{
		for (int i = 0; i < count; i++)
			((T*)DataPtr + index + i)->~T();
	}
};

template<class T> inline void Exchange(TArray<T>& A, TArray<T>& B)
{
	const int size = sizeof(TArray<T>);
	byte buffer[size];
	memcpy(buffer, &A, size);
	memcpy(&A, &B, size);
	memcpy(&B, buffer, size);
}

// Binary-compatible array, but with no allocations inside
template<class T, int N> class TStaticArray : public TArray<T>
{
	// We require "using TArray<T>::*" for gcc 3.4+ compilation
	// http://gcc.gnu.org/gcc-3.4/changes.html
	// - look for "unqualified names"
	// - "temp.dep/3" section of the C++ standard [ISO/IEC 14882:2003]
	using TArray<T>::DataPtr;
	using TArray<T>::MaxCount;
public:
	FORCEINLINE TStaticArray()
	{
		DataPtr = (void*)&StaticData[0];
		MaxCount = N;
	}

protected:
	T		StaticData[N];
};

template<class T> FORCEINLINE void* operator new(size_t size, TArray<T> &Array)
{
	guard(TArray::operator new);
	assert(size == sizeof(T));
	return &Array.AddItem();
	unguard;
}


// Skip array of items of fixed size
void SkipFixedArray(FArchive &Ar, int ItemSize);


// TLazyArray implemented as simple wrapper around TArray with
// different serialization function
// Purpose in UE: array with can me loaded asynchronously (when serializing
// it 1st time only disk position is remembered, and later array can be
// read from file when needed)

template<class T> class TLazyArray : public TArray<T>
{
	// Helper function to reduce TLazyArray<>::operator<<() code size.
	// Used as C-style wrapper around TArray<>::operator<<().
	static FArchive& SerializeArray(FArchive &Ar, void *Array)
	{
		return Ar << *(TArray<T>*)Array;
	}

#if DO_GUARD_MAX
	friend FArchive& operator<<(FArchive &Ar, TLazyArray &A)
	{
		guardfunc;
		return SerializeLazyArray(Ar, A, SerializeArray);
		unguard;
	}
#else
	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, TLazyArray &A)
	{
		return SerializeLazyArray(Ar, A, SerializeArray);
	}
#endif
};


void SkipLazyArray(FArchive &Ar);


#if UNREAL3

// NOTE: real class name is unknown; other suitable names: TCookedArray, TPodArray.
// Purpose in UE: array, which file contents exactly the same as in-memory
// contents. Whole array can be read using single read call. Package
// engine version should equals to game engine version, otherwise per-element
// reading will be performed (as usual in TArray)
// There is no reading optimization performed here (in umodel)

template<class T> class TRawArray : protected TArray<T>
{
public:
	// Helper function to reduce TRawArray<>::operator<<() code size.
	// Used as C-style wrapper around TArray<>::operator<<().
	static FArchive& SerializeArray(FArchive &Ar, void *Array)
	{
		return Ar << *(TArray<T>*)Array;
	}

#if DO_GUARD_MAX
	friend FArchive& operator<<(FArchive &Ar, TRawArray &A)
	{
		guardfunc;
		return SerializeRawArray(Ar, A, SerializeArray);
		unguard;
	}
#else
	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, TRawArray &A)
	{
		return SerializeRawArray(Ar, A, SerializeArray);
	}
#endif

protected:
	// disallow direct creation of TRawArray, this is a helper class with a
	// different serializer
	TRawArray()
	{}
};

void SkipRawArray(FArchive &Ar, int Size);

// helper function for RAW_ARRAY macro
template<class T> inline TRawArray<T>& ToRawArray(TArray<T> &Arr)
{
	return (TRawArray<T>&)Arr;
}

#define RAW_ARRAY(Arr)		ToRawArray(Arr)

#endif // UNREAL3

template<typename T1, typename T2> inline void CopyArray(TArray<T1> &Dst, const TArray<T2> &Src)
{
	if (IsSameType<T1,T2>::Value && TTypeInfo<T1>::IsPod)
	{
		// fast version when copying POD type array
		Dst.RawCopy(Src, sizeof(T1));
		return;
	}

	// copying 2 different types, or non-POD type
	int Count = Src.Num();
	Dst.Empty(Count);
	if (Count)
	{
		Dst.Add(Count);
		T1 *pDst = (T1*)Dst.GetData();
		T2 *pSrc = (T2*)Src.GetData();
		do		// Count is > 0 here - checked above, so "do ... while" is more suitable (and more compact)
		{
			*pDst++ = *pSrc++;
		} while (--Count);
	}
}


/*-----------------------------------------------------------------------------
	TMap template
-----------------------------------------------------------------------------*/

// Very simple class, required only for serialization
template<class TK, class TV> struct TMapPair
{
	TK		Key;
	TV		Value;

	friend FArchive& operator<<(FArchive &Ar, TMapPair &V)
	{
		return Ar << V.Key << V.Value;
	}
};


template<class TK, class TV> class TMap : public TArray<TMapPair<TK, TV> >
{
public:
	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, TMap &Map)
	{
		return Ar << (TArray<TMapPair<TK, TV> >&)Map;
	}
};

template<class TK, class TV, int N> class TStaticMap : public TStaticArray<TMapPair<TK, TV>, N>
{
public:
	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, TStaticMap &Map)
	{
		return Ar << (TStaticArray<TMapPair<TK, TV>, N>&)Map;
	}
};


/*-----------------------------------------------------------------------------
	FString
-----------------------------------------------------------------------------*/

class FString : public TArray<char>
{
public:
	FString()
	{}
	FString(const char* src);

	FString& operator=(const char* src);
	FORCEINLINE FString& operator=(const FString& src)
	{
		return operator=(*src);
	}

	FString& operator+=(const char* text);

	// use FString as allocated char*, FString became empty and will not free
	// detached string in destructor
	char* Detach();

	FORCEINLINE bool IsEmpty()
	{
		return Num() <= 1;
	}

	// convert string to char* - use "*Str"
	//!! WARNING: could crash if string is empty
	FORCEINLINE const char *operator*() const
	{
		return (char*)DataPtr;
	}
	FORCEINLINE operator const char*() const
	{
		return (char*)DataPtr;
	}
	// comparison
	FORCEINLINE bool operator==(const FString& other) const
	{
		return !strcmp((char*)DataPtr, (char*)other.DataPtr);
	}
	FORCEINLINE bool operator==(const char* other) const
	{
		return !strcmp((char*)DataPtr, other);
	}

	friend FArchive& operator<<(FArchive &Ar, FString &S);
};

// Binary-compatible string, but with no allocations inside
template<int N> class FStaticString : public FString
{
public:
	FORCEINLINE FStaticString()
	{
		DataPtr = (void*)&StaticData[0];
		MaxCount = N;
	}

	// operators
	FORCEINLINE FString& operator=(const char* src)
	{
		return (FStaticString&) FString::operator=(src);
	}
	FORCEINLINE FString& operator=(const FString& src)
	{
		return (FStaticString&) FString::operator=(src);
	}

protected:
	char	StaticData[N];
};


/*-----------------------------------------------------------------------------
	Guid
-----------------------------------------------------------------------------*/

class FGuid
{
public:
	unsigned	A, B, C, D;

	friend FArchive& operator<<(FArchive &Ar, FGuid &G)
	{
		Ar << G.A << G.B << G.C << G.D;
#if FURY
		if (Ar.Game == GAME_Fury && Ar.ArLicenseeVer >= 24)
		{
			int guid5;
			Ar << guid5;
		}
#endif // FURY
		return Ar;
	}

	FORCEINLINE bool operator==(const FGuid& Other) const
	{
		return memcmp(this, &Other, sizeof(FGuid)) == 0;
	}
};


#if UNREAL3

/*-----------------------------------------------------------------------------
	Support for UE3 compressed files
-----------------------------------------------------------------------------*/

struct FCompressedChunkBlock
{
	int			CompressedSize;
	int			UncompressedSize;

	friend FArchive& operator<<(FArchive &Ar, FCompressedChunkBlock &B)
	{
#if UNREAL4
		if (Ar.Game >= GAME_UE4)
		{
			// UE4 has 64-bit values here
			int64 CompressedSize64, UncompressedSize64;
			Ar << CompressedSize64 << UncompressedSize64;
			assert((CompressedSize64 | UncompressedSize64) <= 0x7FFFFFFF); // we're using 32 bit values
			B.CompressedSize = (int)CompressedSize64;
			B.UncompressedSize = (int)UncompressedSize64;
			return Ar;
		}
#endif // UNREAL4
		return Ar << B.CompressedSize << B.UncompressedSize;
	}
};

struct FCompressedChunkHeader
{
	int			Tag;
	int			BlockSize;				// maximal size of uncompressed block
	FCompressedChunkBlock Sum;			// summary for the whole compressed block
	TArray<FCompressedChunkBlock> Blocks;

	friend FArchive& operator<<(FArchive &Ar, FCompressedChunkHeader &H);
};

void appReadCompressedChunk(FArchive &Ar, byte *Buffer, int Size, int CompressionFlags);


/*-----------------------------------------------------------------------------
	UE3/UE4 bulk data - replacement for TLazyArray
-----------------------------------------------------------------------------*/

// UE3
#define BULKDATA_StoreInSeparateFile	0x01		// bulk stored in different file
#define BULKDATA_CompressedZlib			0x02		// unknown name
#define BULKDATA_CompressedLzo			0x10		// unknown name
#define BULKDATA_Unused					0x20		// empty bulk block
#define BULKDATA_SeparateData			0x40		// unknown name - bulk stored in a different place in the same file
#define BULKDATA_CompressedLzx			0x80		// unknown name

#if BLADENSOUL
#define BULKDATA_CompressedLzoEncr		0x100		// encrypted LZO
#endif

// UE4

#if UNREAL4

#define BULKDATA_PayloadAtEndOfFile		0x01		//?? bulk data stored at the end of this file
//#define BULKDATA_CompressedZlib		0x02
//#define BULKDATA_Unused				0x20
#define BULKDATA_ForceInlinePayload		0x40		//?? bulk data stored immediately after header

#endif // UNREAL4

struct FByteBulkData //?? separate FUntypedBulkData
{
	int		BulkDataFlags;				// BULKDATA_...
	int		ElementCount;				// number of array elements
	int64	BulkDataOffsetInFile;		// position in file, points to BulkData; 32-bit in UE3, 64-bit in UE4
	int		BulkDataSizeOnDisk;			// size of bulk data on disk
//	int		SavedBulkDataFlags;
//	int		SavedElementCount;
//	int		SavedBulkDataOffsetInFile;
//	int		SavedBulkDataSizeOnDisk;
	byte	*BulkData;					// pointer to array data
//	int		LockStatus;
//	FArchive *AttachedAr;

	FByteBulkData()
	:	BulkData(NULL)
	,	BulkDataOffsetInFile(0)
	{}

	virtual ~FByteBulkData()
	{
		if (BulkData) appFree(BulkData);
	}

	virtual int GetElementSize() const
	{
		return 1;
	}

	// support functions
	void SerializeHeader(FArchive &Ar);
	void SerializeChunk(FArchive &Ar);
	// main functions
	void Serialize(FArchive &Ar);
	void Skip(FArchive &Ar);
};

struct FWordBulkData : public FByteBulkData
{
	virtual int GetElementSize() const
	{
		return 2;
	}
};

struct FIntBulkData : public FByteBulkData
{
	virtual int GetElementSize() const
	{
		return 4;
	}
};

#endif // UNREAL3


// UE3 compression flags; may be used for other engines, so keep it outside of #if UNREAL3 block
#define COMPRESS_ZLIB		1
#define COMPRESS_LZO		2
#define COMPRESS_LZX		4

#if BLADENSOUL
#define COMPRESS_LZO_ENC	8						// encrypted LZO
#endif

#define COMPRESS_FIND		0xFF					// use this flag for appDecompress when exact compression method is not known


int appDecompress(byte *CompressedBuffer, int CompressedSize, byte *UncompressedBuffer, int UncompressedSize, int Flags);


/*-----------------------------------------------------------------------------
	UE4 support
-----------------------------------------------------------------------------*/

#if UNREAL4

// Unreal engine 4 versions, declared as enum to be able to see all revisions in single place
enum
{
	// Pre-release UE4 file versions
	VER_UE4_ASSET_REGISTRY_TAGS = 112,
	VER_UE4_TEXTURE_DERIVED_DATA2 = 124,
	VER_UE4_ADD_COOKED_TO_TEXTURE2D = 125,
	VER_UE4_REMOVED_STRIP_DATA = 130,
	VER_UE4_TEXTURE_SOURCE_ART_REFACTOR = 143,
	VER_UE4_REMOVE_ARCHETYPE_INDEX_FROM_LINKER_TABLES = 163,
	VER_UE4_REMOVE_NET_INDEX = 196,
	VER_UE4_BULKDATA_AT_LARGE_OFFSETS = 198,
	VER_UE4_SUMMARY_HAS_BULKDATA_OFFSET = 212,
	VAR_UE4_ARRAY_PROPERTY_INNER_TAGS = 282,
	VER_UE4_ENGINE_VERSION_OBJECT = 336,
	// UE4.0 source code released on GitHub. Note: if we don't have any VER_UE4_...
	// values between, for instance, VER_UE4_0 and VER_UE4_1, it doesn't matter for
	// this framework which version is serialized - 4.0 or 4.1, because 4.1 has nothing
	// new regarding supported object formats compared to 4.0.
	VER_UE4_0 = 342,
	VER_UE4_1 = 352,
	VER_UE4_2 = 363,
		VER_UE4_LOAD_FOR_EDITOR_GAME = 365,
	VER_UE4_3 = 382,
		VER_UE4_ADD_STRING_ASSET_REFERENCES_MAP = 384,
	VER_UE4_4 = 385,
	VER_UE4_5 = 401,
};

class FStripDataFlags
{
public:
	FStripDataFlags(FArchive& Ar, int MinVersion = VER_UE4_REMOVED_STRIP_DATA)
	{
		if (Ar.ArVer >= MinVersion)
		{
			Ar << GlobalStripFlags << ClassStripFlags;
		}
		else
		{
			GlobalStripFlags = ClassStripFlags = 0;
		}
	}

	FORCEINLINE bool IsEditorDataStripped() const
	{
		return (GlobalStripFlags & 1) != 0;
	}

	FORCEINLINE bool IsServerDataStripped() const
	{
		return (GlobalStripFlags & 2) != 0;
	}

	FORCEINLINE bool IsClassDataStripped(byte Flag) const
	{
		return (ClassStripFlags & Flag) != 0;
	}

protected:
	byte	GlobalStripFlags;
	byte	ClassStripFlags;
};

#endif // UNREAL4

/*-----------------------------------------------------------------------------
	Global variables
-----------------------------------------------------------------------------*/

extern FArchive *GDummySave;
extern int       GForceGame;
extern byte      GForcePlatform;
extern byte      GForceCompMethod;


/*-----------------------------------------------------------------------------
	Miscellaneous game support
-----------------------------------------------------------------------------*/

#if TRIBES3
// macro to skip Tribes3 FHeader structure
// check==3 -- Tribes3
// check==4 -- Bioshock
#define TRIBES_HDR(Ar,Ver)							\
	int t3_hdrV = 0, t3_hdrSV = 0;					\
	if (Ar.Engine() == GAME_VENGEANCE && Ar.ArLicenseeVer >= Ver) \
	{												\
		int check;									\
		Ar << check;								\
		if (check == 3)								\
			Ar << t3_hdrV << t3_hdrSV;				\
		else if (check == 4)						\
			Ar << t3_hdrSV;							\
		else										\
			appError("T3:check=%X (Pos=%X)", check, Ar.Tell()); \
	}
#endif // TRIBES3


#endif // __UNCORE_H__
