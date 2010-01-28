using System;
using System.Xml;

namespace OSSBuild.WiX {
	///<summary>
	///	Marker interface for classes wishing to process the WiX XML document.
	///</summary>
	public interface IWiXDocumentExtension {
		void PreprocessDocument(XmlDocument document, XmlNode node, XmlAttributeCollection attributes);
	}
}
