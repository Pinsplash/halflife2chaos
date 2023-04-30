// Licensed under Apache 2.0, original from ntrf in blamod, modified by tmob for chaos mod

#include "cbase.h"
#include "icvar.h"
#include "filesystem.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "scenefilecache/ISceneFileCache.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file - literally 1984
#include "tier0/memdbgon.h"

extern ISceneFileCache* scenefilecache;
extern IMDLCache* mdlcache;

extern void S_SoundEmitterSystemFlush(void);
extern void ParseParticleEffects(bool bLoadSheets, bool bPrecache);

static bool mount_inited = false;

extern bool SoundEmitter_ForceManifestReload;

static CUtlConstString ep1path;
static CUtlConstString ep2path;
static CUtlConstString hl2path;
static CUtlConstString hlspath;
static CUtlConstString sdkpath;
static CUtlConstString moddir;
static CUtlConstString original_paths;

const char* const hlsMountPoints[] = {
	"%s/hl1/hl1_sound_vo_english.vpk",
	"%s/hl1/hl1_pak.vpk",
	NULL
};

const char* const ep2MountPoints[] = {
	"%s/ep2/ep2_sound_vo_english.vpk",
	"%s/ep2/ep2_pak.vpk",
	NULL
};
const char* const ep1MountPoints[] = {
	"%s/episodic/ep1_sound_vo_english.vpk",
	"%s/episodic/ep1_pak.vpk",
	NULL
};
const char* const hl2MountPoints[] = {
	"%s/hl2/hl2_sound_vo_english.vpk",
	"%s/hl2/hl2_pak.vpk",
	NULL
};

const char* const commonMountPoints[] = {
	"%s/hl2/hl2_textures.vpk",
	"%s/hl2/hl2_sound_misc.vpk",
	"%s/hl2/hl2_misc.vpk",
	NULL
};

const char* hlsFolderMount = "%s/hl1/";
const char* ep2FolderMount = "%s/ep2/";
const char* ep1FolderMount = "%s/episodic/";
const char* hl2FolderMount = "%s/hl2/";

const char* const commonFolderMounts[] = {
	"%s/ep2/",
	"%s/episodic/",
	"%s/hl2/",
	NULL
};

const char* skill_exec = "exec skill_manifest.cfg";

CUtlString ChaosmodMountName;
extern CUtlString ChaosmodCategoryName;
extern ConVar chaos_sandbox;

static void AppendMountPoint(CUtlVector < CUtlConstString >& list, const char* mount, const char* path)
{
	CUtlString path_out;
	path_out.Format(mount, path);
	V_FixSlashes(path_out.Get());
	list.AddToTail(CUtlConstString(path_out));
}

static void AppendMountPoints(CUtlVector < CUtlConstString >& list, const char* const* mounts, const char* path)
{
	const char* const* m = mounts;

	CUtlString path_out;

	for (; *m; ++m) {
		path_out.Format(*m, path);
		V_FixSlashes(path_out.Get());

		list.AddToTail(CUtlConstString(path_out));
	}
}

enum class MountMode
{
	SDK_ONLY,
	HL2,
	HL2EP1,
	HL2EP2,
	HLS
};

static const char* GetMountName(MountMode mount)
{
	switch (mount) {
	case MountMode::HL2: return "hl2";
	case MountMode::HL2EP1: return "ep1";
	case MountMode::HL2EP2: return "ep2";
	case MountMode::HLS: return "hls";
	default: return "";
	}
}

static MountMode currentMountMode = MountMode::SDK_ONLY;

static void chaos_mount_f(const CCommand& cmd)
{
	MountMode mount = MountMode::SDK_ONLY;
	if (cmd.ArgC() > 1) {
		const char* mname = cmd.Arg(1);

		if (stricmp(mname, "hl2") == 0) {
			mount = MountMode::HL2;
		}
		else if (stricmp(mname, "ep1") == 0) {
			mount = MountMode::HL2EP1;
		}
		else if (stricmp(mname, "ep2") == 0) {
			mount = MountMode::HL2EP2;
		}
		else if (stricmp(mname, "hls") == 0) {
			mount = MountMode::HLS;
		}
		else if (stricmp(mname, "sdk") == 0) {
			mount = MountMode::SDK_ONLY;
		}
		else {
			Warning("Unknown mount mode : \"%s\"\n", mname);
			return;
		}
	}

	auto sapps = steamapicontext->SteamApps();

	if (!sapps) {
		Error("You need to have steam running!\n");
		return;
	}

	if (currentMountMode == mount)
		return;

	//const char * language = sapps->GetCurrentGameLanguage();

	char name[2049];
	int namelen = 0;

	if (!mount_inited) {

		namelen = sapps->GetAppInstallDir(380, name, sizeof(name));
		if (namelen > 0) ep1path = name;

		namelen = sapps->GetAppInstallDir(420, name, sizeof(name));
		if (namelen > 0) ep2path = name;

		namelen = sapps->GetAppInstallDir(220, name, sizeof(name));
		if (namelen > 0) hl2path = name;

		namelen = sapps->GetAppInstallDir(280, name, sizeof(name));
		if (namelen > 0) hlspath = name;

		namelen = sapps->GetAppInstallDir(243730, name, sizeof(name));
		if (namelen > 0) sdkpath = name;

		filesystem->GetSearchPath("MOD", true, name, sizeof(name));
		if (namelen > 0) moddir = name;

		// Get list of all current paths
		int res = filesystem->GetSearchPath("GAME", true, name, sizeof(name));
		if (res > 0) original_paths = name;

		mount_inited = true;
	}

	CUtlVector < CUtlConstString > new_paths;

	size_t i;

	AppendMountPoint(new_paths, "%s", moddir.Get());

	if (mount == MountMode::SDK_ONLY) {
		// Only sdk paths
		AppendMountPoints(new_paths, hl2MountPoints, sdkpath.Get());
		AppendMountPoints(new_paths, commonMountPoints, sdkpath.Get());

		AppendMountPoint(new_paths, hl2FolderMount, sdkpath.Get());

	}
	else if (mount == MountMode::HL2EP2) {
		// check if episode 2 is installed
		if (ep2path.IsEmpty()) {
			Warning("Unable to locate instalation of HL2 Episode 2. Aborting\n");
			return;
		}

		// EP2
		AppendMountPoints(new_paths, ep2MountPoints, ep2path.Get());
		AppendMountPoints(new_paths, ep1MountPoints, sdkpath.Get());
		AppendMountPoints(new_paths, hl2MountPoints, sdkpath.Get());
		AppendMountPoints(new_paths, commonMountPoints, sdkpath.Get());

		AppendMountPoint(new_paths, ep2FolderMount, ep2path.Get());

	}
	else if (mount == MountMode::HL2EP1) {
		// check if episode 1 is installed
		if (ep1path.IsEmpty()) {
			Warning("Unable to locate instalation of HL2 Episode 1. Aborting\n");
			return;
		}

		// EP1
		AppendMountPoints(new_paths, ep1MountPoints, ep1path.Get());
		AppendMountPoints(new_paths, hl2MountPoints, sdkpath.Get());
		AppendMountPoints(new_paths, commonMountPoints, sdkpath.Get());

		AppendMountPoint(new_paths, ep1FolderMount, ep1path.Get());
	}
	else if (mount == MountMode::HL2) {
		// check if hl2 is installed
		if (hl2path.IsEmpty()) {
			Warning("Unable to locate instalation of HL2. Aborting\n");
			return;
		}

		// HL2
		AppendMountPoints(new_paths, hl2MountPoints, hl2path.Get());
		AppendMountPoints(new_paths, commonMountPoints, sdkpath.Get());

		AppendMountPoint(new_paths, hl2FolderMount, hl2path.Get());
	}
	else if (mount == MountMode::HLS) {
		// check if episode 2 is installed
		if (hlspath.IsEmpty()) {
			Warning("Unable to locate instalation of HL:Source. Aborting\n");
			return;
		}

		// HL:Source
		AppendMountPoints(new_paths, hlsMountPoints, hlspath.Get());
		AppendMountPoints(new_paths, hl2MountPoints, sdkpath.Get());
		AppendMountPoints(new_paths, commonMountPoints, sdkpath.Get());

		AppendMountPoint(new_paths, hlsFolderMount, hlspath.Get());
	}
	else {
		Warning("Unsupported mount mode\n");
		Assert(false);
		return;
	}

	AppendMountPoints(new_paths, commonFolderMounts, sdkpath.Get());

	filesystem->RemoveSearchPaths("GAME");

	for (i = 0; i < (unsigned)new_paths.Count(); ++i) {
		DevMsg("Mounts [%d] => %s\n", i, new_paths[i]);

		filesystem->AddSearchPath(new_paths[i].Get(), "GAME");
	}

#if _DEBUG
	// show result
	int res = filesystem->GetSearchPath("GAME", true, name, sizeof(name));
#endif

	currentMountMode = mount;
	ChaosmodMountName = GetMountName(mount);

	SoundEmitter_ForceManifestReload = true;

	S_SoundEmitterSystemFlush();

	datacache->Flush();
	mdlcache->Flush();

	scenefilecache->Reload();

	engine->ClientCmd_Unrestricted("snd_restart");

	ParseParticleEffects(true, true);

	// ntrf: i hope this will force CCs to be reloaded
	auto cc_lang = cvar->FindVar("cc_lang");
	if (cc_lang) {
		CUtlConstString cc_language = CUtlConstString(cc_lang->GetString());

		cc_lang->SetValue(cc_language.Get());
	}
}
static ConCommand chaos_mount("chaos_mount", chaos_mount_f, "Mount mod data\n hl2,\nep1,\nep2,\nhl1,\nsdk", FCVAR_CLIENTDLL);
