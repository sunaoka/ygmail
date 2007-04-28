/***********************************************************************
 * Copyright (C) 2004, 2007 SUNAOKA Norifumi All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "BeckyAPI.h"
#include "BkCommon.h"
#include "resource.h"
#include "SetupDlg.h"
#include "ygmail.h"
#include <string>	// STL
#include <map>		// STL

extern std::map<std::string, std::string> g_maillist;
extern char szIni[_MAX_PATH+2];

BOOL InitDialog(HWND hWnd) {

	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST);
	LV_COLUMN	lc;
	LV_ITEM		li;
	int i = 0;
	RECT rect;

	// コモンコントロール初期化
	InitCommonControls();
	ListView_SetExtendedListViewStyle(hWndLV, LVS_EX_FULLROWSELECT);

	GetClientRect(hWndLV, &rect);
	
	lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lc.fmt = LVCFMT_LEFT;
	lc.cx = rect.right / 2;

	lc.pszText = "メールアドレス (From)";
	lc.iSubItem = 0;
	lc.cchTextMax = strlen(lc.pszText);
	ListView_InsertColumn(hWndLV, 0, &lc);
	
	lc.pszText = "サウンド";
	lc.iSubItem = 1;
	lc.cchTextMax = strlen(lc.pszText);
	ListView_InsertColumn(hWndLV, 1, &lc);

	std::map<std::string, std::string>::iterator itr;
	for (itr = g_maillist.begin(); itr != g_maillist.end(); itr++) {
		li.mask = LVIF_TEXT;
		li.iItem = i;

		li.pszText = (char *)((*itr).first).c_str();
		li.iSubItem = 0;
		ListView_InsertItem(hWndLV, &li);

		li.pszText = (char *)((*itr).second).c_str();
		li.iSubItem = 1;
		ListView_SetItem(hWndLV, &li);

		i++;
	}

	EnableWindow(GetDlgItem(hWnd, IDC_MODIFY), FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_DELETE), FALSE);

	return TRUE;
}

BOOL OnOK(HWND hWnd) {
	
	char szEMail[256], szSound[256];
	char szKey[256];
	char szBuff[32768];
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST);

	// マップを全削除
	g_maillist.clear();
	// ini ファイルを初期化
	WritePrivateProfileSection("Settings", "", szIni);

	// リストビューを走査
	int nListCount = ListView_GetItemCount(hWndLV);
	for (int i = 0; i < nListCount; i++) {
		ListView_GetItemText(hWndLV, i, 0, szEMail, sizeof(szEMail));
		ListView_GetItemText(hWndLV, i, 1, szSound, sizeof(szSound));
		// マップに追加
		g_maillist.insert(std::pair<std::string, std::string>(szEMail, szSound));
		// ini ファイルに書き込み
		sprintf(szKey, "%d", i);
		sprintf(szBuff, "%s,%s", szEMail, szSound);
		WritePrivateProfileString("Settings", szKey, szBuff, szIni);
	}

	return TRUE;
}

BOOL OnEdit(HWND hWnd, BOOL bAdd) {

	char szEMail[256], szSound[256];
	int nLen;
	LV_ITEM	li;
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST);

	nLen = GetWindowText(GetDlgItem(hWnd, IDC_EMAIL), szEMail, sizeof(szEMail));
	if (nLen == 0) {
		MessageBox(hWnd, "メールアドレスを記入してください。", "エラー", MB_OK | MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hWnd, IDC_MAIL));
		return FALSE;
	}
	
	nLen = GetWindowText(GetDlgItem(hWnd, IDC_SOUND), szSound, sizeof(szSound));
	if (nLen == 0) {
		MessageBox(hWnd, "wav ファイル名を記入してください。", "エラー", MB_OK | MB_ICONEXCLAMATION);
		SetFocus(GetDlgItem(hWnd, IDC_SOUND));
		return FALSE;
	}

	if (bAdd) {
		// 追加
		// メールアドレスが重複していないかチェック
		int nListCount = ListView_GetItemCount(hWndLV);
		for (int i = 0; i < nListCount; i++) {
			char buf[256];
			ListView_GetItemText(hWndLV, i, 0, buf, sizeof(buf));
			if (__stristr(buf, szEMail)) { // BkCommon.h
				MessageBox(hWnd, "指定したメールアドレスは、すでに登録されています。", "エラー", MB_OK | MB_ICONEXCLAMATION);
				SetFocus(GetDlgItem(hWnd, IDC_MAIL));
				return FALSE;
			}
		}
		
		li.mask = LVIF_TEXT;
		li.iItem = nListCount;

		li.pszText = szEMail;
		li.iSubItem = 0;
		ListView_InsertItem(hWndLV, &li);

		li.pszText = szSound;
		li.iSubItem = 1;
		ListView_SetItem(hWndLV, &li);
	} else {
		// 変更
		int nItem = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
		if (nItem != -1) {
			li.mask = LVIF_TEXT;
			li.iItem = nItem;

			li.pszText = szEMail;
			li.iSubItem = 0;
			ListView_SetItem(hWndLV, &li);

			li.pszText = szSound;
			li.iSubItem = 1;
			ListView_SetItem(hWndLV, &li);
			// 選択状態を解除
			ListView_SetItemState(hWndLV, nItem, 0, LVIS_SELECTED);
		}
	}

	SetWindowText(GetDlgItem(hWnd, IDC_EMAIL), "");
	SetWindowText(GetDlgItem(hWnd, IDC_SOUND), "");
	
	return TRUE;
}

BOOL OnDelete(HWND hWnd) {

	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST);
	int nItem = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
	if (nItem != -1) {
		if (MessageBox(hWnd, "アイテムを削除しますか。", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
			ListView_DeleteItem(hWndLV, nItem);
		} else {
			SetFocus(hWndLV);
		}
	}
	return TRUE;
}

BOOL OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) {

	if (LOWORD(wParam) != IDC_LIST) {
		return TRUE;
	}

	LV_DISPINFO *lvinfo = (LV_DISPINFO *)lParam;
	
	if (lvinfo->hdr.code != LVN_ITEMCHANGED) {
		return TRUE;
	}

	char szEMail[256], szSound[256];
	HWND hWndLV = GetDlgItem(hWnd, IDC_LIST);
	int nItem = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
	
	if (nItem != -1) {
		// アイテムが選択された
		ListView_GetItemText(hWndLV, nItem, 0, szEMail, sizeof(szEMail));
		ListView_GetItemText(hWndLV, nItem, 1, szSound, sizeof(szSound));
		SetWindowText(GetDlgItem(hWnd, IDC_EMAIL), szEMail);
		SetWindowText(GetDlgItem(hWnd, IDC_SOUND), szSound);
		EnableWindow(GetDlgItem(hWnd, IDC_MODIFY), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_DELETE), TRUE);
	} else {
		EnableWindow(GetDlgItem(hWnd, IDC_MODIFY), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_DELETE), FALSE);
		SetWindowText(GetDlgItem(hWnd, IDC_EMAIL), "");
		SetWindowText(GetDlgItem(hWnd, IDC_SOUND), "");
	}

	return TRUE;

}

BOOL OnOpenFile(HWND hWnd) {
	
	OPENFILENAME ofn;
	char szFile[MAX_PATH], szTitle[64];
	szFile[0] = '\0';

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "サウンド (*.wav)\0*.wav\0\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = szTitle;
	ofn.nMaxFileTitle = 64;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "wav";

	if (GetOpenFileName(&ofn)) {
		SetWindowText(GetDlgItem(hWnd, IDC_SOUND), szFile);
	}

	return TRUE;
}

BOOL CALLBACK SetupDlgFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_INITDIALOG:
		return InitDialog(hWnd);

	case WM_NOTIFY:
		OnNotify(hWnd, wParam, lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))	{
		case IDOK:
			OnOK(hWnd);
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;
		
		case IDC_ADD:		// アイテムの追加
			OnEdit(hWnd, TRUE);
			break;

		case IDC_MODIFY:	// アイテムの変更
			OnEdit(hWnd, FALSE);
			break;
		
		case IDC_DELETE:	// アイテムの削除
			OnDelete(hWnd);
			break;
		
		case IDC_OPENFILE:	// ファイルの参照
			OnOpenFile(hWnd);
			break;
		}
	}
	return FALSE;
}
