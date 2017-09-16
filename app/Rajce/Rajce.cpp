/*
 * Copyright (C) 2016 Radek Malcic
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
	SetLanguage(GetSystemLNG());
	Icon(RajceImg::AppLogo());

	m_title_name = "Rajce album download";
	m_download_text = "Download progress";
	m_http_started = false;

	CtrlLayout(*this);
	this->WhenClose = THISBACK(Exit);

	lang.WhenPush = THISBACK(ToggleLang);

	download_dir_select.WhenPush = THISBACK(SelectDownloadDir);
	download_exit.WhenPush = THISBACK(Exit);
	download_ok.WhenPush = THISBACK(HttpDownload);
	download_abort.WhenPush = THISBACK1(HttpAbort, true);

	m_http.WhenContent = THISBACK(HttpContent);
	m_http.WhenStart = THISBACK(HttpStart);

	file_http.WhenContent = THISBACK(FileContent);
	file_http.WhenStart = THISBACK(FileStart);
	file_http.WhenWait = file_http.WhenDo = THISBACK(FileProgress);

	album_authorization <<= THISBACK(ToggleAlbum);
	album_authorization <<= false;
	album_pass.Password();

	proxy_enabled <<= THISBACK(ToggleProxy);
	proxy_enabled <<= false;
	http_proxy_pass.Password();

	download_dir <<= Nvl(GetDownloadFolder(), GetHomeDirFile("downloads"));
	download1_name.SetText("");
	download1_pi.Set(0,1);

	http_uri.WhenEnter = THISBACK(HttpUriChange);
	http_uri.SetText("https://www.rajce.net/");

	download_protocol <<= THISBACK(ToggleProtocol);
	download_protocol.Set(1);

	ToggleLang();
	ToggleProxy();
	ToggleAlbum();
	ToggleDownload();
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
	if (PromptOKCancel(Format("%s %s?", t_("Exit "), t_(m_title_name)))) {
		HttpAbort(false);
		Close();
	}
}

void Rajce::HttpContent(const void *ptr, int size)
{
	if (!m_http_file_out.IsOpen()) {
		RealizePath(m_http_file_out_string);
		m_http_file_out.Open(m_http_file_out_string);
	}
	m_http_file_out.Put(ptr, size);
}

void Rajce::HttpStart(void)
{
	if (m_http_file_out.IsOpen()) {
		m_http_file_out.Close();
		DeleteFile(m_http_file_out_string);
	}
}

bool Rajce::HttpProxy(void)
{
	bool result = true;

	if (proxy_enabled) {
		String proxy_url = ~http_proxy_url;
		int proxy_port = ~http_proxy_port;
		String proxy_user = ~http_proxy_user;
		String proxy_pass = ~http_proxy_pass;

		if ((!proxy_url.IsEmpty()) && (proxy_port > 0)) {
			m_http.CommonProxy(proxy_url, proxy_port);
			if (!proxy_user.IsEmpty()) {
				m_http.CommonProxyAuth(proxy_user, proxy_pass);
			}
		} else {
			Exclamation(t_("[= HTTP proxy settings is wrong!]"));
			result = false;
		}
	} else {
		m_http.CommonProxy("",0);
		m_http.CommonProxyAuth("","");
	}

	return (result);
}

bool Rajce::HttpCheckAndGetUrl(String &url)
{
	int url_len = http_uri.GetLength();

	if (url_len == 0) {
		Exclamation(t_("[= Album URL is not valid!]"));
		http_uri.SetFocus();
		return (false);
	}

	url = ~http_uri;

	// remove last '/' from url
	while ((url_len > 0) && (url[url_len - 1] == '/')) {
		url.Remove(url_len - 1);
	}

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

	return (true);
}

void Rajce::HttpAbort(bool ask)
{
	if (m_http_started) {
		bool abort = !ask;

		if (ask) {
			abort = PromptOKCancel(t_("Abort download?"));
		}

		if (abort) {
			m_http_started = false;

			if (m_http.Do()) {
				m_http.Abort();
			}

			if (file_http.Do()) {
				file_http.Abort();
			}

			while (!q.IsEmpty()) {
				QueueData queue_data = q.Pop();;
			}
		}
	}
}

void Rajce::HttpDownload(void)
{
	m_http_started = true;
	ToggleDownload();

	String download_url;
	bool do_download = HttpCheckAndGetUrl(download_url);

	// prepare download directory and file name
	if ((download_dir.GetLength() == 0) || (m_http_file_out_string.IsEmpty())) {
		m_http_file_out_string = Nvl(GetDownloadFolder(), GetHomeDirFile("downloads"));
	} else {
		m_http_file_out_string = ~download_dir;
	}
	m_http_file_out_string = AppendFileName(m_http_file_out_string, "rajce.html");

	if (do_download) {
		// prepare download statement
		m_http.New();

		if (~album_authorization) {
			m_http.Post("login", ~album_user);
			m_http.Post("password", ~album_pass);
		}

		// HTTP proxy setting
		do_download = HttpProxy();

		// begin download statement
		if (do_download) {
			m_http.Url(download_url).Execute();
		}

		if (m_http_file_out.IsOpen()) {
			m_http_file_out.Close();
		}
	}

	if (do_download) {
		if (!m_http.IsSuccess()) {
			Exclamation(t_("[= Download has failed.&\1") +
				    (m_http.IsError()? m_http.GetErrorDesc() : AsString(m_http.GetStatusCode()) + ' ' +
				     m_http.GetReasonPhrase()));
		} else {
			if (HttpParse() != ERR_NO_ERROR) {
				Exclamation(t_("[= Http parse error!&& Files can't be downloaded!]"));
			} else {
				FileDownload();
			}
		}
	}

	DeleteFile(m_http_file_out_string);
	m_http_started = false;
	ToggleDownload();
}

String Rajce::HttpGetParameterValue(String param, String &txt, bool is_quotes)
{
	String test_char;
	String parameter_value;
	int pos_param = txt.Find(param);
	int pos_first;
	int pos_last;

	if (is_quotes) {
		test_char = "\"";
	} else {
		test_char = " ";
	}

	if (pos_param > -1) {
		pos_first = txt.FindFirstOf(test_char, pos_param) + 1;
		pos_last = txt.Find(test_char, pos_first);

		parameter_value = txt.Mid(pos_first, (pos_last - pos_first));
	}

	return (parameter_value);
}

int Rajce::HttpParse(void)
{
	int64 l = GetFileLength(m_http_file_out_string);

	if (l < 0 || l > 16000000) {
		Exclamation(Format(t_("[= Error opening input file:&& %s!]"), DeQtf(m_http_file_out_string)));
		return (ERR_OPEN);
	}

	FileIn in(m_http_file_out_string);
	if (!in) {
		Exclamation(Format(t_("[= Error reading input file:&& %s!]"), DeQtf(m_http_file_out_string)));
		return (ERR_READ);
	}

	String txt;

	bool album_storage_found = false;
	String album_storage;
	String album_server_dir;
	String album_user_name;

	while (!in.IsEof()) {
		txt = in.GetLine();

		if (txt.Find("Album s přístupem na heslo") > 0) {
			if (~album_authorization) {
				String user = ~album_user;
				String pass = ~album_pass;

				if ((user.IsEmpty()) || (pass.IsEmpty())) {
					Exclamation(t_("[= Authorization is required!&& Fill the album authorization data.]"));
				} else {
					Exclamation(t_("[= Authorization is required!&& Wrong album authorization.]"));
				}
			} else {
				Exclamation(t_("[= Authorization is required!&& Enable album authorization.]"));
			}
			break;
		}

		if (txt.Find("storage") > 0) {
			album_storage = HttpGetParameterValue("storage", txt, true);
			album_storage_found = true;
			continue;
		}

		if (txt.Find("albumServerDir") > 0) {
			album_server_dir = HttpGetParameterValue("albumServerDir", txt, true);
			continue;
		}

		if (txt.Find("albumUserName") > 0) {
			album_user_name = HttpGetParameterValue("albumUserName", txt, true);
			continue;
		}

		if ((album_storage_found)  && (txt.Find("photos") > 0)  && (txt.Find("photoID") > 0)) {
			int pos = txt.FindFirstOf("[");
			Value photos = ParseJSON(txt.Mid(pos));
			
			for (int i = 0; i < photos.GetCount(); ++i) {

				String is_video = photos[i]["isVideo"].ToString();
				String file_name = photos[i]["fileName"].ToString();

				String dir_param = "images";
				if (is_video.Find("false") > 0) {
					dir_param = "video";
				}

				String full_path = AppendFileName(album_storage, dir_param);
				full_path = UnixPath(AppendFileName(full_path, file_name));

				int start = full_path.Find("://") + 2;
				while (full_path.Find("//", start) != -1) {
					int pos = full_path.Find("//", start);
					full_path.Remove(pos);
				}

				QueueData queue_data;
				queue_data.download_url = full_path;
				queue_data.download_dir = GetFileDirectory(m_http_file_out_string);

				if (~append_album_user_name)
					queue_data.download_dir = AppendFileName(queue_data.download_dir, album_user_name);
				queue_data.album_server_dir = album_server_dir;
				q.Push(queue_data);
			}
		}
	}

	DeleteFile(NativePath(m_http_file_out_string));
	return (ERR_NO_ERROR);
}

void Rajce::FileDownload(void)
{
	int processed_files = 0;
	int all_files = q.GetCount();

	while (m_http_started && !q.IsEmpty()) {
		download_label.SetText(Format("%s %d/%d", t_(m_download_text), processed_files + 1, all_files));

		QueueData queue_data = q.Pop();;
		String file_download = UnixPath(queue_data.download_url);
		String file_base_name = GetFileName(file_download);

		file_http_out_string = AppendFileName(queue_data.download_dir, queue_data.album_server_dir);
		RealizeDirectory(file_http_out_string);
		file_http_out_string = AppendFileName(file_http_out_string, file_base_name);

		file_http.New();

		if (~album_authorization) {
			file_http.Post("login", ~album_user);
			file_http.Post("password", ~album_pass);
		}

		// HTTP proxy setting
		bool do_download = HttpProxy();

		// begin download statement
		if (do_download) {
			download1_name.SetText(file_base_name);
			file_http.Url(file_download).Execute();
		}

		if (file_http_out.IsOpen()) {
			file_http_out.Close();
		}

		if (do_download) {
			if (!file_http.IsSuccess()) {
				DeleteFile(file_http_out_string);
				Ctrl::ProcessEvents();
				Exclamation(t_("[= Download has failed.&\1") +
					(file_http.IsError()? file_http.GetErrorDesc() : AsString(file_http.GetStatusCode()) + ' ' +
					 file_http.GetReasonPhrase()));
				break;
			} else {
				processed_files ++;
			}
		}
	}

	download_label.SetText(t_(m_download_text));
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
		download_label.SetText(t_(m_download_text));
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
	Title(t_(m_title_name));

	http_label.SetLabel(t_("Album settings"));
	http_uri_text.SetText(t_("Album URL:"));
	album_user_text.SetText(t_("Album user:"));
	album_pass_text.SetText(t_("Album password:"));
	download_text.SetText(t_("Download directory:"));
	append_album_user_name.SetLabel(t_("Append album user name to download directory"));
	album_authorization.SetLabel(t_("Enable album authorization"));
	download_protocol.SetLabel(t_("Use https protocol for autorization and download"));
	download_protocol.Tip(t_("HTTPS is used for authentication of the visited website and protection of the privacy and integrity of the exchanged data."));
	http_proxy_label.SetLabel(t_("HTTP proxy setting"));
	http_proxy_url_text.SetText(t_("Proxy URL:"));
	http_proxy_user_text.SetText(t_("Proxy user:"));
	http_proxy_pass_text.SetText(t_("Proxy password:"));
	proxy_enabled.SetLabel(t_("Enable HTTP proxy"));
	download_label.SetText(t_(m_download_text));
	download_ok.SetLabel(t_("Download"));
	download_abort.SetLabel(t_("Abort"));
	download_exit.SetLabel(t_("Exit"));
}

void Rajce::ToggleLang(void)
{
	Size lang_size = lang.GetSize();

	if (m_current_lang == LNG_('C','S','C','Z')) {
		m_current_lang = LNG_('E','N','U','S');
		lang.SetImage(Rescale(RajceImg::cz(), lang_size.cx, lang_size.cy));
	} else {
		m_current_lang = LNG_('C','S','C','Z');
		lang.SetImage(Rescale(RajceImg::gb(), lang_size.cx, lang_size.cy));
	}
	SetLanguage(m_current_lang);
	InitText();
}

void Rajce::ToggleProxy(void)
{
	if (~proxy_enabled) {
		http_proxy_url.Enable();
		http_proxy_port.Enable();
		http_proxy_user.Enable();
		http_proxy_pass.Enable();
	} else {
		http_proxy_url.Disable();
		http_proxy_port.Disable();
		http_proxy_user.Disable();
		http_proxy_pass.Disable();
	}
}

void Rajce::ToggleAlbum(void)
{
	if (~album_authorization) {
		album_user.Enable();
		album_pass.Enable();
	} else {
		album_user.Disable();
		album_pass.Disable();
	}
}

void Rajce::ToggleDownload(void)
{
	ToggleProtocol();
	if (m_http_started) {
		download_abort.Enable();
		download_abort.Show();
		download_ok.Disable();
		download_ok.Hide();
		download_protocol.Disable();
	} else {
		download_abort.Disable();
		download_abort.Hide();
		download_ok.Enable();
		download_ok.Show();
		download_protocol.Enable();
	}
}

void Rajce::ToggleProtocol(void)
{
	String url;
	bool do_download = HttpCheckAndGetUrl(url);
	http_uri.SetData(url);
}

void Rajce::HttpUriChange(void)
{
	ToggleProtocol();
}

// vim: ts=4
