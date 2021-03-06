// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace GstSharp {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
	internal delegate bool PadEventProbeCallbackNative(IntPtr pad, IntPtr evnt, IntPtr user_data);

	internal class PadEventProbeCallbackInvoker {

		PadEventProbeCallbackNative native_cb;
		IntPtr __data;
		Gst.GLib.DestroyNotify __notify;

		~PadEventProbeCallbackInvoker ()
		{
			if (__notify == null)
				return;
			__notify (__data);
		}

		internal PadEventProbeCallbackInvoker (PadEventProbeCallbackNative native_cb) : this (native_cb, IntPtr.Zero, null) {}

		internal PadEventProbeCallbackInvoker (PadEventProbeCallbackNative native_cb, IntPtr data) : this (native_cb, data, null) {}

		internal PadEventProbeCallbackInvoker (PadEventProbeCallbackNative native_cb, IntPtr data, Gst.GLib.DestroyNotify notify)
		{
			this.native_cb = native_cb;
			__data = data;
			__notify = notify;
		}

		internal Gst.PadEventProbeCallback Handler {
			get {
				return new Gst.PadEventProbeCallback(InvokeNative);
			}
		}

		bool InvokeNative (Gst.Pad pad, Gst.Event evnt)
		{
			bool result = native_cb (pad == null ? IntPtr.Zero : pad.Handle, evnt == null ? IntPtr.Zero : evnt.Handle, __data);
			return result;
		}
	}

	internal class PadEventProbeCallbackWrapper {

		public bool NativeCallback (IntPtr pad, IntPtr evnt, IntPtr user_data)
		{
			try {
				bool __ret = managed (Gst.GLib.Object.GetObject(pad) as Gst.Pad, Gst.MiniObject.GetObject(evnt) as Gst.Event);
				if (release_on_call)
					gch.Free ();
				return __ret;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, false);
				return false;
			}
		}

		bool release_on_call = false;
		GCHandle gch;

		public void PersistUntilCalled ()
		{
			release_on_call = true;
			gch = GCHandle.Alloc (this);
		}

		internal PadEventProbeCallbackNative NativeDelegate;
		Gst.PadEventProbeCallback managed;

		public PadEventProbeCallbackWrapper (Gst.PadEventProbeCallback managed)
		{
			this.managed = managed;
			if (managed != null)
				NativeDelegate = new PadEventProbeCallbackNative (NativeCallback);
		}

		public static Gst.PadEventProbeCallback GetManagedDelegate (PadEventProbeCallbackNative native)
		{
			if (native == null)
				return null;
			PadEventProbeCallbackWrapper wrapper = (PadEventProbeCallbackWrapper) native.Target;
			if (wrapper == null)
				return null;
			return wrapper.managed;
		}
	}
#endregion
}
