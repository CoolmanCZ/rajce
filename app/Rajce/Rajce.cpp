#include "Rajce.h"

using namespace Upp;

#define IMAGECLASS RajceImg
#define IMAGEFILE <Rajce/Rajce.iml>
#include <Draw/iml_source.h>

static bool ReadHex(StringStream &in, dword &hex, int n) {
	hex = 0;
	while (n-- != 0) {
		if (in.IsEof()) {
			return false;
		}
		int c = in.Get();
		if (!IsXDigit(c)) {
			return false;
		}
		hex = (hex << 4) + ctoi(c);
	}
	return true;
}

static String GetUtfSmall(StringStream &in) {
	String result;
	dword hex = 0;
	if (ReadHex(in, hex, 4)) {
		if (hex >= 0xD800 && hex < 0xDBFF) { // NOLINT (readability-magic-numbers)
			int c = in.Get();
			int next = in.Get();
			if (c == '\\' && next == 'u') {
				dword hex2 = 0;
				if (ReadHex(in, hex2, 4) && hex2 >= 0xDC00 && hex2 <= 0xDFFF) {			  // NOLINT (readability-magic-numbers)
					result.Cat(ToUtf8(((hex & 0x3ff) << 10) | (hex2 & 0x3ff) + 0x10000)); // NOLINT (readability-magic-numbers)
				}
			}
		} else {
			if (hex > 0 && hex < 0xDC00) { // NOLINT (readability-magic-numbers)
				result.Cat(ToUtf8(hex));
			}
		}
	}
	return result;
}

static String GetUtfCapital(StringStream &in) {
	String result;
	dword hex = 0;
	if (ReadHex(in, hex, 8) && hex > 0 && hex < 0x10ffff) { // NOLINT (readability-magic-numbers)
		result.Cat(ToUtf8(hex));
	}
	return result;
}

static String DecodeEscapedUtf(const String &s) {
	StringStream ss(s);
	String result;

	while (!ss.IsEof()) {
		int c = ss.Get();
		if (c == '\\') {
			int next = ss.Get();
			int64 pos = ss.GetPos();
			String utf;
			switch (next) {
			case 'u':
				utf = GetUtfSmall(ss);
				break;
			case 'U':
				utf = GetUtfCapital(ss);
				break;
			default:
				break;
			}
			if (utf.GetCount() > 0) {
				result.Cat(utf);
			} else {
				ss.Seek(pos);
				result.Cat(c);
				result.Cat(next);
			}
		} else {
			result.Cat(c);
		}
	}
	return result;
}

GUI_APP_MAIN { Rajce().Sizeable().Zoomable().Run(); }

Rajce::Rajce() {
	SetLanguage(GetSystemLNG());
	Icon(RajceImg::AppLogo());

	CtrlLayout(*this);
	this->WhenClose = [=] { Exit(); };

	lang.WhenAction = [=] { ToggleLang(); };

	download_dir_select.WhenAction = [=] { SelectDownloadDir(); };
	download1_name.SetText("");
	download1_pi.Set(0, 1);

	download_exit.WhenAction = [=] { Exit(); };
	download_exit.Exit();
	download_ok.WhenAction = [=] { HttpDownload(); };
	download_ok.Ok();
	download_abort.WhenAction = [=] { HttpAbort(true); };
	download_abort.Cancel();

	check_latest.WhenAction = [=] { UpgradeCheck(); };

	http.WhenContent = [=](auto ptr, auto size) { HttpContent(ptr, size); };
	http.WhenStart = [=] { HttpStart(); };

	file_http.WhenContent = [=](auto ptr, auto size) { FileContent(ptr, size); };
	file_http.WhenStart = [=] { FileStart(); };
	file_http.WhenWait = file_http.WhenDo = [=] { FileProgress(); };

	upgrade_http.WhenContent = [=](auto ptr, auto size) { UpgradeContent(ptr, size); };
	upgrade_http.WhenStart = [=] { UpgradeStart(); };
	upgrade_http.WhenWait = upgrade_http.WhenDo = [=] { UpgradeProgress(); };

	album_authorization.WhenAction = [=] { ToggleAuthorization(); };
	album_pass.Password();

	timeout_req_text.WhenAction = [=] { ToggleTimeoutReq(); };
	timeout_con_text.WhenAction = [=] { ToggleTimeoutCon(); };

	proxy_enabled.WhenAction = [=] { ToggleProxy(); };
	http_proxy_pass.Password();

	download_protocol.WhenAction = [=] { ToggleProtocol(); };

	del.SetLabel("X");
	del.WhenAction = [=] { AlbumUrlDel(); };
	album_url.AddFrame(del);
	album_url.WhenAction = [=] { ToggleDownload(); };
	album_url.WhenEnter = [=] { ToggleProtocol(); };
	album_url.WhenSelect = [=] { ToggleUserDataSelect(); };
	album_url.NullText("https://www.rajce.net");

	album_user.WhenAction = [=] { UserDataSet(); };

	start_sz = GetMinSize();
	proxy_sz = http_proxy_label.GetSize();

	LoadCfg();
	album_user.SetData(cfg_album_user);
	download_dir.SetData(cfg_download_dir);
	download_new_only.SetData(cfg_download_new_only);
	download_video.Set(static_cast<int>(cfg_download_video));
	append_user_name.SetData(cfg_append_user_name);
	append_album_name.SetData(cfg_append_album_name);
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

	Size chek_size = check_latest.GetSize();
	check_latest.SetImage(Rescale(RajceImg::upgrade_check(), chek_size.cx - 2, chek_size.cy - 2));

	init_done = true; // NOLINT
}

void Rajce::SelectDownloadDir() {
	SelectDirButton select_download_dir;
	String active_dir = ~download_dir;
	select_download_dir.ActiveDir(active_dir);
	select_download_dir.ExecuteSelectDir(active_dir);
	download_dir <<= select_download_dir.GetActiveDir();
}

void Rajce::Exit() {
	if (PromptOKCancel(Format("%s %s?", t_("Exit"), t_(APP_TITLE))) == 1) {
		HttpAbort(false);
		UserDataSet();
		SaveCfg();
		Close();
	}
}

int Rajce::UserDataFind(const String &url) {
	int result = -1;
	for (int i = 0; i < userdata.GetCount(); ++i) {
		if (userdata[i].url == url) {
			result = i;
			break;
		}
	}
	return result;
}

void Rajce::UserDataLoad() {
	for (int i = 0; i < userdata.GetCount(); ++i) {
		album_url.AddList(userdata[i].url);
	}
	UserDataSelect(cfg_album_url);
}

void Rajce::UserDataAdd() {
	UserData data;
	data.url = album_url.GetData();
	data.user = album_user.GetData();
	data.authorization = album_authorization.GetData();

	if (UserDataFind(data.url) == -1) {
		userdata.Add(data);
	}
}

void Rajce::UserDataDel(const String &url) {
	int i = UserDataFind(url);
	if (i > -1) {
		userdata.Remove(i);
	}
}

void Rajce::UserDataSet() {
	if (!init_done) {
		return;
	}

	int i = UserDataFind(album_url.GetData());
	if (i > -1) {
		userdata[i].user = album_user.GetData();
		userdata[i].authorization = album_authorization.GetData();
	}
}

void Rajce::UserDataSelect(const String &url) {
	int i = UserDataFind(url);
	if (i > -1) {
		album_url.SetData(userdata[i].url);
		album_user.SetData(userdata[i].user);
		album_authorization <<= userdata[i].authorization;
		ToggleAuthorization();
	}
}

void Rajce::AlbumUrlAdd(const String &url) {
	if (album_url.Find(url) < 0) {
		album_url.AddList(url);
		album_url.SetData(url);
		UserDataAdd();
	}
}

void Rajce::AlbumUrlDel() {
	String url = album_url.GetData();

	if (url.GetCount() == 0) {
		return;
	}

	if (PromptYesNo(DeQtf(Format("%s \"%s\" ?", t_("Delete url:"), url))) != IDOK) {
		return;
	}

	int i = album_url.Find(url);
	if (i > -1) {
		album_url.Remove(i);
		UserDataDel(url);
		if (album_url.GetCount() > 0) {
			UserDataSelect(album_url.Get(0));
		}
	}
	album_url.Clear();
	ToggleDownload();
}

void Rajce::HttpStart() {
	if (http_file_out.IsOpen()) {
		http_file_out.Close();
		DeleteFile(http_file_out_string);
	}
}

void Rajce::HttpContent(const void *ptr, int size) {
	if (!http_file_out.IsOpen()) {
		RealizePath(http_file_out_string);
		http_file_out.Open(http_file_out_string);
	}
	http_file_out.Put(ptr, size);
}

bool Rajce::HttpProxy(HttpRequest &request) {
	bool result = true;

	request.CommonProxy("", 0);
	request.CommonProxyAuth("", "");
	request.SSLProxy("", 0);
	request.SSLProxyAuth("", "");

	request.RequestTimeout(default_http_timeout_req);
	if (timeout_req_text.Get() == 1) {
		request.RequestTimeout(timeout_req.GetData());
	}
	request.Timeout(Null);
	if (timeout_con_text.Get() == 1) {
		request.Timeout(timeout_con.GetData());
	}

	if (~proxy_enabled) {
		String proxy_url = ~http_proxy_url;
		int proxy_port = ~http_proxy_port;
		String proxy_user = ~http_proxy_user;
		String proxy_pass = ~http_proxy_pass;

		if (proxy_url.GetCount() > 0 && proxy_port > 0) {
			if (download_protocol.Get() == 0) {
				request.CommonProxy(proxy_url, proxy_port);
				if (proxy_user.GetCount() > 0) {
					request.CommonProxyAuth(proxy_user, proxy_pass);
				}
			} else {
				request.SSLProxy(proxy_url, proxy_port);
				if (proxy_user.GetCount() > 0) {
					request.SSLProxyAuth(proxy_user, proxy_pass);
				}
			}
		} else {
			ErrorOK(t_("[= HTTP proxy settings is wrong!]"));
			result = false;
		}
	}

	return (result);
}

bool Rajce::HttpCheckUrl() {
	if (!init_done) {
		return false;
	}

	if (album_url.GetLength() == 0) {
		ErrorOK(t_("[= Album URL is not valid!]"));
		album_url.SetFocus();
		return false;
	}

	HttpPrependProtocol();
	AlbumUrlAdd(album_url.GetData());
	return true;
}

bool Rajce::HttpCheckParameters() {
	if (!DirectoryExists(download_dir.GetData().ToString())) {
		ErrorOK(t_("[= Download directory doesn't exist!]"));
		return false;
	}

	int64 min = timeout_req.GetMin();
	int64 cur = timeout_req.GetData();
	if (timeout_req_text.Get() > 0 && min > cur) {
		ErrorOK(Format("%s %d ms", t_("Request timeout must be greater than"), min));
		return false;
	}

	min = timeout_con.GetMin();
	cur = timeout_con.GetData();
	if (timeout_con_text.Get() > 0 && min > cur) {
		ErrorOK(Format("%s %d ms", t_("Connection timeout must be greater than"), min));
		return false;
	}

	return true;
}

void Rajce::HttpPrependProtocol() {
	String url = album_url.GetData();
	// remove last '/' from url
	int url_len = url.GetLength();
	while ((url_len > 0) && (url[--url_len] == '/')) {
		url.Remove(url_len);
	}

	// prepend https:// or http:// protocol
	String protocol = "https://";
	if (download_protocol.Get() == 0) {
		protocol = "http://";
	}

	int https_pos = url.FindFirstOf("//");
	if (https_pos == -1) {
		url = protocol + url;
	} else {
		url = protocol + url.Mid(https_pos + 2);
	}
	album_url.SetData(url);
}

void Rajce::HttpAbort(bool ask) {
	if (http_started) {
		bool abort = !ask;

		if (ask) {
			abort = PromptOKCancel(t_("Abort download?")) == 1;
		}

		if (abort) {
			http_started = false;

			if (http.Do()) {
				http.Abort();
			}

			if (file_http.Do()) {
				file_http.Abort();
			}

			if (q.GetCount() > 0) {
				q.Clear();
			}

			EnableElements(true);
		}
	}
}

void Rajce::HttpDownload() {
	if (!HttpCheckParameters()) {
		return;
	}

	http_started = true;
	ToggleProtocol();
	EnableElements(false);
	SaveCfg();

	String tmp_file;
	if (HttpCheckUrl() && (ERR_NO_ERROR == HttpDownloadPage(album_url.GetData(), http, http_file_out, http_file_out_string))) {
		tmp_file = http_file_out_string;
		if (HttpParse() != ERR_NO_ERROR) {
			ErrorOK(t_("[= Http parse error!&& Files can't be downloaded!]"));
		} else {
			FileDownload();
		}
	}

	if (FileExists(tmp_file)) {
		DeleteFile(tmp_file);
	}

	http_started = false;
	EnableElements(true);
}

int Rajce::HttpDownloadPage(const String &url, HttpRequest &request, FileOut &file, String &file_name, bool authorize) {
	file_name = GetTempFileName("rajce");

	// prepare download statement
	request.New();

	if (authorize && ~album_authorization) {
		request.Post("login", ~album_user);
		request.Post("code", ~album_pass);
	}

	// HTTP proxy setting
	bool do_download = HttpProxy(request);

	// begin download statement
	if (do_download) {
		request.Url(url).Execute();
	}

	if (file.IsOpen()) {
		file.Close();
	}

	int result = ERR_NO_ERROR;

	if (!request.IsSuccess()) {
		ErrorOK(t_("[= Download has failed.&\1") +
				(request.IsError() ? request.GetErrorDesc() : AsString(request.GetStatusCode()) + ' ' + request.GetReasonPhrase()));
		result = ERR_DOWNLOAD;
	}

	return (result);
}

String Rajce::HttpGetParameterValue(const String &param, const String &txt) {
	String test_char = "\"";
	String parameter_value;
	int pos_param = txt.Find(param);

	if (pos_param > -1) {
		int pos_first = txt.FindFirstOf(test_char, pos_param) + 1;
		int pos_last = txt.Find(test_char, pos_first);

		parameter_value = txt.Mid(pos_first, (pos_last - pos_first));
	}

	return (DecodeEscapedUtf(parameter_value));
}

int Rajce::HttpParse() {
	int64 l = GetFileLength(http_file_out_string);

	if (l < 0) {
		ErrorOK(Format(t_("[= Error opening input file:&& %s!]"), DeQtf(http_file_out_string)));
		return (ERR_OPEN);
	}

	FileIn in(http_file_out_string);
	if (!in) {
		ErrorOK(Format(t_("[= Error reading input file:&& %s!]"), DeQtf(http_file_out_string)));
		return (ERR_READ);
	}

	String txt;

	bool album_storage_found = false;
	String album_storage;
	String album_server_dir;
	String album_name;

	q.Clear();

	while (!in.IsEof()) {
		txt = ToCharset(CHARSET_UTF8, in.GetLine(), CHARSET_UTF8);

		if (txt.Find("Album s přístupem na kód") > -1) {
			if (txt.Find("Příliš mnoho neúspěšných pokusů") > -1) {
				ErrorOK(t_("[= Too many unsuccessful attempts - try this in a moment!]"));
				break;
			}

			if (~album_authorization) {
				String user = ~album_user;
				String pass = ~album_pass;

				if ((user.IsEmpty()) || (pass.IsEmpty())) {
					ErrorOK(t_("[= Authorization is required!&& Fill the album authorization data.]"));
				} else {
					ErrorOK(t_("[= Authorization is required!&& Wrong album authorization.]"));
				}
			} else {
				ErrorOK(t_("[= Authorization is required!&& Enable album authorization.]"));
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

		if (txt.Find("albumName") > 0) {
			album_name = HttpGetParameterValue("albumName", txt);
			continue;
		}

		if ((album_storage_found) && (txt.Find("photos") > 0) && (txt.Find("photoID") > 0)) {
			int pos = txt.FindFirstOf("[");
			Value photos = ParseJSON(txt.Mid(pos));

			for (int i = 0; i < photos.GetCount(); ++i) {

				String is_video = photos[i]["isVideo"].ToString();
				String file_name = photos[i]["fileName"].ToString();

				String full_path;

				if (is_video.Compare("true") == 0) {
					if (download_video.GetData()) {
						String video_format = photos[i]["videoStructure"]["items"][1]["video"][0]["format"].ToString();
						file_name = file_name + '.' + video_format;
						full_path = UnixPath(photos[i]["videoStructure"]["items"][1]["video"][0]["file"].ToString());
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
					queue_data.album_server_dir = album_server_dir;

					if (~append_user_name && ~album_authorization) {
						queue_data.download_dir = AppendFileName(queue_data.download_dir, album_user.GetData().ToString());
					}
					if (~append_album_name) {
						queue_data.download_dir = AppendFileName(queue_data.download_dir, album_name);
					}

					// Check if the file is already downloaded
					String test = AppendFileName(queue_data.download_dir, queue_data.album_server_dir);
					test = AppendFileName(test, queue_data.download_name);
					if (download_new_only.GetData() != 0 && FileExists(test)) {
						continue;
					}

					q.Add(queue_data);
				}
			}
		}
	}

	return (ERR_NO_ERROR);
}

void Rajce::FileDownload() {
	int processed_files = 0;
	int all_files = q.GetCount();

	if (http_started) {
		for (int i = 0; i < q.GetCount(); ++i) {
			download_label.SetText(Format("%s %d/%d", t_("Download progress"), processed_files + 1, all_files));

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

			if (file_http_out.IsOpen()) {
				file_http_out.Close();
			}

			if (do_download) {
				if (!file_http.IsSuccess()) {
					DeleteFile(file_http_out_string);
					Ctrl::ProcessEvents();
					ErrorOK(t_("[= Download has failed.&\1") +
							(file_http.IsError() ? file_http.GetErrorDesc()
												 : AsString(file_http.GetStatusCode()) + ' ' + file_http.GetReasonPhrase()));
					break;
				}
				processed_files++;
			}
		}
	}

	download_label.SetText(t_("Download progress"));
	download1_name.SetText("");
	download1_pi.Set(0, 1);

	Exclamation(t_("[= Download complete!&& Files downloaded: ") + AsString(processed_files) + ']');
}

void Rajce::FileStart() {
	if (file_http_out.IsOpen()) {
		file_http_out.Close();
		DeleteFile(file_http_out_string);
	}
	file_http_loaded = 0;
}

void Rajce::FileContent(const void *ptr, int size) {
	file_http_loaded += size;
	if (!file_http_out.IsOpen()) {
		RealizePath(file_http_out_string);
		file_http_out.Open(file_http_out_string);
	}
	file_http_out.Put(ptr, size);
	Ctrl::ProcessEvents();
}

void Rajce::FileProgress() {
	if (file_http.GetContentLength() >= 0) {
		download1_pi.Set((int)file_http_loaded, (int)file_http.GetContentLength());
	} else {
		download_label.SetText(t_("Download progress"));
		download1_name.SetText("");
		download1_pi.Set(0, 1);
	}
}

void Rajce::InitText() {
	Title(Format("%s - v%s", t_(APP_TITLE), APP_VERSION_STR));

	album_label.SetLabel(t_("Album"));
	album_url_text.SetText(t_("Album URL:"));
	album_user_text.SetText(t_("Album user:"));
	album_pass_text.SetText(t_("Album password:"));
	download_text.SetText(t_("Download directory:"));
	download_new_only.SetLabel(t_("Download new files only"));
	download_video.SetLabel(t_("Download video files"));
	append_user_name.SetLabel(t_("Append user name to download directory"));
	append_album_name.SetLabel(t_("Append album name to download directory"));
	album_authorization.SetLabel(t_("Enable album authorization"));
	timeout_req_text.SetLabel(t_("Request timeout (ms)"));
	timeout_con_text.SetLabel(t_("Connection timeout (ms)"));
	download_protocol.SetLabel(t_("Use https protocol for autorization and download"));
	download_protocol.Tip(t_("HTTPS is used for authentication of the visited website and "
							 "protection of the privacy and integrity of the exchanged data."));
	settings.SetLabel(t_("Settings"));
	http_proxy_label.SetLabel(t_("HTTP proxy setting"));
	http_proxy_url_text.SetText(t_("Proxy URL:"));
	http_proxy_user_text.SetText(t_("Proxy user:"));
	http_proxy_pass_text.SetText(t_("Proxy password:"));
	proxy_enabled.SetLabel(t_("Enable HTTP proxy"));
	download_label.SetText(t_("Download progress"));
	download_ok.SetLabel(t_("Download"));
	download_abort.SetLabel(t_("Abort"));
	download_exit.SetLabel(t_("Exit"));

	homepage.SetQTF(t_("[^https://github.com/CoolmanCZ/rajce^ Rajce album downloader homepage]"), Zoom(64, 600)); // NOLINT
	homepage.SetZoom(Zoom(1, 1));

	lang.Tip(t_("Switch language"));
	check_latest.Tip(t_("Check available update"));
}

void Rajce::ToggleLang() {
	Size lang_size = lang.GetSize();

	if (current_lang == LNG_('C', 'S', 'C', 'Z')) {
		current_lang = LNG_('E', 'N', 'U', 'S');
		lang.SetImage(Rescale(RajceImg::cz(), lang_size.cx, lang_size.cy));
	} else {
		current_lang = LNG_('C', 'S', 'C', 'Z');
		lang.SetImage(Rescale(RajceImg::gb(), lang_size.cx, lang_size.cy));
	}
	SetLanguage(current_lang);
	InitText();
}

void Rajce::ToggleProxy() {
	Rect main_rc = GetRect();
	Size main_sz = main_rc.GetSize();

	if (~proxy_enabled) {
		if (main_sz.cy < start_sz.cy) {
			main_sz.cy += proxy_sz.cy;
			main_rc.InflateVert(proxy_sz.cy / 2);
			main_rc.SetSize(main_sz);
		}
		SetMinSize(start_sz);
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

void Rajce::ToggleAuthorization() {
	album_user.Enable(~album_authorization);
	album_pass.Enable(~album_authorization);
	append_user_name.Enable(~album_authorization);
	UserDataSet();
}

void Rajce::ToggleDownload() {
	bool url_not_null = album_url.GetData().ToString().GetCount() > 0;

	del.Enable(url_not_null);

	download_ok.Enable(!http_started && url_not_null);
	download_abort.Enable(http_started);
	download_abort.Show(http_started);
	download_exit.Enable(!http_started);
	download_exit.Show(!http_started);
}

void Rajce::ToggleProtocol() {
	if (download_protocol.Get() == 0) {
		http.SSL(false);
	} else {
		http.SSL(true);
	}

	HttpCheckUrl();
	UserDataSet();
}

void Rajce::ToggleTimeoutReq() { timeout_req.Enable(~timeout_req_text); }

void Rajce::ToggleTimeoutCon() { timeout_con.Enable(~timeout_con_text); }

void Rajce::ToggleUserDataSelect() { UserDataSelect(album_url.GetData()); }

void Rajce::HttpProxyShow(bool show) {
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

void Rajce::EnableElements(bool enable) {
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
	append_album_name.Enable(enable);
	album_authorization.Enable(enable);
	download_protocol.Enable(enable);
	proxy_enabled.Enable(enable);

	http_proxy_url.Enable(enable);
	http_proxy_port.Enable(enable);
	http_proxy_user.Enable(enable);
	http_proxy_pass.Enable(enable);

	check_latest.Enable(enable);

	if (enable) {
		ToggleTimeoutReq();
		ToggleTimeoutCon();
		ToggleAuthorization();
	}
}

void Rajce::LoadCfg() {
	String cfg_file = GetCfgFileName();
	VectorMap<String, String> data = LoadIniFile(cfg_file);

	String dir = Nvl(GetDownloadFolder(), GetHomeDirFile("downloads"));

	cfg_download_dir = data.Get("DOWNLOAD_DIR", dir);
	cfg_download_new_only = data.Get("DOWNLOAD_NEW_ONLY", Null) == "true";
	cfg_download_video = data.Get("DOWNLOAD_VIDEO", Null) == "true";
	cfg_album_url = data.Get("ALBUM_URL", Null);
	cfg_album_user = data.Get("ALBUM_USER", Null);
	cfg_append_user_name = data.Get("APPEND_USER_NAME", Null) == "true";
	cfg_append_album_name = data.Get("APPEND_ALBUM_NAME", Null) == "true";
	cfg_enable_user_auth = data.Get("ENABLE_USER_AUTH", Null) == "true";
	cfg_use_https = data.Get("USE_HTTPS", Null) == "true";
	cfg_use_https_proxy = data.Get("USE_HTTP_PROXY", Null) == "true";
	cfg_https_proxy_url = data.Get("HTTP_PROXY_URL", Null);
	cfg_https_proxy_port = data.Get("HTTP_PROXY_PORT", Null);
	int tmp = ScanInt(data.Get("HTTP_TIMEOUT_REQUEST", Null));
	cfg_http_timeout_req = tmp <= min_http_timeout_req ? default_http_timeout_req : tmp;
	tmp = ScanInt(data.Get("HTTP_TIMEOUT_CONNECTION", Null));
	cfg_http_timeout_con = tmp < min_http_timeout_con ? default_http_timeout_con : tmp;

	LoadFromJson(userdata, data.Get("USER_DATA", Null));
}

void Rajce::SaveCfg() {
	String cfg_file = GetCfgFileName();
	String data;

	data << APP_TITLE << ": Configuration Text File"
		 << "\n\n"
		 << "DOWNLOAD_DIR = " << download_dir.GetData() << "\n"
		 << "DOWNLOAD_NEW_ONLY = " << (download_new_only.GetData() ? "true" : "false") << "\n"
		 << "DOWNLOAD_VIDEO = " << (download_video.GetData() ? "true" : "false") << "\n"
		 << "ALBUM_URL = " << album_url.GetData() << "\n"
		 << "ALBUM_USER = " << album_user.GetData() << "\n"
		 << "APPEND_USER_NAME = " << (append_user_name.GetData() ? "true" : "false") << "\n"
		 << "APPEND_ALBUM_NAME = " << (append_album_name.GetData() ? "true" : "false") << "\n"
		 << "ENABLE_USER_AUTH = " << (album_authorization.GetData() ? "true" : "false") << "\n"
		 << "USE_HTTPS = " << (download_protocol.GetData() ? "true" : "false") << "\n"
		 << "USE_HTTP_PROXY = " << (proxy_enabled.GetData() ? "true" : "false") << "\n"
		 << "HTTP_PROXY_URL = " << http_proxy_url.GetData() << "\n"
		 << "HTTP_PROXY_PORT = " << http_proxy_port.GetData() << "\n"
		 << "HTTP_TIMEOUT_REQUEST = " << timeout_req.GetData() << "\n"
		 << "HTTP_TIMEOUT_CONNECTION = " << timeout_con.GetData() << "\n"
		 << "USER_DATA = " << StoreAsJson(userdata) << "\n";

	if (!FileExists(cfg_file)) {
		RealizePath(cfg_file);
	}

	if (!SaveFile(cfg_file, data)) {
		ErrorOK(t_("Configuration file saving has failed!"));
	}
}

String Rajce::GetAppDirectory() {
	String p;
#if defined(PLATFORM_WIN32)
	p = AppendFileName(GetEnv("LOCALAPPDATA"), internal_name);
	ONCELOCK
	RealizeDirectory(p);
#elif defined(PLATFORM_POSIX)
	p = AppendFileName(GetHomeDirectory(), ".config/" + internal_name);
	ONCELOCK
	RealizeDirectory(p);
	String old = AppendFileName(GetHomeDirectory(), "." + internal_name);
	if (DirectoryExists(old)) {
		FileMove(old, p);
	}
#else
	Exclamation("Configuration is not implemented for this platform");
#endif
	return (p);
}

String Rajce::GetCfgFileName() { return (AppendFileName(GetAppDirectory(), internal_name + ".ini")); }

String Rajce::GetOS() {
	String result = "unix";
#if defined(_WIN32)
	result = "windows";
#endif
	return result;
}

String Rajce::sha256sum(const String &filename) const {
	String result;
	Sha256Stream sha256;
	FileIn file(filename);
	int chunk = buffer_size * buffer_size;
	while (!file.IsError() && !file.IsEof()) {
		auto buff = file.Get(chunk);
		if (buff.GetCount() <= 0) {
			break;
		}
		sha256.Put(buff);
	}

	if (!file.IsError()) {
		result = sha256.FinishString();
	}

	return result;
}

// vim: ts=4 sw=4 expandtab
