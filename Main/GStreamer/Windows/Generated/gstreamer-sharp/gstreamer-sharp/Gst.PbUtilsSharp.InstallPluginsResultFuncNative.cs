// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst.PbUtilsSharp {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
	internal delegate void InstallPluginsResultFuncNative(int result, IntPtr user_data);

	internal class InstallPluginsResultFuncInvoker {

		InstallPluginsResultFuncNative native_cb;
		IntPtr __data;
		Gst.GLib.DestroyNotify __notify;

		~InstallPluginsResultFuncInvoker ()
		{
			if (__notify == null)
				return;
			__notify (__data);
		}

		internal InstallPluginsResultFuncInvoker (InstallPluginsResultFuncNative native_cb) : this (native_cb, IntPtr.Zero, null) {}

		internal InstallPluginsResultFuncInvoker (InstallPluginsResultFuncNative native_cb, IntPtr data) : this (native_cb, data, null) {}

		internal InstallPluginsResultFuncInvoker (InstallPluginsResultFuncNative native_cb, IntPtr data, Gst.GLib.DestroyNotify notify)
		{
			this.native_cb = native_cb;
			__data = data;
			__notify = notify;
		}

		internal Gst.PbUtils.InstallPluginsResultFunc Handler {
			get {
				return new Gst.PbUtils.InstallPluginsResultFunc(InvokeNative);
			}
		}

		void InvokeNative (Gst.PbUtils.InstallPluginsReturn result)
		{
			native_cb ((int) result, __data);
		}
	}

	internal class InstallPluginsResultFuncWrapper {

		public void NativeCallback (int result, IntPtr user_data)
		{
			try {
				managed ((Gst.PbUtils.InstallPluginsReturn) result);
				if (release_on_call)
					gch.Free ();
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, false);
			}
		}

		bool release_on_call = false;
		GCHandle gch;

		public void PersistUntilCalled ()
		{
			release_on_call = true;
			gch = GCHandle.Alloc (this);
		}

		internal InstallPluginsResultFuncNative NativeDelegate;
		Gst.PbUtils.InstallPluginsResultFunc managed;

		public InstallPluginsResultFuncWrapper (Gst.PbUtils.InstallPluginsResultFunc managed)
		{
			this.managed = managed;
			if (managed != null)
				NativeDelegate = new InstallPluginsResultFuncNative (NativeCallback);
		}

		public static Gst.PbUtils.InstallPluginsResultFunc GetManagedDelegate (InstallPluginsResultFuncNative native)
		{
			if (native == null)
				return null;
			InstallPluginsResultFuncWrapper wrapper = (InstallPluginsResultFuncWrapper) native.Target;
			if (wrapper == null)
				return null;
			return wrapper.managed;
		}
	}
#endregion
}
