/*
 * Copyright (C) 2016-2019 Radek Malcic
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _Rajce_Rajce_h
#define _Rajce_Rajce_h

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
	typedef Rajce CLASSNAME;
	Rajce();
	virtual ~Rajce(){};

  private:
	Upp::String cfg_download_dir;
	Upp::String cfg_album_url;
	Upp::String cfg_album_user;
	bool cfg_download_new_only;
	bool cfg_download_video;
	bool cfg_append_user_name;
	bool cfg_enable_user_auth;
	bool cfg_use_https;
	bool cfg_use_https_proxy;
	Upp::String cfg_https_proxy_url;
	Upp::String cfg_https_proxy_port;
	int cfg_http_timeout_req;
	int cfg_http_timeout_con;

	Upp::String version;
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

	int VersionToInt(const Upp::String &version);

	Upp::String GetAppDirectory();
	Upp::String GetCfgFileName();
	Upp::String GetOS();
	Upp::String sha256sum(const Upp::String &filename);
};

#endif

// vim: ts=4
