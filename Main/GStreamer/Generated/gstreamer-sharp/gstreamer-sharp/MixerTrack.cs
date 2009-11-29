// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst.Interfaces {

	using System;
	using System.Collections;
	using System.Runtime.InteropServices;

#region Autogenerated code
	public partial class MixerTrack : Gst.GLib.Object {

		public MixerTrack(IntPtr raw) : base(raw) {}

		protected MixerTrack() : base(IntPtr.Zero)
		{
			CreateNativeObject (new string [0], new Gst.GLib.Value [0]);
		}

		[Gst.GLib.Property ("label")]
		public string Label {
			get {
				Gst.GLib.Value val = GetProperty ("label");
				string ret = (string) val;
				val.Dispose ();
				return ret;
			}
		}

		[Gst.GLib.Property ("min-volume")]
		public int MinVolume {
			get {
				Gst.GLib.Value val = GetProperty ("min-volume");
				int ret = (int) val;
				val.Dispose ();
				return ret;
			}
		}

		[Gst.GLib.Property ("max-volume")]
		public int MaxVolume {
			get {
				Gst.GLib.Value val = GetProperty ("max-volume");
				int ret = (int) val;
				val.Dispose ();
				return ret;
			}
		}

		[Gst.GLib.Property ("index")]
		public uint Index {
			get {
				Gst.GLib.Value val = GetProperty ("index");
				uint ret = (uint) val;
				val.Dispose ();
				return ret;
			}
		}

		[Gst.GLib.Property ("flags")]
		public uint Flags {
			get {
				Gst.GLib.Value val = GetProperty ("flags");
				uint ret = (uint) val;
				val.Dispose ();
				return ret;
			}
		}

		[Gst.GLib.Property ("untranslated-label")]
		public string UntranslatedLabel {
			get {
				Gst.GLib.Value val = GetProperty ("untranslated-label");
				string ret = (string) val;
				val.Dispose ();
				return ret;
			}
		}

		[Gst.GLib.Property ("num-channels")]
		public int NumChannels {
			get {
				Gst.GLib.Value val = GetProperty ("num-channels");
				int ret = (int) val;
				val.Dispose ();
				return ret;
			}
		}

		[StructLayout (LayoutKind.Sequential)]
		struct GstMixerTrackClass {
			[MarshalAs (UnmanagedType.ByValArray, SizeConst=4)]
			public IntPtr[] GstReserved;
		}

		static uint class_offset = ((Gst.GLib.GType) typeof (Gst.GLib.Object)).GetClassSize ();
		static Hashtable class_structs;

		static GstMixerTrackClass GetClassStruct (Gst.GLib.GType gtype, bool use_cache)
		{
			if (class_structs == null)
				class_structs = new Hashtable ();

			if (use_cache && class_structs.Contains (gtype))
				return (GstMixerTrackClass) class_structs [gtype];
			else {
				IntPtr class_ptr = new IntPtr (gtype.GetClassPtr ().ToInt64 () + class_offset);
				GstMixerTrackClass class_struct = (GstMixerTrackClass) Marshal.PtrToStructure (class_ptr, typeof (GstMixerTrackClass));
				if (use_cache)
					class_structs.Add (gtype, class_struct);
				return class_struct;
			}
		}

		static void OverrideClassStruct (Gst.GLib.GType gtype, GstMixerTrackClass class_struct)
		{
			IntPtr class_ptr = new IntPtr (gtype.GetClassPtr ().ToInt64 () + class_offset);
			Marshal.StructureToPtr (class_struct, class_ptr, false);
		}

		[DllImport("libgstinterfaces-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_mixer_track_get_type();

		public static new Gst.GLib.GType GType { 
			get {
				IntPtr raw_ret = gst_mixer_track_get_type();
				Gst.GLib.GType ret = new Gst.GLib.GType(raw_ret);
				return ret;
			}
		}


		static MixerTrack ()
		{
			GtkSharp.GstreamerSharp.ObjectManager.Initialize ();
		}
#endregion
#region Customized extensions
#line 1 "MixerTrack.custom"
[DllImport ("gstreamersharpglue-0.10.dll") ]
extern static uint gst__interfacessharp_gst__interfaces_mixertrack_get_label_offset ();
static uint label_offset = gst__interfacessharp_gst__interfaces_mixertrack_get_label_offset ();
[DllImport ("gstreamersharpglue-0.10.dll") ]
extern static uint gst__interfacessharp_gst__interfaces_mixertrack_get_flags_offset ();
static uint flags_offset = gst__interfacessharp_gst__interfaces_mixertrack_get_flags_offset ();
[DllImport ("gstreamersharpglue-0.10.dll") ]
extern static uint gst__interfacessharp_gst__interfaces_mixertrack_get_num_channels_offset ();
static uint num_channels_offset = gst__interfacessharp_gst__interfaces_mixertrack_get_num_channels_offset ();
[DllImport ("gstreamersharpglue-0.10.dll") ]
extern static uint gst__interfacessharp_gst__interfaces_mixertrack_get_min_volume_offset ();
static uint min_volume_offset = gst__interfacessharp_gst__interfaces_mixertrack_get_min_volume_offset ();
[DllImport ("gstreamersharpglue-0.10.dll") ]
extern static uint gst__interfacessharp_gst__interfaces_mixertrack_get_max_volume_offset ();
static uint max_volume_offset = gst__interfacessharp_gst__interfaces_mixertrack_get_max_volume_offset ();

public MixerTrack (uint index, string untranslated_label, string label, MixerTrackFlags flags, int num_channels, int min_volume, int max_volume) : base (IntPtr.Zero) {
  CreateNativeObject (new string [] {"index", "untranslated-label"}, new Gst.GLib.Value [] { new Gst.GLib.Value (index), new Gst.GLib.Value (untranslated_label) });

  unsafe {
    IntPtr* raw_ptr = (IntPtr*) ( ( (byte*) Handle) + label_offset);
    *raw_ptr = Gst.GLib.Marshaller.StringToPtrGStrdup (label);
  }

  unsafe {
    int* raw_ptr = (int*) ( ( (byte*) Handle) + flags_offset);
    *raw_ptr = (int) flags;
  }

  unsafe {
    int* raw_ptr = (int*) ( ( (byte*) Handle) + num_channels_offset);
    *raw_ptr = num_channels;
  }

  unsafe {
    int* raw_ptr = (int*) ( ( (byte*) Handle) + min_volume_offset);
    *raw_ptr = min_volume;
  }

  unsafe {
    int* raw_ptr = (int*) ( ( (byte*) Handle) + max_volume_offset);
    *raw_ptr = max_volume;
  }

}

#endregion
	}
}