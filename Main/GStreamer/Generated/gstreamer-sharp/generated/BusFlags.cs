// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[Flags]
	[Gst.GLib.GType (typeof (Gst.BusFlagsGType))]
	public enum BusFlags {

		Flushing = MiniObjectFlags.Last << 0,
		Last = MiniObjectFlags.Last << 1,
	}

	internal class BusFlagsGType {
		[DllImport ("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_bus_flags_get_type ();

		public static Gst.GLib.GType GType {
			get {
				return new Gst.GLib.GType (gst_bus_flags_get_type ());
			}
		}
	}
#endregion
}
