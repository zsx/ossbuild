// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[Flags]
	[Gst.GLib.GType (typeof (Gst.PadFlagsGType))]
	public enum PadFlags {

		Blocked = ObjectFlags.Last << 0,
		Flushing = ObjectFlags.Last << 1,
		InGetcaps = ObjectFlags.Last << 2,
		InSetcaps = ObjectFlags.Last << 3,
		Blocking = ObjectFlags.Last << 4,
		Last = ObjectFlags.Last << 8,
	}

	internal class PadFlagsGType {
		[DllImport ("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_pad_flags_get_type ();

		public static Gst.GLib.GType GType {
			get {
				return new Gst.GLib.GType (gst_pad_flags_get_type ());
			}
		}
	}
#endregion
}
