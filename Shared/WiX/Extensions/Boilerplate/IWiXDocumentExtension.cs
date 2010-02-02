using System;
using System.Xml;

namespace OSSBuild.WiX {
	///<summary>
	///	Marker interface for classes wishing to process the WiX XML document.
	///</summary>
	public interface IWiXDocumentExtension {
		XmlNode PreprocessDocument(XmlDocument document, XmlNode parentNode, XmlNode node, XmlAttributeCollection attributes);
	}
}
