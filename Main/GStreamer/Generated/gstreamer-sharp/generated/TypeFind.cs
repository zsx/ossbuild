// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Collections;
	using System.Runtime.InteropServices;

#region Autogenerated code
	public partial class TypeFind : Gst.GLib.Opaque {

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_type_find_get_type();

		public static Gst.GLib.GType GType { 
			get {
				IntPtr raw_ret = gst_type_find_get_type();
				Gst.GLib.GType ret = new Gst.GLib.GType(raw_ret);
				return ret;
			}
		}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_type_find_suggest(IntPtr raw, uint probability, IntPtr caps);

		public void Suggest(uint probability, Gst.Caps caps) {
			gst_type_find_suggest(Handle, probability, caps == null ? IntPtr.Zero : caps.Handle);
		}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern ulong gst_type_find_get_length(IntPtr raw);

		public ulong Length { 
			get {
				ulong raw_ret = gst_type_find_get_length(Handle);
				ulong ret = raw_ret;
				return ret;
			}
		}

		public TypeFind(IntPtr raw) : base(raw) {}

#endregion
#region Customized extensions
#line 1 "TypeFind.custom"
[DllImport ("libgstreamer-0.10.dll") ]
static extern IntPtr gst_type_find_peek (IntPtr raw, long offset, uint size);

public byte[] Peek (long offset, uint size) {
  IntPtr raw_ret = gst_type_find_peek (Handle, offset, size);
  if (raw_ret == IntPtr.Zero)
    return null;

  byte[] ret = new byte[size];
  Marshal.Copy (raw_ret, ret, 0, (int) size);
  return ret;
}

private GstSharp.TypeFindPeekFunctionWrapper peek;
private GstSharp.TypeFindSuggestFunctionWrapper suggest;
private GstSharp.TypeFindGetLengthFunctionWrapper get_length;

[DllImport ("gstreamersharpglue-0.10.dll") ]
static extern IntPtr gstsharp_gst_type_find_new (GstSharp.TypeFindPeekFunctionNative peek, GstSharp.TypeFindSuggestFunctionNative suggest, GstSharp.TypeFindGetLengthFunctionNative get_length);



public TypeFind (TypeFindPeekFunction peek, TypeFindSuggestFunction suggest, TypeFindGetLengthFunction get_length) : base () {
  this.peek = new GstSharp.TypeFindPeekFunctionWrapper (peek);
  this.suggest = new GstSharp.TypeFindSuggestFunctionWrapper (suggest);
  this.get_length = new GstSharp.TypeFindGetLengthFunctionWrapper (get_length);

  Raw = gstsharp_gst_type_find_new (this.peek.NativeDelegate, this.suggest.NativeDelegate, this.get_length.NativeDelegate);
  Owned = true;
}

protected override void Free (IntPtr raw) {
  Gst.GLib.Marshaller.Free (raw);
}

#endregion
	}
}
