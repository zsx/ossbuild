// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[Flags]
	[Gst.GLib.GType (typeof (Gst.ElementFlagsGType))]
	public enum ElementFlags {

		LockedState = ObjectFlags.Last << 0,
		IsSink = ObjectFlags.Last << 1,
		Unparenting = ObjectFlags.Last << 2,
		Last = ObjectFlags.Last << 16,
	}

	internal class ElementFlagsGType {
		[DllImport ("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_element_flags_get_type ();

		public static Gst.GLib.GType GType {
			get {
				return new Gst.GLib.GType (gst_element_flags_get_type ());
			}
		}
	}
#endregion
}