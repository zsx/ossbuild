// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	[Gst.GLib.GType (typeof (Gst.IndexEntryTypeGType))]
	public enum IndexEntryType {

		Id,
		Association,
		Object,
		Format,
	}

	internal class IndexEntryTypeGType {
		[DllImport ("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_index_entry_type_get_type ();

		public static Gst.GLib.GType GType {
			get {
				return new Gst.GLib.GType (gst_index_entry_type_get_type ());
			}
		}
	}
#endregion
}
