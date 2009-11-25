// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace Gst {

	using System;

#region Autogenerated code
	public partial interface ChildProxy : Gst.GLib.IWrapper {

		event Gst.ChildAddedHandler ChildAdded;
		event Gst.ChildRemovedHandler ChildRemoved;
		void EmitChildAdded(Gst.Object child);
		Gst.Object GetChildByName(string name);
		Gst.Object GetChildByIndex(uint index);
		uint ChildrenCount { 
			get;
		}
		void EmitChildRemoved(Gst.Object child);
	}

	[Gst.GLib.GInterface (typeof (ChildProxyAdapter))]
	public partial interface ChildProxyImplementor : Gst.GLib.IWrapper {

		Gst.Object GetChildByIndex (uint index);
		uint ChildrenCount { get; }
	}
#endregion
}
