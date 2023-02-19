#ifndef Rajce_Rajce_h
#define Rajce_Rajce_h

#include <CtrlLib/CtrlLib.h>

namespace Upp {
#define LAYOUTFILE <Rajce/Rajce.lay>
#include <CtrlCore/lay.h>
} // namespace Upp

#define TFILE <Rajce/Rajce.t>
#include <Core/t.h>

#define IMAGECLASS RajceImg
#define IMAGEFILE <Rajce/Rajce.iml>
#include <Draw/iml_header.h>

#include "Version.h"

#define ERR_NO_ERROR 0
#define ERR_NO_DATA -1
#define ERR_OPEN -2
#define ERR_CLOSE -3
#define ERR_READ -4
#define ERR_CREATE -5
#define ERR_EXIST -6
#define ERR_SAVE -7
#define ERR_SELECT -8
#define ERR_PARSE -9
#define ERR_DOWNLOAD -10

struct UserData {
	Upp::String url;
	Upp::String user;
	bool authorization;
	void Jsonize(Upp::JsonIO &jio) {
		jio("url", url)("user", user)("authorization", authorization);
	}
	void Xmlize(Upp::XmlIO &xio) { Upp::XmlizeByJsonize(xio, *this); }
};

struct QueueData {
	Upp::String download_url;
	Upp::String download_dir;
	Upp::String download_name;
	Upp::String album_server_dir;
};

class Rajce : public Upp::WithRajceLayout<Upp::TopWindow> {
  public:
	Rajce();
	~Rajce() override{};

  private:
	const int version_numbers = 4;
	const int default_http_timeout_req = 120000;
	const int min_http_timeout_req = 1000;
	const int default_http_timeout_con = 1000;
	const int min_http_timeout_con = 0;
	const int buffer_size = 1024;
	const int sha256_size = 64;

	Upp::String cfg_download_dir;
	Upp::String cfg_album_url;
	Upp::String cfg_album_user;
	bool cfg_download_new_only;
	bool cfg_download_video;
	bool cfg_append_user_name;
	bool cfg_append_album_name;
	bool cfg_enable_user_auth;
	bool cfg_use_https;
	bool cfg_use_https_proxy;
	Upp::String cfg_https_proxy_url;
	Upp::String cfg_https_proxy_port;
	int cfg_http_timeout_req;
	int cfg_http_timeout_con;

	Upp::String internal_name;
	bool init_done;

	Upp::Size start_sz;
	Upp::Size proxy_sz;

	Upp::Array<UserData> userdata;
	Upp::Array<QueueData> q;

	Upp::FrameRight<Upp::Button> del;

	Upp::HttpRequest http;
	Upp::FileOut http_file_out;		  // download directory with filename
	Upp::String http_file_out_string; // download directory with filename
	bool http_started;

	Upp::HttpRequest file_http;
	Upp::FileOut file_http_out;
	Upp::String file_http_out_string;
	Upp::int64 file_http_loaded;

	Upp::Size upgrade_sz;
	Upp::Size release_sz;

	Upp::WithUpgradeLayout<Upp::TopWindow> upgrade;
	Upp::String upgrade_url;
	Upp::String upgrade_url_sha256;
	Upp::String upgrade_version;
	Upp::String upgrade_release;
	Upp::int64 upgrade_size;

	Upp::HttpRequest upgrade_http;
	Upp::FileOut upgrade_http_out;
	Upp::String upgrade_http_out_string;
	Upp::int64 upgrade_http_loaded;

	int current_lang;

	void SelectDownloadDir();
	void Exit();

	int UserDataFind(const Upp::String &url);
	void UserDataLoad();
	void UserDataAdd();
	void UserDataDel(const Upp::String &url);
	void UserDataSet();
	void UserDataSelect(const Upp::String &url);

	void AlbumUrlAdd(const Upp::String &url);
	void AlbumUrlDel();

	void HttpStart();
	void HttpContent(const void *ptr, int size);
	bool HttpProxy(Upp::HttpRequest &request);
	void HttpAuthorization();
	bool HttpCheckUrl();
	bool HttpCheckParameters();
	void HttpPrependProtocol();
	void HttpAbort(bool ask);
	void HttpDownload();
	int HttpDownloadPage(const Upp::String &url, Upp::HttpRequest &request, Upp::FileOut &file,
						 Upp::String &file_name, bool authorize = true);

	Upp::String HttpGetParameterValue(const Upp::String &param, const Upp::String &txt);
	int HttpParse();

	void FileDownload();
	void FileStart();
	void FileContent(const void *ptr, int size);
	void FileProgress();

	Upp::Vector<int> VersionSplit(const Upp::String &s);
	int VersionCompare(const Upp::String &s1, const Upp::String &s2);

	void UpgradeCheck();
	void UpgradeSelectDirectory();
	void UpgradeDownloadVersion(const Upp::String &bite_size);
	void UpgradeDownload(const Upp::String &download_path, const Upp::String &download_file);
	void UpgradeStart();
	void UpgradeContent(const void *ptr, int size);
	void UpgradeProgress();
	void UpgradeAbort();
	void UpgradeToggleElements(bool enable);
	void UpgradeToggleRelease(bool enable);

	void InitText();
	void ToggleLang();
	void ToggleProxy();
	void ToggleAuthorization();
	void ToggleDownload();
	void ToggleProtocol();
	void ToggleTimeoutReq();
	void ToggleTimeoutCon();
	void ToggleUserDataSelect();

	void HttpProxyShow(bool show);
	void EnableElements(bool enable);

	void LoadCfg();
	void SaveCfg();

	Upp::String GetAppDirectory();
	Upp::String GetCfgFileName();
	Upp::String GetOS();
	Upp::String sha256sum(const Upp::String &filename);
};

#endif

// vim: ts=4
