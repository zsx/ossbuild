// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[Flags]
	[Gst.GLib.GType (typeof (Gst.ParseFlagsGType))]
	public enum ParseFlags {

		None,
		FatalErrors = 1 << 0,
	}

	internal class ParseFlagsGType {
		[DllImport ("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_parse_flags_get_type ();

		public static Gst.GLib.GType GType {
			get {
				return new Gst.GLib.GType (gst_parse_flags_get_type ());
			}
		}
	}
#endregion
}
