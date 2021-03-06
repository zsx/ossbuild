// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace GtkSharp.GstreamerSharp {

	internal class ObjectManager {

		static bool initialized = false;
		// Call this method from the appropriate module init function.
		public static void Initialize ()
		{
			if (initialized)
				return;

			initialized = true;
			Gst.GLib.GType.Register (Gst.Controller.Controller.GType, typeof (Gst.Controller.Controller));
			Gst.GLib.GType.Register (Gst.Interfaces.ColorBalanceChannel.GType, typeof (Gst.Interfaces.ColorBalanceChannel));
			Gst.GLib.GType.Register (Gst.Cdda.CddaBaseSrc.GType, typeof (Gst.Cdda.CddaBaseSrc));
			Gst.GLib.GType.Register (Gst.Video.VideoSink.GType, typeof (Gst.Video.VideoSink));
			Gst.GLib.GType.Register (Gst.App.AppSink.GType, typeof (Gst.App.AppSink));
			Gst.GLib.GType.Register (Gst.Controller.ControlSource.GType, typeof (Gst.Controller.ControlSource));
			Gst.GLib.GType.Register (Gst.Interfaces.TunerChannel.GType, typeof (Gst.Interfaces.TunerChannel));
			Gst.GLib.GType.Register (Gst.Controller.LFOControlSource.GType, typeof (Gst.Controller.LFOControlSource));
			Gst.GLib.GType.Register (Gst.App.AppSrc.GType, typeof (Gst.App.AppSrc));
			Gst.GLib.GType.Register (Gst.Base.Adapter.GType, typeof (Gst.Base.Adapter));
			Gst.GLib.GType.Register (Gst.Base.BaseSink.GType, typeof (Gst.Base.BaseSink));
			Gst.GLib.GType.Register (Gst.Base.BaseSrc.GType, typeof (Gst.Base.BaseSrc));
			Gst.GLib.GType.Register (Gst.Interfaces.TunerNorm.GType, typeof (Gst.Interfaces.TunerNorm));
			Gst.GLib.GType.Register (Gst.Base.BaseTransform.GType, typeof (Gst.Base.BaseTransform));
			Gst.GLib.GType.Register (Gst.Controller.InterpolationControlSource.GType, typeof (Gst.Controller.InterpolationControlSource));
			Gst.GLib.GType.Register (Gst.Interfaces.MixerOptions.GType, typeof (Gst.Interfaces.MixerOptions));
			Gst.GLib.GType.Register (Gst.Base.PushSrc.GType, typeof (Gst.Base.PushSrc));
			Gst.GLib.GType.Register (Gst.Video.VideoFilter.GType, typeof (Gst.Video.VideoFilter));
			Gst.GLib.GType.Register (Gst.Interfaces.MixerTrack.GType, typeof (Gst.Interfaces.MixerTrack));
		}
	}
}
