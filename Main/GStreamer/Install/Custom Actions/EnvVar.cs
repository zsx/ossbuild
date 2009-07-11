#region Copyright/Legal Notices
/***************************************************
* Copyright (c) 2009 OSSBuild. All rights reserved.
*	Please see included license for more details.
***************************************************/
#endregion

using System;
using System.IO;
using System.Text;
using System.ComponentModel;
using System.Configuration.Install;
using System.Collections.Generic;
using Microsoft.Win32;

namespace OSSBuild.Install.CustomActions {
	///<summary>
	///	This code was provided courtesy Anthony Glenwright
	///	
	///	http://www.devx.com/dotnet/Article/20849/0/page/1
	///</summary>
	[System.ComponentModel.RunInstaller(true)]
	public class EnvVar : System.Configuration.Install.Installer {
		#region Install/Uninstall Methods
		public override void Install(System.Collections.IDictionary stateSaver) {
			base.Install(stateSaver);

			//Use a passed-in argument from the setup project or use the path that this assembly 
			//has been installed to.
			//string[] args = System.Environment.GetCommandLineArgs();
			string myPath = MyPath(); //(args != null && args.Length > 1 ? args[1] : MyPath());
			string pluginPath;

			pluginPath = Path.Combine(myPath, "lib\\gstreamer-0.10");
			myPath = Path.Combine(myPath, "bin");

			string curPath;
			string newPath;

			#region Append to system path
			//Try the system path
			curPath = GetSystemPath();
			if (!string.IsNullOrEmpty(curPath)) {
				stateSaver.Add("previousSystemPath", curPath);
				newPath = AddPath(curPath, myPath);

				if (curPath != newPath) {
					stateSaver.Add("changedSystemPath", true);
					SetPath(newPath);
				} else
					stateSaver.Add("changedSystemPath", false);
			} else {
				//Add it to the user's path
				curPath = GetUserPath();
				stateSaver.Add("previousUserPath", curPath);
				newPath = AddPath(curPath, myPath);
				if (curPath != newPath) {
					stateSaver.Add("changedUserPath", true);
					SetPath(newPath);
				} else
					stateSaver.Add("changedUserPath", false);
			}
			#endregion

			#region Add GST_PLUGIN_PATH env var
			if (!AddSystemEnvironmentVariable("GST_PLUGIN_PATH", pluginPath))
				AddUserEnvironmentVariable("GST_PLUGIN_PATH", pluginPath);
			#endregion

			try { Win32.NotifySystemEnvVarChanged(); } catch { }
		}

		public override void Uninstall(System.Collections.IDictionary savedState) {
			base.Uninstall(savedState);

			//string[] args = System.Environment.GetCommandLineArgs();
			string myPath = MyPath(); //(args != null && args.Length > 1 ? args[1] : MyPath());

			try { if ((bool)savedState["changedSystemPath"]) SetPath(RemovePath(GetSystemPath(), myPath)); } catch { }
			try { if ((bool)savedState["changedUserPath"]) SetPath(RemovePath(GetUserPath(), myPath)); } catch { }

			try { RemoveSystemEnvironmentVariable("GST_PLUGIN_PATH"); } catch { }
			try { RemoveUserEnvironmentVariable("GST_PLUGIN_PATH"); } catch { }

			try { Win32.NotifySystemEnvVarChanged(); } catch { }
		}

		public override void Rollback(System.Collections.IDictionary savedState) {
			base.Rollback(savedState);
			
			try { if ((bool)savedState["changedSystemPath"]) SetPath((string)savedState["previousSystemPath"]); } catch { }
			try { if ((bool)savedState["changedUserPath"]) SetPath((string)savedState["previousUserPath"]); } catch { }

			try { RemoveSystemEnvironmentVariable("GST_PLUGIN_PATH"); } catch { }
			try { RemoveUserEnvironmentVariable("GST_PLUGIN_PATH"); } catch { }

			try { Win32.NotifySystemEnvVarChanged(); } catch { }
		}
		#endregion

		#region Helper Methods
		private static string MyPath() {
			string myFile = System.Reflection.Assembly.GetExecutingAssembly().Location;
			string myPath = System.IO.Path.GetDirectoryName(myFile);
			return myPath;
		}

		private static RegistryKey GetUserPathRegKey(bool writable) {
			// for the user-specific path...
			return Registry.CurrentUser.OpenSubKey("Environment", writable);
		}

		private static RegistryKey GetSystemPathRegKey(bool writable) {
			// for the system-wide path...
			return Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Control\Session Manager\Environment", writable);
		}

		private static void SetPath(string value) {
			using (RegistryKey reg = GetUserPathRegKey(true)) {
				reg.SetValue("Path", value, RegistryValueKind.ExpandString);
			}
		}

		private static string GetUserPath() {
			using (RegistryKey reg = GetUserPathRegKey(false)) {
				return (string)reg.GetValue("Path", "", RegistryValueOptions.DoNotExpandEnvironmentNames);
			}
		}

		private static string GetSystemPath() {
			try {
				using (RegistryKey reg = GetSystemPathRegKey(false)) {
					return (string)reg.GetValue("Path", "", RegistryValueOptions.DoNotExpandEnvironmentNames);
				}
			} catch {
				return string.Empty;
			}
		}

		private static string AddPath(string list, string item) {
			List<string> paths = new List<string>(list.Split(Path.PathSeparator));

			foreach (string path in paths) {
				if (string.Compare(path, item, true) == 0) {
					// already present
					return list;
				}
			}

			paths.Add(item);
			return string.Join(string.Empty + Path.PathSeparator, paths.ToArray());
		}

		private static string RemovePath(string list, string item) {
			List<string> paths = new List<string>(list.Split(Path.PathSeparator));

			for (int i = 0; i < paths.Count; i++) {
				if (string.Compare(paths[i], item, true) == 0) {
					paths.RemoveAt(i);
					return string.Join(string.Empty + Path.PathSeparator, paths.ToArray());
				}
			}

			// not present
			return list;
		}

		private static bool AddUserEnvironmentVariable(string name, string value) {
			try {
				using (RegistryKey reg = Registry.CurrentUser.OpenSubKey("Environment", true)) {
					reg.SetValue(name, value, RegistryValueKind.ExpandString);
				}
				return true;
			} catch {
				return false;
			}
		}

		private static bool AddSystemEnvironmentVariable(string name, string value) {
			try {
				using (RegistryKey reg = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Control\Session Manager\Environment", true)) {
					reg.SetValue(name, value, RegistryValueKind.ExpandString);
				}
				return true;
			} catch {
				return false;
			}
		}

		private static bool RemoveUserEnvironmentVariable(string name) {
			try {
				using (RegistryKey reg = Registry.CurrentUser.OpenSubKey("Environment", true)) {
					reg.DeleteValue(name, false);
				}
				return true;
			} catch {
				return false;
			}
		}

		private static bool RemoveSystemEnvironmentVariable(string name) {
			try {
				using (RegistryKey reg = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Control\Session Manager\Environment", true)) {
					reg.DeleteValue(name, false);
				}
				return true;
			} catch {
				return false;
			}
		}
		#endregion
	}
}
