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

#include "Rajce.h"

void Rajce::UpgradeCheck(void)
{
	String bite_size = sizeof(void *) == 8 ? "64bit" : "32bit";
	Progress progress(t_("Checking latest version"));
	progress.Step();

	CtrlLayout(upgrade, t_("Download latest version"));

	upgrade.Rejector(upgrade.cancel, IDCANCEL);
	upgrade.WhenClose = upgrade.Rejector(IDCANCEL);
	upgrade.Acceptor(upgrade.ok, IDOK);
	upgrade.ok.Ok();
	upgrade.cancel.Cancel();
	upgrade.cancel.WhenPush = THISBACK(UpgradeAbort);
	upgrade.Sizeable();
	upgrade.pi.Set(0, 1);
	upgrade.check_sha256 <<= 1;

	upgrade_sz = upgrade.GetMinSize();
	release_sz = upgrade.release_label.GetSize();

	UpgradeToggleElements(false);
	upgrade.Open();
	UpgradeDownloadVersion(bite_size);

	String file_name = GetFileName(upgrade_url);
	String download_path = download_dir.GetData();

	if (download_dir.GetLength() == 0)
		download_path = Nvl(GetDownloadFolder(), GetHomeDirFile("downloads"));

	upgrade.actual.SetText(version);
	upgrade.latest.SetText(upgrade_version);
	upgrade.file_name.SetText(file_name);
	upgrade.download_url.SetText(upgrade_url);
	upgrade.download_url.SetForeground();
	upgrade.download_dir.SetText(download_path);
	upgrade.download_dir_select.WhenPush = THISBACK(UpgradeSelectDirectory);
	upgrade.pi.Set(0, 1);

	int tag = VersionToInt(upgrade_version);
	int ver = VersionToInt(version);

	UpgradeToggleElements(tag > ver);

	progress.Close();

	if (upgrade.Run() == IDOK) {
		UpgradeToggleElements(false);
		UpgradeDownload(upgrade.download_dir.GetData(), upgrade.file_name.GetData());
		UpgradeToggleElements(true);
	}

	upgrade.Close();
}

void Rajce::UpgradeSelectDirectory(void)
{
	SelectDirButton select_download_dir;
	String active_dir = ~upgrade.download_dir;
	select_download_dir.ActiveDir(active_dir);
	select_download_dir.ExecuteSelectDir(active_dir);
	upgrade.download_dir <<= select_download_dir.GetActiveDir();
}

void Rajce::UpgradeDownloadVersion(const String bite_size)
{
	String check_url = "https://api.github.com/repos/coolmancz/rajce/releases";
	upgrade_release.Clear();

	if (ERR_NO_ERROR == HttpDownloadPage(check_url, upgrade_http, upgrade_http_out, upgrade_http_out_string, false)) {
		String tmp_file = upgrade_http_out_string;
		Value data = ParseJSON(LoadFile(tmp_file));
		String os = GetOS();

		String version_last;
		int tag = VersionToInt(upgrade_version);
		int ver = VersionToInt(version);
		int tag_last = tag;

		for (int j = 0; j < data.GetCount(); ++j) {
			int tag_cur = VersionToInt(data[j]["tag_name"]);

			if (tag_cur > ver)
				upgrade_release += "\n\n" + data[j]["body"].ToString();

			if (tag_cur > tag_last) {
				tag_last = tag_cur;
				upgrade_version = data[j]["tag_name"].ToString();

				Value assets = data[j]["assets"];
				for (int i = 0; i < assets.GetCount(); ++i) {
					String tmp = assets[i]["browser_download_url"].ToString();
					int len = tmp.GetLength();

					String test = bite_size + ".zip";
					if (os.Compare("windows") == 0 && tmp.Mid(len - test.GetLength()).Find(test) == 0) {
						upgrade_size = assets[i]["size"];
						upgrade_url = tmp;
						continue;
					}
					test += ".sha256";
					if (os.Compare("windows") == 0 && tmp.Mid(len - test.GetLength()).Find(test) == 0) {
						upgrade_url_sha256 = tmp;
						continue;
					}

					test = "tar.bz2";
					if (os.Compare("unix") == 0 && tmp.Mid(len - test.GetLength()).Find(test) == 0) {
						upgrade_size = assets[i]["size"];
						upgrade_url = tmp;
						continue;
					}
					test += ".sha256";
					if (os.Compare("unix") == 0 && tmp.Mid(len - test.GetLength()).Find(test) == 0) {
						upgrade_url_sha256 = tmp;
						continue;
					}
				}
			}
		}

		if (FileExists(tmp_file))
			DeleteFile(tmp_file);
	} else
		ErrorOK(t_("Latest version check failed!"));

	upgrade.release.SetQTF(DeQtf(upgrade_release), Zoom(96, 600));
	upgrade.release.SetZoom(Zoom(1,1));
}

void Rajce::UpgradeDownload(const String download_path, const String download_file)
{
	if (upgrade_url.IsEmpty()) {
		ErrorOK(t_("Download URL is empty!"));
		return;
	}

	if (!DirectoryExists(download_path)) {
		ErrorOK(Format("[= %s&& %s]", t_("Download directory doesn't exist."), DeQtf(download_path)));
		return;
	}
	String file_path = AppendFileName(download_path, download_file);
	String file_path_sha256 = file_path + ".sha256";

	String upgrade_sha256;
	if (upgrade.check_sha256.Get() && upgrade_url_sha256.GetCount()) {
		if (ERR_NO_ERROR == HttpDownloadPage(upgrade_url_sha256, upgrade_http, upgrade_http_out, upgrade_http_out_string, false)) {
			String tmp_file = upgrade_http_out_string;

			if (FileExists(tmp_file)) {
				if(!FileCopy(tmp_file, file_path_sha256))
					ErrorOK(t_("[= Rename file failed!&\1") + file_path_sha256);
				DeleteFile(tmp_file);
			}
			FileIn fi(file_path_sha256);
			String line = fi.GetLine();
			upgrade_sha256 = line.Mid(1, 64);
			fi.Close();
		} else
			ErrorOK(Format("[= %s&&%s]", t_("SHA256 hash file download failed!"), DeQtf(file_path_sha256)));
	}

	if (FileExists(file_path)) {
		if (upgrade.check_sha256.Get() && upgrade_sha256.GetCount()) {
			String sha256 = sha256sum(file_path);
			if (upgrade_sha256.Compare(sha256) != 0) {
				ErrorOK(Format("[= %s&&%s]", t_("The new version has been already downloaded, but sha256 hash doesn't match!"), DeQtf(file_path)));
				return;
			}
		}
		if (upgrade_size == GetFileLength(file_path))
			Exclamation(Format("[= %s&&%s]", t_("The new version has been already downloaded."), DeQtf(file_path)));
		else
			ErrorOK(Format("[= %s&&%s]", t_("The new version has been already downloaded, but file size is wrong!"), DeQtf(file_path)));
		return;
	}

	if (upgrade_url.GetCount() && ERR_NO_ERROR == HttpDownloadPage(upgrade_url, upgrade_http, upgrade_http_out, upgrade_http_out_string, false)) {
		String tmp_file = upgrade_http_out_string;

		if (FileExists(tmp_file)) {
			if(!FileCopy(tmp_file, file_path))
				ErrorOK(t_("[= Rename file failed!&\1") + file_path);
			DeleteFile(tmp_file);
		}

		if (upgrade.check_sha256.Get() && upgrade_sha256.GetCount()) {
			String sha256 = sha256sum(file_path);
			if (upgrade_sha256.Compare(sha256) != 0) {
				ErrorOK(Format("[= %s&&%s]", t_("The new version has been downloaded, but sha256 hash doesn't match!"), DeQtf(file_path)));
				return;
			}
		}

		if (FileExists(file_path)) {
			if (upgrade_size == GetFileLength(file_path))
				Exclamation(Format("[= %s&& %s]", t_("The new version successfully downloaded."), DeQtf(file_path)));
			else
				ErrorOK(Format("[= %s&&%s]", t_("The new version has been downloaded, but file size is wrong!"), DeQtf(file_path)));
		}
	} else
		ErrorOK(Format("[= %s&& %s]", t_("The new version download failed!"), DeQtf(file_path)));
}

void Rajce::UpgradeStart(void)
{
	if (upgrade_http_out.IsOpen()) {
		upgrade_http_out.Close();
		DeleteFile(upgrade_http_out_string);
	}
	upgrade_http_loaded = 0;
}

void Rajce::UpgradeContent(const void *ptr, int size)
{
	upgrade_http_loaded += size;
	if (!upgrade_http_out.IsOpen()) {
		RealizePath(upgrade_http_out_string);
		upgrade_http_out.Open(upgrade_http_out_string);
	}
	upgrade_http_out.Put(ptr, size);
	Ctrl::ProcessEvents();
}

void Rajce::UpgradeProgress(void)
{
	if (upgrade_http.GetContentLength() >= 0)
		upgrade.pi.Set((int)upgrade_http_loaded, (int)upgrade_http.GetContentLength());
	else
		upgrade.pi.Set(0, 1);
}

void Rajce::UpgradeAbort(void)
{
	int phase = upgrade_http.GetPhase();
	if (phase > 0 && phase < Upp::HttpRequest::FINISHED && PromptOKCancel(t_("Abort download?"))) {
		UpgradeToggleElements(true);
		upgrade_http.Abort();
	}
}

void Rajce::UpgradeToggleElements(bool enable)
{
	upgrade.file_name.Enable(enable);
	upgrade.download_dir.Enable(enable);
	upgrade.download_dir_select.Enable(enable);
	upgrade.check_sha256.Enable(enable);
	upgrade.ok.Enable(enable);
	UpgradeToggleRelease(enable);
}

void Rajce::UpgradeToggleRelease(bool enable)
{
	Rect main_rc = upgrade.GetRect();
	Size main_sz = main_rc.GetSize();

	upgrade.release.Show(enable);
	upgrade.release_label.Show(enable);

	if (enable) {
		if (main_sz.cy < upgrade_sz.cy) {
			main_sz.cy += release_sz.cy;
			main_rc.InflateVert(release_sz.cy / 2);
			main_rc.SetSize(main_sz);
		}
		upgrade.SetMinSize(upgrade_sz);
	} else {
		if (main_sz.cy >= upgrade_sz.cy) {
			main_sz.cy -= release_sz.cy;
			main_rc.DeflateVert(release_sz.cy / 2);
			main_rc.SetSize(main_sz);

			Size sz = upgrade_sz;
			sz.cy -= release_sz.cy;
			upgrade.SetMinSize(sz);
		}
	}
	upgrade.SetRect(main_rc);
	UpdateLayout();
}

