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

#include "Rajce.h"

GUI_APP_MAIN {
	Rajce().Sizeable().Zoomable().Run();
}

Rajce::Rajce()
{
	init_done = false;

	SetLanguage(GetSystemLNG());
	Icon(RajceImg::AppLogo());

	version = "v1.5.0";
	internal_name = "rad";
	title_name = "Rajce album download";
	download_str = "Download progress";
	http_started = false;

	CtrlLayout(*this);
	this->WhenClose = THISBACK(Exit);

	lang.WhenPush = THISBACK(ToggleLang);

	download_dir_select.WhenPush = THISBACK(SelectDownloadDir);
	download1_name.SetText("");
	download1_pi.Set(0,1);

	download_exit.WhenPush = THISBACK(Exit);
	download_ok.WhenPush = THISBACK(HttpDownload);
	download_abort.WhenPush = THISBACK1(HttpAbort, true);

	http.WhenContent = THISBACK(HttpContent);
	http.WhenStart = THISBACK(HttpStart);

	file_http.WhenContent = THISBACK(FileContent);
	file_http.WhenStart = THISBACK(FileStart);
	file_http.WhenWait = file_http.WhenDo = THISBACK(FileProgress);

	album_authorization <<= THISBACK(ToggleAuthorization);
	album_pass.Password();

	timeout_req_text <<= THISBACK(ToggleTimeoutReq);
	timeout_con_text <<= THISBACK(ToggleTimeoutCon);

	proxy_enabled <<= THISBACK(ToggleProxy);
	http_proxy_pass.Password();

	download_protocol <<= THISBACK(ToggleProtocol);

	del.SetLabel("X");
	del.WhenPush = THISBACK(AlbumUrlDel);
	album_url.AddFrame(del);
	album_url.WhenAction = THISBACK(ToggleDownload);
	album_url.WhenEnter  = THISBACK(ToggleProtocol);
	album_url.WhenSelect = THISBACK(ToggleUserDataSelect);
	album_url.NullText("https://www.rajce.net");

	album_user.WhenAction = THISBACK(UserDataSet);

	start_sz = GetMinSize();
	proxy_sz = http_proxy_label.GetSize();

	LoadCfg();
	album_user.SetData(cfg_album_user);
	download_dir.SetData(cfg_download_dir);
	download_new_only.SetData(cfg_download_new_only);
	download_video.Set(cfg_download_video);
	append_album_user_name.SetData(cfg_append_user_name);
	album_authorization.SetData(cfg_enable_user_auth);
	download_protocol.SetData(cfg_use_https);
	proxy_enabled.SetData(cfg_use_https_proxy);
	http_proxy_url.SetData(cfg_https_proxy_url);
	http_proxy_port.SetData(cfg_https_proxy_port);
	timeout_req.SetData(cfg_http_timeout_req);
	timeout_con.SetData(cfg_http_timeout_con);

	UserDataLoad();
	EnableElements(true);

	ToggleLang();
	ToggleProxy();
	ToggleAuthorization();
	ToggleTimeoutReq();
	ToggleTimeoutCon();

	init_done = true;
}

void Rajce::SelectDownloadDir(void)
{
	SelectDirButton select_download_dir;
	String active_dir = ~download_dir;
	select_download_dir.ActiveDir(active_dir);
	select_download_dir.ExecuteSelectDir(active_dir);
	download_dir <<= select_download_dir.GetActiveDir();
}

void Rajce::Exit(void)
{
	if (PromptOKCancel(Format("%s %s?", t_("Exit"), t_(title_name)))) {
		HttpAbort(false);
		UserDataSet();
		SaveCfg();
		Close();
	}
}

int Rajce::UserDataFind(const String& url)
{
	int result = -1;
	for (int i = 0; i < userdata.GetCount(); ++i) {
		if (userdata[i].url == url) {
			result = i;
			break;
		}
	}
	return result;
}

void Rajce::UserDataLoad(void)
{
	for (int i = 0; i < userdata.GetCount(); ++i)
		album_url.AddList(userdata[i].url);
	UserDataSelect(cfg_album_url);
}

void Rajce::UserDataAdd(void)
{
	UserData data;
	data.url = album_url.GetData();
	data.user = album_user.GetData();
	data.authorization = album_authorization.GetData();

	if (UserDataFind(data.url) == -1)
		userdata.Add(data);
}

void Rajce::UserDataDel(const String& url)
{
	int i = UserDataFind(url);
	if (i > -1)
		userdata.Remove(i);
}

void Rajce::UserDataSet(void)
{
	if (!init_done)
		return;

	int i = UserDataFind(album_url.GetData());
	if (i > -1) {
		userdata[i].user = album_user.GetData();
		userdata[i].authorization = album_authorization.GetData();
	}
}

void Rajce::UserDataSelect(const String& url)
{
	int i = UserDataFind(url);
	if (i > -1) {
		album_url.SetData(userdata[i].url);
		album_user.SetData(userdata[i].user);
		album_authorization <<= userdata[i].authorization;
		ToggleAuthorization();
	}
}

void Rajce::AlbumUrlAdd(const String url)
{
	if (album_url.Find(url) < 0) {
		album_url.AddList(url);
		album_url.SetData(url);
		UserDataAdd();
	}
}

void Rajce::AlbumUrlDel(void)
{
	String url = album_url.GetData();

	if (url.GetCount() == 0)
		return;

	if (PromptYesNo(DeQtf(Format("%s \"%s\" ?", t_("Delete url:"), url))) != IDOK)
		return;

	int i = album_url.Find(url);
	if (i > -1) {
		album_url.Remove(i);
		UserDataDel(url);
		if (album_url.GetCount())
			UserDataSelect(album_url.Get(0));
	}
	album_url.Clear();
	ToggleDownload();
}

void Rajce::HttpContent(const void *ptr, int size)
{
	if (!http_file_out.IsOpen()) {
		RealizePath(http_file_out_string);
		http_file_out.Open(http_file_out_string);
	}
	http_file_out.Put(ptr, size);
}

void Rajce::HttpStart(void)
{
	if (http_file_out.IsOpen()) {
		http_file_out.Close();
		DeleteFile(http_file_out_string);
	}
}

bool Rajce::HttpProxy(HttpRequest& request)
{
	bool result = true;

	request.CommonProxy("",0);
	request.CommonProxyAuth("","");
	request.SSLProxy("",0);
	request.SSLProxyAuth("","");

	if (timeout_req.IsEnabled())
		request.RequestTimeout(timeout_req.GetData());
	if (timeout_con.IsEnabled())
		request.Timeout(timeout_con.GetData());

	if (proxy_enabled) {
		String proxy_url = ~http_proxy_url;
		int proxy_port = ~http_proxy_port;
		String proxy_user = ~http_proxy_user;
		String proxy_pass = ~http_proxy_pass;

		if ((!proxy_url.IsEmpty()) && (proxy_port > 0)) {
			if (download_protocol.Get() == 0) {
				request.CommonProxy(proxy_url, proxy_port);
				if (!proxy_user.IsEmpty()) {
					request.CommonProxyAuth(proxy_user, proxy_pass);
				}
			} else {
				request.SSLProxy(proxy_url, proxy_port);
				if (!proxy_user.IsEmpty()) {
					request.SSLProxyAuth(proxy_user, proxy_pass);
				}
			}
		} else {
			Exclamation(t_("[= HTTP proxy settings is wrong!]"));
			result = false;
		}
	}

	return (result);
}

bool Rajce::HttpCheckUrl(void)
{
	if (!init_done)
		return false;

	if (album_url.GetLength() == 0) {
		Exclamation(t_("[= Album URL is not valid!]"));
		album_url.SetFocus();
		return false;
	}

	HttpPrependProtocol();
	AlbumUrlAdd(album_url.GetData());
	return true;
}

void Rajce::HttpPrependProtocol(void)
{
	String url = album_url.GetData();
	// remove last '/' from url
	int url_len = url.GetLength();
	while ((url_len > 0) && (url[--url_len] == '/'))
		url.Remove(url_len);

	// prepend https:// or http:// protocol
	String protocol = "https://";
	if (download_protocol.Get() == 0)
		protocol = "http://";

	int https_pos = url.FindFirstOf("//");
	if (https_pos == -1) {
		url = protocol + url;
	} else {
		url = protocol + url.Mid(https_pos + 2);
	}
	album_url.SetData(url);
}

void Rajce::HttpAbort(bool ask)
{
	if (http_started) {
		bool abort = !ask;

		if (ask) {
			abort = PromptOKCancel(t_("Abort download?"));
		}

		if (abort) {
			http_started = false;

			if (http.Do())
				http.Abort();

			if (file_http.Do())
				file_http.Abort();

			if (q.GetCount() > 0)
				q.Clear();
		}
	}
}

void Rajce::HttpDownload(void)
{
	if (!DirectoryExists(download_dir.GetData().ToString())) {
		Exclamation(t_("[= Download directory doesn't exist!]"));
		return;
	}

	int64 min = timeout_req.GetMin();
	int64 cur = timeout_req.GetData();
	if (timeout_req.IsEnabled() && min > cur) {
		Exclamation(Format("%s %d ms", t_("Request timeout must be greater than"), min));
		return;
	}

	min = timeout_con.GetMin();
	cur = timeout_con.GetData();
	if (timeout_con.IsEnabled() && min > cur) {
		Exclamation(Format("%s %d ms", t_("Connection timeout must be greater than"), min));
		return;
	}

	http_started = true;
	ToggleProtocol();
	EnableElements(false);
	SaveCfg();

	String tmp_file;
	if (HttpCheckUrl() && (ERR_NO_ERROR == HttpDownloadPage(album_url.GetData()))) {
		tmp_file = http_file_out_string;
		if (HttpParse() != ERR_NO_ERROR)
			Exclamation(t_("[= Http parse error!&& Files can't be downloaded!]"));
		else
			FileDownload();
	}

	if (FileExists(tmp_file))
		DeleteFile(tmp_file);

	http_started = false;
	EnableElements(true);
	ToggleAuthorization();
}

int Rajce::HttpDownloadPage(String url)
{
	String download_url = url;

	// prepare download directory and file name
	if ((download_dir.GetLength() == 0) || (http_file_out_string.IsEmpty()))
		http_file_out_string = Nvl(GetDownloadFolder(), GetHomeDirFile("downloads"));
	else
		http_file_out_string = download_dir.GetData();

	http_file_out_string = GetTempFileName("rajce");

	// prepare download statement
	http.New();

	if (~album_authorization) {
		http.Post("login", ~album_user);
		http.Post("password", ~album_pass);
	}

	// begin download statement
	if (HttpProxy(http))
		http.Url(download_url).Execute();

	if (http_file_out.IsOpen())
		http_file_out.Close();

	int result = ERR_NO_ERROR;

	if (!http.IsSuccess()) {
		Exclamation(t_("[= Download has failed.&\1") +
			    (http.IsError()? http.GetErrorDesc() : AsString(http.GetStatusCode()) + ' ' +
			     http.GetReasonPhrase()));
		result = ERR_DOWNLOAD;
	}

	return (result);
}

String Rajce::HttpGetParameterValue(const String param, const String &txt)
{
	String test_char = "\"";
	String parameter_value;
	int pos_param = txt.Find(param);

	if (pos_param > -1) {
		int pos_first = txt.FindFirstOf(test_char, pos_param) + 1;
		int pos_last = txt.Find(test_char, pos_first);

		parameter_value = txt.Mid(pos_first, (pos_last - pos_first));
	}

	return (parameter_value);
}

int Rajce::HttpParse(void)
{
	int64 l = GetFileLength(http_file_out_string);

	if (l < 0 || l > 16000000) {
		Exclamation(Format(t_("[= Error opening input file:&& %s!]"), DeQtf(http_file_out_string)));
		return (ERR_OPEN);
	}

	FileIn in(http_file_out_string);
	if (!in) {
		Exclamation(Format(t_("[= Error reading input file:&& %s!]"), DeQtf(http_file_out_string)));
		return (ERR_READ);
	}

	String txt;

	bool album_storage_found = false;
	String album_storage;
	String album_server_dir;
	String album_user_name;

	q.Clear();

	while (!in.IsEof()) {
		txt = in.GetLine();

		if (txt.Find("Album s přístupem na kód") > 0) {
			if (txt.Find("Příliš mnoho neúspěšných pokusů"))
				Exclamation(t_("[= Too many unsuccessful attempts - try this in a moment!]"));

			if (~album_authorization) {
				String user = ~album_user;
				String pass = ~album_pass;

				if ((user.IsEmpty()) || (pass.IsEmpty()))
					Exclamation(t_("[= Authorization is required!&& Fill the album authorization data.]"));
				else
					Exclamation(t_("[= Authorization is required!&& Wrong album authorization.]"));
			} else {
				Exclamation(t_("[= Authorization is required!&& Enable album authorization.]"));
			}
			break;
		}

		if (txt.Find("storage") > 0) {
			album_storage = HttpGetParameterValue("storage", txt);
			album_storage_found = true;
			continue;
		}

		if (txt.Find("albumServerDir") > 0) {
			album_server_dir = HttpGetParameterValue("albumServerDir", txt);
#ifdef PLATFORM_WIN32
			// remove all '.' from the end of the album name
			int dir_len = album_server_dir.GetCount();
			while ((dir_len > 0) && (album_server_dir[--dir_len] == '.'))
				album_server_dir.Remove(dir_len);
#endif
			continue;
		}

		if (txt.Find("albumUserName") > 0) {
			album_user_name = HttpGetParameterValue("albumUserName", txt);
			continue;
		}

		if ((album_storage_found)  && (txt.Find("photos") > 0) && (txt.Find("photoID") > 0)) {
			int pos = txt.FindFirstOf("[");
			Value photos = ParseJSON(txt.Mid(pos));

			for (int i = 0; i < photos.GetCount(); ++i) {

				String is_video = photos[i]["isVideo"].ToString();
				String file_name = photos[i]["fileName"].ToString();

				String full_path;

				if (is_video.Compare("true") == 0) {
					if (download_video.GetData()) {
						int len = photos[i]["info"].ToString().Find(" |");
						file_name = photos[i]["info"].ToString().Mid(0, len);
						full_path = UnixPath(photos[i]["videoStructure"]["items"][0]["video"][0]["file"].ToString());
					}
				} else {
					full_path = AppendFileName(album_storage, "images");
					full_path = UnixPath(AppendFileName(full_path, file_name));
				}

				if (full_path.GetCount() > 0) {
					int start = full_path.Find("://") + 2;
					while (full_path.Find("//", start) != -1) {
						int pos = full_path.Find("//", start);
						full_path.Remove(pos);
					}

					QueueData queue_data;
					queue_data.download_url = full_path;
					queue_data.download_dir = download_dir.GetData();
					queue_data.download_name = file_name;

					if (~append_album_user_name)
						queue_data.download_dir = AppendFileName(queue_data.download_dir, album_user_name);
					queue_data.album_server_dir = album_server_dir;

					// Check if the file is already downloaded
					String test = AppendFileName(queue_data.download_dir, queue_data.album_server_dir);
					test = AppendFileName(test, queue_data.download_name);
					if (download_new_only.GetData() != 0 && FileExists(test)) {
						continue;
					} else {
						q.Add(queue_data);
					}
				}
			}
		}
	}

	return (ERR_NO_ERROR);
}

void Rajce::FileDownload(void)
{
	int processed_files = 0;
	int all_files = q.GetCount();

	if (http_started) {
		for (int i = 0; i < q.GetCount(); ++i) {
			download_label.SetText(Format("%s %d/%d", t_(download_str), processed_files + 1, all_files));

			QueueData queue_data = q[i];
			String file_download = UnixPath(queue_data.download_url);

			file_http_out_string = AppendFileName(queue_data.download_dir, queue_data.album_server_dir);
			file_http_out_string = AppendFileName(file_http_out_string, queue_data.download_name);

			// Download the file
			file_http.New();

			if (~album_authorization) {
				file_http.Post("login", ~album_user);
				file_http.Post("password", ~album_pass);
			}

			// HTTP proxy setting
			bool do_download = HttpProxy(file_http);

			// begin download statement
			if (do_download) {
				download1_name.SetText(queue_data.download_name);
				file_http.Url(file_download).Execute();
			}

			if (file_http_out.IsOpen())
				file_http_out.Close();

			if (do_download) {
				if (!file_http.IsSuccess()) {
					DeleteFile(file_http_out_string);
					Ctrl::ProcessEvents();
					Exclamation(t_("[= Download has failed.&\1") +
						(file_http.IsError()? file_http.GetErrorDesc() : AsString(file_http.GetStatusCode()) + ' ' +
						 file_http.GetReasonPhrase()));
					break;
				} else {
					processed_files++;
				}
			}
		}
	}

	download_label.SetText(t_(download_str));
	download1_name.SetText("");
	download1_pi.Set(0, 1);

	Exclamation(t_("[= Download complete!&& Files downloaded: ") + AsString(processed_files) + ']');
}

void Rajce::FileContent(const void *ptr, int size)
{
	file_http_loaded += size;
	if (!file_http_out.IsOpen()) {
		RealizePath(file_http_out_string);
		file_http_out.Open(file_http_out_string);
	}
	file_http_out.Put(ptr, size);
	Ctrl::ProcessEvents();
}

void Rajce::FileProgress(void)
{
	if (file_http.GetContentLength() >= 0) {
		download1_pi.Set((int)file_http_loaded, (int)file_http.GetContentLength());
	} else {
		download_label.SetText(t_(download_str));
		download1_name.SetText("");
		download1_pi.Set(0, 1);
	}
}

void Rajce::FileStart(void)
{
	if (file_http_out.IsOpen()) {
		file_http_out.Close();
		DeleteFile(file_http_out_string);
	}
	file_http_loaded = 0;
}

void Rajce::InitText(void)
{
	Title(Format("%s - %s", t_(title_name), version));

	album_label.SetLabel(t_("Album"));
	album_url_text.SetText(t_("Album URL:"));
	album_user_text.SetText(t_("Album user:"));
	album_pass_text.SetText(t_("Album password:"));
	download_text.SetText(t_("Download directory:"));
	download_new_only.SetLabel(t_("Download new files only"));
	download_video.SetLabel(t_("Download video files"));
	append_album_user_name.SetLabel(t_("Append album user name to download directory"));
	album_authorization.SetLabel(t_("Enable album authorization"));
	timeout_req_text.SetLabel(t_("Request timeout (ms)"));
	timeout_con_text.SetLabel(t_("Connection timeout (ms)"));
	download_protocol.SetLabel(t_("Use https protocol for autorization and download"));
	download_protocol.Tip(t_("HTTPS is used for authentication of the visited website and protection of the privacy and integrity of the exchanged data."));
	settings.SetLabel(t_("Settings"));
	http_proxy_label.SetLabel(t_("HTTP proxy setting"));
	http_proxy_url_text.SetText(t_("Proxy URL:"));
	http_proxy_user_text.SetText(t_("Proxy user:"));
	http_proxy_pass_text.SetText(t_("Proxy password:"));
	proxy_enabled.SetLabel(t_("Enable HTTP proxy"));
	download_label.SetText(t_(download_str));
	download_ok.SetLabel(t_("Download"));
	download_abort.SetLabel(t_("Abort"));
	download_exit.SetLabel(t_("Exit"));

	homepage.SetQTF(t_("[^https://github.com/CoolmanCZ/rajce^ [4 Rajce album download homepage]]"));
}

void Rajce::ToggleLang(void)
{
	Size lang_size = lang.GetSize();

	if (current_lang == LNG_('C','S','C','Z')) {
		current_lang = LNG_('E','N','U','S');
		lang.SetImage(Rescale(RajceImg::cz(), lang_size.cx, lang_size.cy));
	} else {
		current_lang = LNG_('C','S','C','Z');
		lang.SetImage(Rescale(RajceImg::gb(), lang_size.cx, lang_size.cy));
	}
	SetLanguage(current_lang);
	InitText();
}

void Rajce::ToggleProxy(void)
{
	Rect main_rc = GetRect();
	Size main_sz = main_rc.GetSize();

	if (~proxy_enabled) {
		if (main_sz.cy < start_sz.cy) {
			main_sz.cy += proxy_sz.cy;
			main_rc.InflateVert(proxy_sz.cy / 2);
			main_rc.SetSize(main_sz);

			SetMinSize(start_sz);
		}
		HttpProxyShow(true);

	} else {
		if (main_sz.cy >= start_sz.cy) {
			main_sz.cy -= proxy_sz.cy;
			main_rc.DeflateVert(proxy_sz.cy / 2);
			main_rc.SetSize(main_sz);

			Size sz = start_sz;
			sz.cy -= proxy_sz.cy;
			SetMinSize(sz);
		}
		HttpProxyShow(false);
	}

	SetRect(main_rc);
	UpdateLayout();
}

void Rajce::ToggleAuthorization(void)
{
	album_user.Enable(~album_authorization);
	album_pass.Enable(~album_authorization);
	UserDataSet();
}

void Rajce::ToggleDownload(void)
{
	bool url_not_null = album_url.GetData().ToString().GetCount() > 0;

	del.Enable(url_not_null);

	download_abort.Enable(http_started);
	download_abort.Show(http_started);
	download_ok.Enable(!http_started && url_not_null);
	download_ok.Show(!http_started);
}

void Rajce::ToggleProtocol(void)
{
	if (download_protocol.Get() == 0) {
		http.SSL(false);
	} else {
		http.SSL(true);
	}

	HttpCheckUrl();
	UserDataSet();
}

void Rajce::ToggleTimeoutReq(void)
{
	timeout_req.Enable(~timeout_req_text);
}

void Rajce::ToggleTimeoutCon()
{
	timeout_con.Enable(~timeout_con_text);
}

void Rajce::ToggleUserDataSelect()
{
	UserDataSelect(album_url.GetData());
}

void Rajce::HttpProxyShow(bool show)
{
	http_proxy_label.Show(show);
	http_proxy_url.Show(show);
	http_proxy_url_text.Show(show);
	http_proxy_port.Show(show);
	http_proxy_user.Show(show);
	http_proxy_user_text.Show(show);
	http_proxy_pass.Show(show);
	http_proxy_pass_text.Show(show);
	http_proxy_colon.Show(show);
}

void Rajce::EnableElements(bool enable)
{
	ToggleDownload();

	album_url.Enable(enable);
	download_dir.Enable(enable);
	download_video.Enable(enable);
	download_dir_select.Enable(enable);
	download_new_only.Enable(enable);

	timeout_req_text.Enable(enable);
	timeout_req.Enable(enable);
	timeout_con_text.Enable(enable);
	timeout_con.Enable(enable);

	album_user.Enable(enable);
	album_pass.Enable(enable);
	append_album_user_name.Enable(enable);
	album_authorization.Enable(enable);
	download_protocol.Enable(enable);
	proxy_enabled.Enable(enable);

	http_proxy_url.Enable(enable);
	http_proxy_port.Enable(enable);
	http_proxy_user.Enable(enable);
	http_proxy_pass.Enable(enable);
}

void Rajce::LoadCfg(void)
{
	String cfg_file = GetCfgFileName();
	VectorMap<String, String> data = LoadIniFile(cfg_file);

	String dir = Nvl(GetDownloadFolder(), GetHomeDirFile("downloads"));

	cfg_download_dir = data.Get("DOWNLOAD_DIR", dir);
	cfg_download_new_only = data.Get("DOWNLOAD_NEW_ONLY", Null) == "true" ? true : false;
	cfg_download_video = data.Get("DOWNLOAD_VIDEO", Null) == "true" ? true : false;
	cfg_album_url = data.Get("ALBUM_URL", Null);
	cfg_album_user = data.Get("ALBUM_USER", Null);
	cfg_append_user_name = data.Get("APPEND_USER_NAME", Null) == "true" ? true : false;
	cfg_enable_user_auth = data.Get("ENABLE_USER_AUTH", Null) == "true" ? true : false;
	cfg_use_https = data.Get("USE_HTTPS", Null) == "true" ? true : false;
	cfg_use_https_proxy = data.Get("USE_HTTP_PROXY", Null) == "true" ? true : false;
	cfg_https_proxy_url = data.Get("HTTP_PROXY_URL", Null);
	cfg_https_proxy_port =  data.Get("HTTP_PROXY_PORT", Null);
	int tmp = ScanInt64(data.Get("HTTP_TIMEOUT_REQUEST", Null));
	cfg_http_timeout_req = tmp < 999 ? 1000 : tmp;
	tmp = ScanInt64(data.Get("HTTP_TIMEOUT_CONNECTION", Null));
	cfg_http_timeout_con = tmp < 999 ? 1000 : tmp;

	LoadFromJson(userdata, data.Get("USER_DATA", Null));
}

void Rajce::SaveCfg(void)
{
	String cfg_file = GetCfgFileName();
	String data;

	data
		<< title_name << ": Configuration Text File" << "\n\n"
		<< "DOWNLOAD_DIR = " << download_dir.GetData() << "\n"
		<< "DOWNLOAD_NEW_ONLY = " << (download_new_only.GetData() ? "true" : "false") << "\n"
		<< "DOWNLOAD_VIDEO = " << (download_video.GetData() ? "true" : "false") << "\n"
		<< "ALBUM_URL = " << album_url.GetData() << "\n"
		<< "ALBUM_USER = " << album_user.GetData() << "\n"
		<< "APPEND_USER_NAME = " << (append_album_user_name.GetData() ? "true" : "false") << "\n"
		<< "ENABLE_USER_AUTH = " << (album_authorization.GetData() ? "true" : "false") << "\n"
		<< "USE_HTTPS = " << (download_protocol.GetData() ? "true" : "false") << "\n"
		<< "USE_HTTP_PROXY = " << (proxy_enabled.GetData() ? "true" : "false") << "\n"
		<< "HTTP_PROXY_URL = " << http_proxy_url.GetData() << "\n"
		<< "HTTP_PROXY_PORT = " << http_proxy_port.GetData() << "\n"
		<< "HTTP_TIMEOUT_REQUEST = " << timeout_req.GetData() << "\n"
		<< "HTTP_TIMEOUT_CONNECTION = " << timeout_con.GetData() << "\n"
		<< "USER_DATA = " << StoreAsJson(userdata) << "\n";
		;

	if(!FileExists(cfg_file))
		RealizePath(cfg_file);

	if(!SaveFile(cfg_file, data))
		Exclamation(t_("Configuration file saving has failed!"));
}

String Rajce::GetAppDirectory(void)
{
	String p;
#if defined(PLATFORM_WIN32)
	p = AppendFileName(GetEnv("LOCALAPPDATA"), internal_name);
	ONCELOCK
	RealizeDirectory(p);
#elif defined(PLATFORM_POSIX)
	p = AppendFileName(GetHomeDirectory(), "." + internal_name);
	ONCELOCK
	RealizeDirectory(p);
#else
	Exclamation("Configuration is not implemented for this platform");
#endif
	return (p);
}

String Rajce::GetCfgFileName(void)
{
	return (AppendFileName(GetAppDirectory(), internal_name + ".ini"));
}
// vim: ts=4
