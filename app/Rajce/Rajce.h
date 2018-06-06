/*
 * Copyright (C) 2016-2018 Radek Malcic
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

using namespace Upp;

#define LAYOUTFILE <Rajce/Rajce.lay>
#include <CtrlCore/lay.h>

#define TFILE <Rajce/Rajce.t>
#include <Core/t.h>

#define IMAGECLASS RajceImg
#define IMAGEFILE  <Rajce/Rajce.iml>
#include <Draw/iml.h>

#define ERR_NO_ERROR  0
#define ERR_NO_DATA  -1
#define ERR_OPEN     -2
#define ERR_CLOSE    -3
#define ERR_READ     -4
#define ERR_CREATE   -5
#define ERR_EXIST    -6
#define ERR_SAVE     -7
#define ERR_SELECT   -8
#define ERR_PARSE    -9
#define ERR_DOWNLOAD -10

struct QueueData {
	String download_url;
	String download_dir;
	String download_name;
	String album_server_dir;
};

class Rajce:public WithRajceLayout < TopWindow > {
 public:
	typedef Rajce CLASSNAME;
	Rajce();

 private:
	String m_cfg_name = "rad.ini";
	String m_cfg_download_dir;
	String m_cfg_album_url;
	String m_cfg_album_user;
	bool   m_cfg_download_new_only;
	bool   m_cfg_download_video;
	bool   m_cfg_append_user_name;
	bool   m_cfg_enable_user_auth;
	bool   m_cfg_use_https;
	bool   m_cfg_use_https_proxy;
	String m_cfg_https_proxy_url;
	String m_cfg_https_proxy_port;
	int    m_cfg_http_timeout_req;
	int    m_cfg_http_timeout_con;

	String m_version;
	String m_title_name;
	String m_download_text;

	Size start_sz;
	Size proxy_sz;

	Array<QueueData> q;

	HttpRequest file_http;
	FileOut		file_http_out;
	String		file_http_out_string;
	int64		file_http_loaded;

	HttpRequest m_http;
	FileOut		m_http_file_out;		// download directory with filename
	String		m_http_file_out_string;	// download directory with filename
	bool		m_http_started;
	int			m_current_lang;

	void SelectDownloadDir(void);
	void Exit(void);

	void HttpContent(const void *ptr, int size);
	void HttpStart(void);
	bool HttpProxy(void);
	void HttpAuthorization(void);
	bool HttpCheckAndGetUrl(String &url);
	void HttpPrependProtocol(String &url);
	void HttpAbort(bool ask);
	void HttpDownload(void);
	int  HttpDownloadPage(String url);

	String HttpGetParameterValue(const String param, const String &txt);
	int  HttpParse(void);

	void FileDownload(void);
	void FileContent(const void *ptr, int size);
	void FileProgress(void);
	void FileStart(void);

	void InitText(void);
	void ToggleProxy(void);
	void ToggleAlbum(void);
	void ToggleDownload(void);
	void ToggleLang(void);
	void ToggleProtocol(void);
	void ToggleTimeoutReq(void);
	void ToggleTimeoutCon(void);

	void HttpProxyShow(bool show);
	void EnableElements(bool enable);

	void LoadCfg(void);
	void SaveCfg(void);
	String GetCfgFileName(void);
};

#endif

// vim: ts=4
