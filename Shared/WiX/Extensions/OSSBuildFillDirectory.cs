using System;
using System.Xml;

namespace OSSBuild.WiX {
	internal class OSSBuildFillDirectory : IWiXDocumentExtension {
		#region IWiXDocumentExtension Members
		///<summary>
		///	Processes the document and attempts to read a provided directory and 
		///	generate WiX directory/component/file tags.
		///</summary>
		public void PreprocessDocument(XmlDocument document, XmlNode node, XmlAttributeCollection attributes) {
			//Does nothing at this point
			//TODO: Fill this in
		}
		#endregion
	}
}
