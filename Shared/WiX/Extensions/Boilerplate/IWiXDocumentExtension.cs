using System;
using System.Xml;

namespace OSSBuild.WiX {
	///<summary>
	///	Marker interface for classes wishing to process the WiX XML document.
	///</summary>
	public interface IWiXDocumentExtension {
		#region Properties
		string NamespaceURI { get; }
		#endregion

		XmlNode PreprocessDocument(XmlDocument document, XmlNode parentNode, XmlNode node, XmlAttributeCollection attributes);
	}
}
