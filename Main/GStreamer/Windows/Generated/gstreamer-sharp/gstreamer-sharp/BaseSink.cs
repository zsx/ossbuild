// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst.Base {

	using System;
	using System.Collections;
	using System.Runtime.InteropServices;

#region Autogenerated code
	public partial class BaseSink : Gst.Element {

		public BaseSink(IntPtr raw) : base(raw) {}

		protected BaseSink() : base(IntPtr.Zero)
		{
			CreateNativeObject (new string [0], new Gst.GLib.Value [0]);
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern uint gst_base_sink_get_blocksize(IntPtr raw);

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_base_sink_set_blocksize(IntPtr raw, uint blocksize);

		[Gst.GLib.Property ("blocksize")]
		public uint Blocksize {
			get  {
				uint raw_ret = gst_base_sink_get_blocksize(Handle);
				uint ret = raw_ret;
				return ret;
			}
			set  {
				gst_base_sink_set_blocksize(Handle, value);
			}
		}

		[Gst.GLib.Property ("qos")]
		public bool Qos {
			get {
				Gst.GLib.Value val = GetProperty ("qos");
				bool ret = (bool) val;
				val.Dispose ();
				return ret;
			}
			set {
				Gst.GLib.Value val = new Gst.GLib.Value(value);
				SetProperty("qos", val);
				val.Dispose ();
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern bool gst_base_sink_get_sync(IntPtr raw);

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_base_sink_set_sync(IntPtr raw, bool sync);

		[Gst.GLib.Property ("sync")]
		public bool Sync {
			get  {
				bool raw_ret = gst_base_sink_get_sync(Handle);
				bool ret = raw_ret;
				return ret;
			}
			set  {
				gst_base_sink_set_sync(Handle, value);
			}
		}

		[Gst.GLib.Property ("async")]
		public bool Async {
			get {
				Gst.GLib.Value val = GetProperty ("async");
				bool ret = (bool) val;
				val.Dispose ();
				return ret;
			}
			set {
				Gst.GLib.Value val = new Gst.GLib.Value(value);
				SetProperty("async", val);
				val.Dispose ();
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern ulong gst_base_sink_get_render_delay(IntPtr raw);

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_base_sink_set_render_delay(IntPtr raw, ulong delay);

		[Gst.GLib.Property ("render-delay")]
		public ulong RenderDelay {
			get  {
				ulong raw_ret = gst_base_sink_get_render_delay(Handle);
				ulong ret = raw_ret;
				return ret;
			}
			set  {
				gst_base_sink_set_render_delay(Handle, value);
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern long gst_base_sink_get_max_lateness(IntPtr raw);

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_base_sink_set_max_lateness(IntPtr raw, long max_lateness);

		[Gst.GLib.Property ("max-lateness")]
		public long MaxLateness {
			get  {
				long raw_ret = gst_base_sink_get_max_lateness(Handle);
				long ret = raw_ret;
				return ret;
			}
			set  {
				gst_base_sink_set_max_lateness(Handle, value);
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern long gst_base_sink_get_ts_offset(IntPtr raw);

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void gst_base_sink_set_ts_offset(IntPtr raw, long offset);

		[Gst.GLib.Property ("ts-offset")]
		public long TsOffset {
			get  {
				long raw_ret = gst_base_sink_get_ts_offset(Handle);
				long ret = raw_ret;
				return ret;
			}
			set  {
				gst_base_sink_set_ts_offset(Handle, value);
			}
		}

		[Gst.GLib.Property ("preroll-queue-len")]
		public uint PrerollQueueLen {
			get {
				Gst.GLib.Value val = GetProperty ("preroll-queue-len");
				uint ret = (uint) val;
				val.Dispose ();
				return ret;
			}
			set {
				Gst.GLib.Value val = new Gst.GLib.Value(value);
				SetProperty("preroll-queue-len", val);
				val.Dispose ();
			}
		}

		static GetCapsNativeDelegate GetCaps_cb_delegate;
		static GetCapsNativeDelegate GetCapsVMCallback {
			get {
				if (GetCaps_cb_delegate == null)
					GetCaps_cb_delegate = new GetCapsNativeDelegate (GetCaps_cb);
				return GetCaps_cb_delegate;
			}
		}

		static void OverrideGetCaps (Gst.GLib.GType gtype)
		{
			OverrideGetCaps (gtype, GetCapsVMCallback);
		}

		static void OverrideGetCaps (Gst.GLib.GType gtype, GetCapsNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.GetCaps = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate IntPtr GetCapsNativeDelegate (IntPtr inst);

		static IntPtr GetCaps_cb (IntPtr inst)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				Gst.Caps __result = __obj.OnGetCaps ();
				return __result == null ? IntPtr.Zero : __result.OwnedCopy;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideGetCaps")]
		protected virtual Gst.Caps OnGetCaps ()
		{
			return InternalGetCaps ();
		}

		private Gst.Caps InternalGetCaps ()
		{
			GetCapsNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).GetCaps;
			if (unmanaged == null) return null;

			IntPtr __result = unmanaged (this.Handle);
			return __result == IntPtr.Zero ? null : (Gst.Caps) Gst.GLib.Opaque.GetOpaque (__result, typeof (Gst.Caps), true);
		}

		static SetCapsNativeDelegate SetCaps_cb_delegate;
		static SetCapsNativeDelegate SetCapsVMCallback {
			get {
				if (SetCaps_cb_delegate == null)
					SetCaps_cb_delegate = new SetCapsNativeDelegate (SetCaps_cb);
				return SetCaps_cb_delegate;
			}
		}

		static void OverrideSetCaps (Gst.GLib.GType gtype)
		{
			OverrideSetCaps (gtype, SetCapsVMCallback);
		}

		static void OverrideSetCaps (Gst.GLib.GType gtype, SetCapsNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.SetCaps = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool SetCapsNativeDelegate (IntPtr inst, IntPtr caps);

		static bool SetCaps_cb (IntPtr inst, IntPtr caps)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnSetCaps (caps == IntPtr.Zero ? null : (Gst.Caps) Gst.GLib.Opaque.GetOpaque (caps, typeof (Gst.Caps), false));
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideSetCaps")]
		protected virtual bool OnSetCaps (Gst.Caps caps)
		{
			return InternalSetCaps (caps);
		}

		private bool InternalSetCaps (Gst.Caps caps)
		{
			SetCapsNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).SetCaps;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle, caps == null ? IntPtr.Zero : caps.Handle);
			return __result;
		}

		static BufferAllocNativeDelegate BufferAlloc_cb_delegate;
		static BufferAllocNativeDelegate BufferAllocVMCallback {
			get {
				if (BufferAlloc_cb_delegate == null)
					BufferAlloc_cb_delegate = new BufferAllocNativeDelegate (BufferAlloc_cb);
				return BufferAlloc_cb_delegate;
			}
		}

		static void OverrideBufferAlloc (Gst.GLib.GType gtype)
		{
			OverrideBufferAlloc (gtype, BufferAllocVMCallback);
		}

		static void OverrideBufferAlloc (Gst.GLib.GType gtype, BufferAllocNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.BufferAlloc = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate int BufferAllocNativeDelegate (IntPtr inst, ulong offset, uint size, IntPtr caps, out IntPtr buf);

		static int BufferAlloc_cb (IntPtr inst, ulong offset, uint size, IntPtr caps, out IntPtr buf)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				Gst.Buffer mybuf;
				Gst.FlowReturn __result = __obj.OnBufferAlloc (offset, size, caps == IntPtr.Zero ? null : (Gst.Caps) Gst.GLib.Opaque.GetOpaque (caps, typeof (Gst.Caps), false), out mybuf);
				buf = mybuf == null ? IntPtr.Zero : mybuf.Handle;
				return (int) __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideBufferAlloc")]
		protected virtual Gst.FlowReturn OnBufferAlloc (ulong offset, uint size, Gst.Caps caps, out Gst.Buffer buf)
		{
			return InternalBufferAlloc (offset, size, caps, out buf);
		}

		private Gst.FlowReturn InternalBufferAlloc (ulong offset, uint size, Gst.Caps caps, out Gst.Buffer buf)
		{
			BufferAllocNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).BufferAlloc;
			if (unmanaged == null) throw new InvalidOperationException ("No base method to invoke");

			IntPtr native_buf;
			int __result = unmanaged (this.Handle, offset, size, caps == null ? IntPtr.Zero : caps.Handle, out native_buf);
			buf = Gst.MiniObject.GetObject(native_buf, true) as Gst.Buffer;
			return (Gst.FlowReturn) __result;
		}

		static GetTimesNativeDelegate GetTimes_cb_delegate;
		static GetTimesNativeDelegate GetTimesVMCallback {
			get {
				if (GetTimes_cb_delegate == null)
					GetTimes_cb_delegate = new GetTimesNativeDelegate (GetTimes_cb);
				return GetTimes_cb_delegate;
			}
		}

		static void OverrideGetTimes (Gst.GLib.GType gtype)
		{
			OverrideGetTimes (gtype, GetTimesVMCallback);
		}

		static void OverrideGetTimes (Gst.GLib.GType gtype, GetTimesNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.GetTimes = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate void GetTimesNativeDelegate (IntPtr inst, IntPtr buffer, out ulong start, out ulong end);

		static void GetTimes_cb (IntPtr inst, IntPtr buffer, out ulong start, out ulong end)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				__obj.OnGetTimes (Gst.MiniObject.GetObject(buffer) as Gst.Buffer, out start, out end);
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideGetTimes")]
		protected virtual void OnGetTimes (Gst.Buffer buffer, out ulong start, out ulong end)
		{
			InternalGetTimes (buffer, out start, out end);
		}

		private void InternalGetTimes (Gst.Buffer buffer, out ulong start, out ulong end)
		{
			GetTimesNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).GetTimes;
			if (unmanaged == null) throw new InvalidOperationException ("No base method to invoke");

			unmanaged (this.Handle, buffer == null ? IntPtr.Zero : buffer.Handle, out start, out end);
		}

		static StartNativeDelegate Start_cb_delegate;
		static StartNativeDelegate StartVMCallback {
			get {
				if (Start_cb_delegate == null)
					Start_cb_delegate = new StartNativeDelegate (Start_cb);
				return Start_cb_delegate;
			}
		}

		static void OverrideStart (Gst.GLib.GType gtype)
		{
			OverrideStart (gtype, StartVMCallback);
		}

		static void OverrideStart (Gst.GLib.GType gtype, StartNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Start = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool StartNativeDelegate (IntPtr inst);

		static bool Start_cb (IntPtr inst)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnStart ();
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideStart")]
		protected virtual bool OnStart ()
		{
			return InternalStart ();
		}

		private bool InternalStart ()
		{
			StartNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Start;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle);
			return __result;
		}

		static StopNativeDelegate Stop_cb_delegate;
		static StopNativeDelegate StopVMCallback {
			get {
				if (Stop_cb_delegate == null)
					Stop_cb_delegate = new StopNativeDelegate (Stop_cb);
				return Stop_cb_delegate;
			}
		}

		static void OverrideStop (Gst.GLib.GType gtype)
		{
			OverrideStop (gtype, StopVMCallback);
		}

		static void OverrideStop (Gst.GLib.GType gtype, StopNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Stop = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool StopNativeDelegate (IntPtr inst);

		static bool Stop_cb (IntPtr inst)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnStop ();
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideStop")]
		protected virtual bool OnStop ()
		{
			return InternalStop ();
		}

		private bool InternalStop ()
		{
			StopNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Stop;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle);
			return __result;
		}

		static UnlockNativeDelegate Unlock_cb_delegate;
		static UnlockNativeDelegate UnlockVMCallback {
			get {
				if (Unlock_cb_delegate == null)
					Unlock_cb_delegate = new UnlockNativeDelegate (Unlock_cb);
				return Unlock_cb_delegate;
			}
		}

		static void OverrideUnlock (Gst.GLib.GType gtype)
		{
			OverrideUnlock (gtype, UnlockVMCallback);
		}

		static void OverrideUnlock (Gst.GLib.GType gtype, UnlockNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Unlock = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool UnlockNativeDelegate (IntPtr inst);

		static bool Unlock_cb (IntPtr inst)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnUnlock ();
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideUnlock")]
		protected virtual bool OnUnlock ()
		{
			return InternalUnlock ();
		}

		private bool InternalUnlock ()
		{
			UnlockNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Unlock;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle);
			return __result;
		}

		static EventNativeDelegate Event_cb_delegate;
		static EventNativeDelegate EventVMCallback {
			get {
				if (Event_cb_delegate == null)
					Event_cb_delegate = new EventNativeDelegate (Event_cb);
				return Event_cb_delegate;
			}
		}

		static void OverrideEvent (Gst.GLib.GType gtype)
		{
			OverrideEvent (gtype, EventVMCallback);
		}

		static void OverrideEvent (Gst.GLib.GType gtype, EventNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Event = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool EventNativeDelegate (IntPtr inst, IntPtr evnt);

		static bool Event_cb (IntPtr inst, IntPtr evnt)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnEvent (Gst.MiniObject.GetObject(evnt) as Gst.Event);
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideEvent")]
		protected virtual bool OnEvent (Gst.Event evnt)
		{
			return InternalEvent (evnt);
		}

		private bool InternalEvent (Gst.Event evnt)
		{
			EventNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Event;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle, evnt == null ? IntPtr.Zero : evnt.Handle);
			return __result;
		}

		static PrerollNativeDelegate Preroll_cb_delegate;
		static PrerollNativeDelegate PrerollVMCallback {
			get {
				if (Preroll_cb_delegate == null)
					Preroll_cb_delegate = new PrerollNativeDelegate (Preroll_cb);
				return Preroll_cb_delegate;
			}
		}

		static void OverridePreroll (Gst.GLib.GType gtype)
		{
			OverridePreroll (gtype, PrerollVMCallback);
		}

		static void OverridePreroll (Gst.GLib.GType gtype, PrerollNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Preroll = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate int PrerollNativeDelegate (IntPtr inst, IntPtr buffer);

		static int Preroll_cb (IntPtr inst, IntPtr buffer)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				Gst.FlowReturn __result = __obj.OnPreroll (Gst.MiniObject.GetObject(buffer) as Gst.Buffer);
				return (int) __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverridePreroll")]
		protected virtual Gst.FlowReturn OnPreroll (Gst.Buffer buffer)
		{
			return InternalPreroll (buffer);
		}

		private Gst.FlowReturn InternalPreroll (Gst.Buffer buffer)
		{
			PrerollNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Preroll;
			if (unmanaged == null) return (Gst.FlowReturn) 0;

			int __result = unmanaged (this.Handle, buffer == null ? IntPtr.Zero : buffer.Handle);
			return (Gst.FlowReturn) __result;
		}

		static RenderNativeDelegate Render_cb_delegate;
		static RenderNativeDelegate RenderVMCallback {
			get {
				if (Render_cb_delegate == null)
					Render_cb_delegate = new RenderNativeDelegate (Render_cb);
				return Render_cb_delegate;
			}
		}

		static void OverrideRender (Gst.GLib.GType gtype)
		{
			OverrideRender (gtype, RenderVMCallback);
		}

		static void OverrideRender (Gst.GLib.GType gtype, RenderNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Render = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate int RenderNativeDelegate (IntPtr inst, IntPtr buffer);

		static int Render_cb (IntPtr inst, IntPtr buffer)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				Gst.FlowReturn __result = __obj.OnRender (Gst.MiniObject.GetObject(buffer) as Gst.Buffer);
				return (int) __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideRender")]
		protected virtual Gst.FlowReturn OnRender (Gst.Buffer buffer)
		{
			return InternalRender (buffer);
		}

		private Gst.FlowReturn InternalRender (Gst.Buffer buffer)
		{
			RenderNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Render;
			if (unmanaged == null) return (Gst.FlowReturn) 0;

			int __result = unmanaged (this.Handle, buffer == null ? IntPtr.Zero : buffer.Handle);
			return (Gst.FlowReturn) __result;
		}

		static AsyncPlayNativeDelegate AsyncPlay_cb_delegate;
		static AsyncPlayNativeDelegate AsyncPlayVMCallback {
			get {
				if (AsyncPlay_cb_delegate == null)
					AsyncPlay_cb_delegate = new AsyncPlayNativeDelegate (AsyncPlay_cb);
				return AsyncPlay_cb_delegate;
			}
		}

		static void OverrideAsyncPlay (Gst.GLib.GType gtype)
		{
			OverrideAsyncPlay (gtype, AsyncPlayVMCallback);
		}

		static void OverrideAsyncPlay (Gst.GLib.GType gtype, AsyncPlayNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.AsyncPlay = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate int AsyncPlayNativeDelegate (IntPtr inst);

		static int AsyncPlay_cb (IntPtr inst)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				Gst.StateChangeReturn __result = __obj.OnAsyncPlay ();
				return (int) __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideAsyncPlay")]
		protected virtual Gst.StateChangeReturn OnAsyncPlay ()
		{
			return InternalAsyncPlay ();
		}

		private Gst.StateChangeReturn InternalAsyncPlay ()
		{
			AsyncPlayNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).AsyncPlay;
			if (unmanaged == null) return (Gst.StateChangeReturn) 0;

			int __result = unmanaged (this.Handle);
			return (Gst.StateChangeReturn) __result;
		}

		static ActivatePullNativeDelegate ActivatePull_cb_delegate;
		static ActivatePullNativeDelegate ActivatePullVMCallback {
			get {
				if (ActivatePull_cb_delegate == null)
					ActivatePull_cb_delegate = new ActivatePullNativeDelegate (ActivatePull_cb);
				return ActivatePull_cb_delegate;
			}
		}

		static void OverrideActivatePull (Gst.GLib.GType gtype)
		{
			OverrideActivatePull (gtype, ActivatePullVMCallback);
		}

		static void OverrideActivatePull (Gst.GLib.GType gtype, ActivatePullNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.ActivatePull = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool ActivatePullNativeDelegate (IntPtr inst, bool active);

		static bool ActivatePull_cb (IntPtr inst, bool active)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnActivatePull (active);
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideActivatePull")]
		protected virtual bool OnActivatePull (bool active)
		{
			return InternalActivatePull (active);
		}

		private bool InternalActivatePull (bool active)
		{
			ActivatePullNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).ActivatePull;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle, active);
			return __result;
		}

		static FixateNativeDelegate Fixate_cb_delegate;
		static FixateNativeDelegate FixateVMCallback {
			get {
				if (Fixate_cb_delegate == null)
					Fixate_cb_delegate = new FixateNativeDelegate (Fixate_cb);
				return Fixate_cb_delegate;
			}
		}

		static void OverrideFixate (Gst.GLib.GType gtype)
		{
			OverrideFixate (gtype, FixateVMCallback);
		}

		static void OverrideFixate (Gst.GLib.GType gtype, FixateNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.Fixate = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate void FixateNativeDelegate (IntPtr inst, IntPtr caps);

		static void Fixate_cb (IntPtr inst, IntPtr caps)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				__obj.OnFixate (caps == IntPtr.Zero ? null : (Gst.Caps) Gst.GLib.Opaque.GetOpaque (caps, typeof (Gst.Caps), false));
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, false);
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideFixate")]
		protected virtual void OnFixate (Gst.Caps caps)
		{
			InternalFixate (caps);
		}

		private void InternalFixate (Gst.Caps caps)
		{
			FixateNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).Fixate;
			if (unmanaged == null) return;

			unmanaged (this.Handle, caps == null ? IntPtr.Zero : caps.Handle);
		}

		static UnlockStopNativeDelegate UnlockStop_cb_delegate;
		static UnlockStopNativeDelegate UnlockStopVMCallback {
			get {
				if (UnlockStop_cb_delegate == null)
					UnlockStop_cb_delegate = new UnlockStopNativeDelegate (UnlockStop_cb);
				return UnlockStop_cb_delegate;
			}
		}

		static void OverrideUnlockStop (Gst.GLib.GType gtype)
		{
			OverrideUnlockStop (gtype, UnlockStopVMCallback);
		}

		static void OverrideUnlockStop (Gst.GLib.GType gtype, UnlockStopNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.UnlockStop = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate bool UnlockStopNativeDelegate (IntPtr inst);

		static bool UnlockStop_cb (IntPtr inst)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				bool __result = __obj.OnUnlockStop ();
				return __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideUnlockStop")]
		protected virtual bool OnUnlockStop ()
		{
			return InternalUnlockStop ();
		}

		private bool InternalUnlockStop ()
		{
			UnlockStopNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).UnlockStop;
			if (unmanaged == null) return false;

			bool __result = unmanaged (this.Handle);
			return __result;
		}

		static RenderListNativeDelegate RenderList_cb_delegate;
		static RenderListNativeDelegate RenderListVMCallback {
			get {
				if (RenderList_cb_delegate == null)
					RenderList_cb_delegate = new RenderListNativeDelegate (RenderList_cb);
				return RenderList_cb_delegate;
			}
		}

		static void OverrideRenderList (Gst.GLib.GType gtype)
		{
			OverrideRenderList (gtype, RenderListVMCallback);
		}

		static void OverrideRenderList (Gst.GLib.GType gtype, RenderListNativeDelegate callback)
		{
			GstBaseSinkClass class_iface = GetClassStruct (gtype, false);
			class_iface.RenderList = callback;
			OverrideClassStruct (gtype, class_iface);
		}

		[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
		delegate int RenderListNativeDelegate (IntPtr inst, IntPtr buffer_list);

		static int RenderList_cb (IntPtr inst, IntPtr buffer_list)
		{
			try {
				BaseSink __obj = Gst.GLib.Object.GetObject (inst, false) as BaseSink;
				Gst.FlowReturn __result = __obj.OnRenderList (Gst.MiniObject.GetObject(buffer_list) as Gst.BufferList);
				return (int) __result;
			} catch (Exception e) {
				Gst.GLib.ExceptionManager.RaiseUnhandledException (e, true);
				// NOTREACHED: above call does not return.
				throw e;
			}
		}

		[Gst.GLib.DefaultSignalHandler(Type=typeof(Gst.Base.BaseSink), ConnectionMethod="OverrideRenderList")]
		protected virtual Gst.FlowReturn OnRenderList (Gst.BufferList buffer_list)
		{
			return InternalRenderList (buffer_list);
		}

		private Gst.FlowReturn InternalRenderList (Gst.BufferList buffer_list)
		{
			RenderListNativeDelegate unmanaged = GetClassStruct (this.LookupGType ().GetThresholdType (), true).RenderList;
			if (unmanaged == null) return (Gst.FlowReturn) 0;

			int __result = unmanaged (this.Handle, buffer_list == null ? IntPtr.Zero : buffer_list.Handle);
			return (Gst.FlowReturn) __result;
		}

		[StructLayout (LayoutKind.Sequential)]
		struct GstBaseSinkClass {
			public GetCapsNativeDelegate GetCaps;
			public SetCapsNativeDelegate SetCaps;
			public BufferAllocNativeDelegate BufferAlloc;
			public GetTimesNativeDelegate GetTimes;
			public StartNativeDelegate Start;
			public StopNativeDelegate Stop;
			public UnlockNativeDelegate Unlock;
			public EventNativeDelegate Event;
			public PrerollNativeDelegate Preroll;
			public RenderNativeDelegate Render;
			public AsyncPlayNativeDelegate AsyncPlay;
			public ActivatePullNativeDelegate ActivatePull;
			public FixateNativeDelegate Fixate;
			public UnlockStopNativeDelegate UnlockStop;
			public RenderListNativeDelegate RenderList;
			[MarshalAs (UnmanagedType.ByValArray, SizeConst=15)]
			public IntPtr[] GstReserved;
		}

		static uint class_offset = ((Gst.GLib.GType) typeof (Gst.Element)).GetClassSize ();
		static Hashtable class_structs;

		static GstBaseSinkClass GetClassStruct (Gst.GLib.GType gtype, bool use_cache)
		{
			if (class_structs == null)
				class_structs = new Hashtable ();

			if (use_cache && class_structs.Contains (gtype))
				return (GstBaseSinkClass) class_structs [gtype];
			else {
				IntPtr class_ptr = new IntPtr (gtype.GetClassPtr ().ToInt64 () + class_offset);
				GstBaseSinkClass class_struct = (GstBaseSinkClass) Marshal.PtrToStructure (class_ptr, typeof (GstBaseSinkClass));
				if (use_cache)
					class_structs.Add (gtype, class_struct);
				return class_struct;
			}
		}

		static void OverrideClassStruct (Gst.GLib.GType gtype, GstBaseSinkClass class_struct)
		{
			IntPtr class_ptr = new IntPtr (gtype.GetClassPtr ().ToInt64 () + class_offset);
			Marshal.StructureToPtr (class_struct, class_ptr, false);
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_base_sink_get_last_buffer(IntPtr raw);

		public Gst.Buffer LastBuffer { 
			get {
				IntPtr raw_ret = gst_base_sink_get_last_buffer(Handle);
				Gst.Buffer ret = Gst.MiniObject.GetObject(raw_ret) as Gst.Buffer;
				return ret;
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern bool gst_base_sink_query_latency(IntPtr raw, out bool live, out bool upstream_live, out ulong min_latency, out ulong max_latency);

		public bool QueryLatency(out bool live, out bool upstream_live, out ulong min_latency, out ulong max_latency) {
			bool raw_ret = gst_base_sink_query_latency(Handle, out live, out upstream_live, out min_latency, out max_latency);
			bool ret = raw_ret;
			return ret;
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern IntPtr gst_base_sink_get_type();

		public static new Gst.GLib.GType GType { 
			get {
				IntPtr raw_ret = gst_base_sink_get_type();
				Gst.GLib.GType ret = new Gst.GLib.GType(raw_ret);
				return ret;
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_base_sink_wait_clock(IntPtr raw, ulong time, out long jitter);

		public Gst.ClockReturn WaitClock(ulong time, out long jitter) {
			int raw_ret = gst_base_sink_wait_clock(Handle, time, out jitter);
			Gst.ClockReturn ret = (Gst.ClockReturn) raw_ret;
			return ret;
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern ulong gst_base_sink_get_latency(IntPtr raw);

		public ulong Latency { 
			get {
				ulong raw_ret = gst_base_sink_get_latency(Handle);
				ulong ret = raw_ret;
				return ret;
			}
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_base_sink_wait_preroll(IntPtr raw);

		public Gst.FlowReturn WaitPreroll() {
			int raw_ret = gst_base_sink_wait_preroll(Handle);
			Gst.FlowReturn ret = (Gst.FlowReturn) raw_ret;
			return ret;
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_base_sink_do_preroll(IntPtr raw, IntPtr obj);

		public Gst.FlowReturn DoPreroll(Gst.MiniObject obj) {
			int raw_ret = gst_base_sink_do_preroll(Handle, obj == null ? IntPtr.Zero : obj.Handle);
			Gst.FlowReturn ret = (Gst.FlowReturn) raw_ret;
			return ret;
		}

		[DllImport("libgstbase-0.10.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int gst_base_sink_wait_eos(IntPtr raw, ulong time, out long jitter);

		public Gst.FlowReturn WaitEos(ulong time, out long jitter) {
			int raw_ret = gst_base_sink_wait_eos(Handle, time, out jitter);
			Gst.FlowReturn ret = (Gst.FlowReturn) raw_ret;
			return ret;
		}


		static BaseSink ()
		{
			GtkSharp.GstreamerSharp.ObjectManager.Initialize ();
		}
#endregion
	}
}
