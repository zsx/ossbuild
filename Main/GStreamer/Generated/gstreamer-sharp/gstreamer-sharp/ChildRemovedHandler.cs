// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;

	public delegate void ChildRemovedHandler(object o, ChildRemovedArgs args);

	public class ChildRemovedArgs : Gst.GLib.SignalArgs {
		public Gst.Object Child{
			get {
				return (Gst.Object) Args [0];
			}
		}

	}
}