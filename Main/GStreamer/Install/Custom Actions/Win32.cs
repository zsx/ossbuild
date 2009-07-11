#region Copyright/Legal Notices
/***************************************************
* Copyright (c) 2009 OSSBuild. All rights reserved.
*	Please see included license for more details.
***************************************************/
#endregion

using System;
using System.Runtime.InteropServices;

namespace OSSBuild.Install.CustomActions {
	internal sealed class Win32 {
		public const int HWND_BROADCAST = 0xffff;
		public const int WM_SETTINGCHANGE = 0x001a;

		[DllImport("user32", CharSet=CharSet.Auto, SetLastError=true)]
		public static extern int SendMessage(
			int hWnd,		// handle to destination window 
			uint Msg,		// message 
			uint wParam,	// first message parameter 
			string lParam	// second message parameter 
		);

		public static void NotifySystemEnvVarChanged() {
			SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, "Environment");
		}
	}
}
