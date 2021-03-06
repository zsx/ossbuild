// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;
	using System.Collections;
	using System.Runtime.InteropServices;

#region Autogenerated code
	public partial class ClockEntry : Gst.GLib.Opaque {

		[DllImport ("gstreamersharpglue-0.10.dll")]
		extern static uint gstsharp_gst_clockentry_get_clock_offset ();

		static uint clock_offset = gstsharp_gst_clockentry_get_clock_offset ();
		public Gst.Clock Clock {
			get {
				unsafe {
					IntPtr* raw_ptr = (IntPtr*)(((byte*)Handle) + clock_offset);
					return Gst.GLib.Object.GetObject((*raw_ptr)) as Gst.Clock;
				}
			}
			set {
				unsafe {
					IntPtr* raw_ptr = (IntPtr*)(((byte*)Handle) + clock_offset);
					*raw_ptr = value == null ? IntPtr.Zero : value.Handle;
				}
			}
		}

		[DllImport ("gstreamersharpglue-0.10.dll")]
		extern static uint gstsharp_gst_clockentry_get_status_offset ();

		static uint status_offset = gstsharp_gst_clockentry_get_status_offset ();
		public Gst.ClockReturn Status {
			get {
				unsafe {
					int* raw_ptr = (int*)(((byte*)Handle) + status_offset);
					return (Gst.ClockReturn) (*raw_ptr);
				}
			}
			set {
				unsafe {
					int* raw_ptr = (int*)(((byte*)Handle) + status_offset);
					*raw_ptr = (int) value;
				}
			}
		}

		[DllImport ("gstreamersharpglue-0.10.dll")]
		extern static uint gstsharp_gst_clockentry_get_interval_offset ();

		static uint interval_offset = gstsharp_gst_clockentry_get_interval_offset ();
		public ulong Interval {
			get {
				unsafe {
					ulong* raw_ptr = (ulong*)(((byte*)Handle) + interval_offset);
					return (*raw_ptr);
				}
			}
			set {
				unsafe {
					ulong* raw_ptr = (ulong*)(((byte*)Handle) + interval_offset);
					*raw_ptr = value;
				}
			}
		}

		[DllImport ("gstreamersharpglue-0.10.dll")]
		extern static uint gstsharp_gst_clockentry_get_type_offset ();

		static uint type_offset = gstsharp_gst_clockentry_get_type_offset ();
		public Gst.ClockEntryType Type {
			get {
				unsafe {
					int* raw_ptr = (int*)(((byte*)Handle) + type_offset);
					return (Gst.ClockEntryType) (*raw_ptr);
				}
			}
			set {
				unsafe {
					int* raw_ptr = (int*)(((byte*)Handle) + type_offset);
					*raw_ptr = (int) value;
				}
			}
		}

		[DllImport ("gstreamersharpglue-0.10.dll")]
		extern static uint gstsharp_gst_clockentry_get_time_offset ();

		static uint time_offset = gstsharp_gst_clockentry_get_time_offset ();
		public ulong Time {
			get {
				unsafe {
					ulong* raw_ptr = (ulong*)(((byte*)Handle) + time_offset);
					return (*raw_ptr);
				}
			}
			set {
				unsafe {
					ulong* raw_ptr = (ulong*)(((byte*)Handle) + time_offset);
					*raw_ptr = value;
				}
			}
		}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_clock_id_unschedule(IntPtr raw);

		public void Unschedule() {
			gst_clock_id_unschedule(Handle);
		}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_clock_id_wait_async(IntPtr raw, GstSharp.ClockCallbackNative func, IntPtr user_data);

		public Gst.ClockReturn WaitAsync(Gst.ClockCallback func) {
			GstSharp.ClockCallbackWrapper func_wrapper = new GstSharp.ClockCallbackWrapper (func);
			int raw_ret = gst_clock_id_wait_async(Handle, func_wrapper.NativeDelegate, IntPtr.Zero);
			Gst.ClockReturn ret = (Gst.ClockReturn) raw_ret;
			return ret;
		}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_clock_id_wait(IntPtr raw, out long jitter);

		public Gst.ClockReturn Wait(out long jitter) {
			int raw_ret = gst_clock_id_wait(Handle, out jitter);
			Gst.ClockReturn ret = (Gst.ClockReturn) raw_ret;
			return ret;
		}

		public ClockEntry(IntPtr raw) : base(raw) {}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_clock_id_ref(IntPtr raw);

		protected override void Ref (IntPtr raw)
		{
			if (!Owned) {
				gst_clock_id_ref (raw);
				Owned = true;
			}
		}

		[DllImport("libgstreamer-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_clock_id_unref(IntPtr raw);

		protected override void Unref (IntPtr raw)
		{
			if (Owned) {
				gst_clock_id_unref (raw);
				Owned = false;
			}
		}

		class FinalizerInfo {
			IntPtr handle;

			public FinalizerInfo (IntPtr handle)
			{
				this.handle = handle;
			}

			public bool Handler ()
			{
				gst_clock_id_unref (handle);
				return false;
			}
		}

		~ClockEntry ()
		{
			if (!Owned)
				return;
			FinalizerInfo info = new FinalizerInfo (Handle);
			Gst.GLib.Timeout.Add (50, new Gst.GLib.TimeoutHandler (info.Handler));
		}

#endregion
#region Customized extensions
#line 1 "ClockEntry.custom"
[DllImport ("gstreamersharpglue-0.10.dll") ]
static extern GstSharp.ClockCallbackNative gstsharp_gst_clock_entry_get_func (IntPtr raw);
[DllImport ("gstreamersharpglue-0.10.dll") ]
static extern void gstsharp_gst_clock_entry_set_func (IntPtr raw, GstSharp.ClockCallbackNative func);

private GstSharp.ClockCallbackWrapper wrapper;

public Gst.ClockCallback Func {
  set {
    wrapper = new GstSharp.ClockCallbackWrapper (value);
    gstsharp_gst_clock_entry_set_func (Handle, wrapper.NativeDelegate);
  }

  get {
    return GstSharp.ClockCallbackWrapper.GetManagedDelegate (gstsharp_gst_clock_entry_get_func (Handle));
  }
}


#endregion
	}
}
