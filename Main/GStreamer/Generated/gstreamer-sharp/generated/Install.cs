// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst.PbUtils {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	public partial class Install {

		[DllImport("libgstpbutils-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern bool gst_install_plugins_supported();

		public static bool Supported { 
			get {
				bool raw_ret = gst_install_plugins_supported();
				bool ret = raw_ret;
				return ret;
			}
		}

		[DllImport("libgstpbutils-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern bool gst_install_plugins_installation_in_progress();

		public static bool InProgress { 
			get {
				bool raw_ret = gst_install_plugins_installation_in_progress();
				bool ret = raw_ret;
				return ret;
			}
		}

		[DllImport("libgstpbutils-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_install_plugins_sync(IntPtr details, IntPtr ctx);

		public static Gst.PbUtils.InstallPluginsReturn InstallPlugins(string details, Gst.PbUtils.InstallPluginsContext ctx) {
			int raw_ret = gst_install_plugins_sync(Gst.GLib.Marshaller.StringToPtrGStrdup(details), ctx == null ? IntPtr.Zero : ctx.Handle);
			Gst.PbUtils.InstallPluginsReturn ret = (Gst.PbUtils.InstallPluginsReturn) raw_ret;
			return ret;
		}

		[DllImport("libgstpbutils-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_install_plugins_async(IntPtr details, IntPtr ctx, Gst.PbUtilsSharp.InstallPluginsResultFuncNative func, IntPtr user_data);

		public static Gst.PbUtils.InstallPluginsReturn InstallPlugins(string details, Gst.PbUtils.InstallPluginsContext ctx, Gst.PbUtils.InstallPluginsResultFunc func) {
			Gst.PbUtilsSharp.InstallPluginsResultFuncWrapper func_wrapper = new Gst.PbUtilsSharp.InstallPluginsResultFuncWrapper (func);
			int raw_ret = gst_install_plugins_async(Gst.GLib.Marshaller.StringToPtrGStrdup(details), ctx == null ? IntPtr.Zero : ctx.Handle, func_wrapper.NativeDelegate, IntPtr.Zero);
			Gst.PbUtils.InstallPluginsReturn ret = (Gst.PbUtils.InstallPluginsReturn) raw_ret;
			return ret;
		}

#endregion
	}
}
