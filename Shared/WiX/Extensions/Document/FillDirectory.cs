using System;
using System.IO;
using System.Xml;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace OSSBuild.WiX {
	internal class FillDirectory : IWiXDocumentExtension {
		#region Constants
		private const string
			  ATTR_DIRECTORY					= "Directory"
			, ATTR_DIRECTORY_REF				= "DirectoryRef"
			, ATTR_INCLUDE_HIDDEN_DIRECTORIES	= "IncludeHiddenDirectories"
			, ATTR_INCLUDE_HIDDEN_FILES			= "IncludeHiddenFiles"
			, ATTR_COMPONENT_GROUP				= "ComponentGroup"
		;
		#endregion

		#region Variables
		private OSSBuild ossbuild;
		private List<string> collectedFiles = new List<string>(10);
		private List<string> collectedDirectories = new List<string>(10);
		#endregion

		#region Initialization
		public FillDirectory() {
			init();
		}

		private void init() {
			ossbuild = new OSSBuild();
		}
		#endregion

		#region Properties
		public string NamespaceURI {
			get { return Namespace.OSSBuild; }
		}
		#endregion

		#region Helper Methods
		private bool isTopLevelDefaultWiXNode(XmlNode node) {
			string localName = node.LocalName;
			return (
				(
					   "Product".Equals(localName, StringComparison.OrdinalIgnoreCase)
					|| "Fragment".Equals(localName, StringComparison.OrdinalIgnoreCase)
					|| "Bundle".Equals(localName, StringComparison.OrdinalIgnoreCase)
					|| "Module".Equals(localName, StringComparison.OrdinalIgnoreCase)
					|| "Patch".Equals(localName, StringComparison.OrdinalIgnoreCase)
					|| "PatchCreation".Equals(localName, StringComparison.OrdinalIgnoreCase)
				) && (
					Namespace.Default.Equals(node.NamespaceURI, StringComparison.OrdinalIgnoreCase)
				)
			);
		}

		private XmlNode findTopLevelDefaultWiXNode(XmlNode node) {
			//Look for <Product />, <Fragment />, <Bundle />, <Module />, <Patch />, <PatchCreation />
			do {
				if (isTopLevelDefaultWiXNode(node))
					return node;
			} while ((node = node.ParentNode) != null && node.NodeType == XmlNodeType.Element);
			return null;
		}

		private XmlNode createDefaultWiXNode(XmlDocument document, string name) {
			return document.CreateNode(XmlNodeType.Element, name, Namespace.Default);
		}

		private XmlNode createDefaultWiXNode(XmlNode parent, string name) {
			return parent.OwnerDocument.CreateNode(XmlNodeType.Element, name, Namespace.Default);
		}

		private XmlNode appendDefaultWiXNode(XmlNode parent, string name) {
			return parent.AppendChild(createDefaultWiXNode(parent, name));
		}

		private XmlAttribute createDefaultWiXAttribute(XmlNode node, string name, string value) {
			XmlAttribute a;
			if (Namespace.Default.Equals(node.NamespaceURI, StringComparison.OrdinalIgnoreCase))
				a = node.OwnerDocument.CreateAttribute(name);
			else
				a = node.OwnerDocument.CreateAttribute(null, name, Namespace.Default);
			a.Value = value;
			return a;
		}

		private XmlAttribute appendDefaultWiXAttribute(XmlNode node, string name, string value) {
			return node.Attributes.Append(createDefaultWiXAttribute(node, name, value));
		}

		private string findDefaultWiXAttributeValue(XmlNode node, string attributeName) {
			XmlNode attrib = null;
			if (Namespace.Default.Equals(node.NamespaceURI, StringComparison.OrdinalIgnoreCase))
				attrib = node.Attributes.GetNamedItem(attributeName);
			else
				attrib = node.Attributes.GetNamedItem(attributeName, Namespace.Default);
			if (attrib == null)
				return null;
			return attrib.Value;
		}

		private string createValidID(string name) {
			return ossbuild.CreateValidID(new string[] { name });
		}

		private string trim(string name) {
			return ossbuild.Trim(new string[] { name });
		}

		private string stringToGUID(string name) {
			return ossbuild.StringToGUID(new string[] { name });
		}

		private string trimStringToGUID(string name) {
			string[] arr = new string[] { name };
			arr[0] = ossbuild.Trim(arr);
			return ossbuild.StringToGUID(arr);
		}

		private bool isHidden(FileInfo file) {
			return ((file.Attributes & FileAttributes.Hidden) == FileAttributes.Hidden);
		}

		private bool isHidden(DirectoryInfo dir) {
			return ((dir.Attributes & FileAttributes.Hidden) == FileAttributes.Hidden);
		}
		#endregion

		private void populateForDirectory(XmlNode componentGroupNode, XmlNode node, DirectoryInfo dir, Filter[] filters, bool includeHiddenDirectories, bool includeHiddenFiles, List<string> collectedFiles, List<string> collectedDirectories, ref int fileCount) {
			string dirID;
			XmlNode dirNode;
			XmlNode fileNode;
			XmlNode componentNode;
			string componentNodeID;

			
			foreach (FileInfo f in dir.GetFiles()) {
				if (!includeHiddenFiles && isHidden(f))
					continue;

				if (!Filter.Validate(filters, f.FullName))
					continue;

				//Disregard if we've already added this file
				if (collectedFiles.Contains(f.FullName))
					continue;

				collectedFiles.Add(f.FullName);
				++fileCount;
				

				componentNodeID = createValidID(f.FullName);
				appendDefaultWiXAttribute((componentNode = appendDefaultWiXNode(node, "Component")), "Id", componentNodeID);
				appendDefaultWiXAttribute(componentNode, "Guid", trimStringToGUID(f.FullName));
				//<Component /> requires a reference to the directory in which it's in
				//appendDefaultWiXAttribute(componentNode, "Directory", findDefaultWiXAttributeValue(node, "Id"));

				appendDefaultWiXAttribute((fileNode = appendDefaultWiXNode(componentNode, "File")), "Name", f.Name);
				appendDefaultWiXAttribute(fileNode, "Id", componentNodeID);
				appendDefaultWiXAttribute(fileNode, "Source", f.FullName);
				appendDefaultWiXAttribute(fileNode, "DefaultLanguage", "0");
				appendDefaultWiXAttribute(fileNode, "KeyPath", "yes");

				//Add to component group
				if (componentGroupNode != null)
					appendDefaultWiXAttribute(appendDefaultWiXNode(componentGroupNode, "ComponentRef"), "Id", componentNodeID);
			}

			foreach (DirectoryInfo d in dir.GetDirectories()) {
				if (!includeHiddenDirectories && isHidden(d))
					continue;

				collectedDirectories.Add(d.FullName);

				int localFileCount = 0;

				dirID = createValidID(d.FullName);
				appendDefaultWiXAttribute((dirNode = createDefaultWiXNode(node, "Directory")), "Id", dirID);
				appendDefaultWiXAttribute(dirNode, "Name", d.Name);

				populateForDirectory(componentGroupNode, dirNode, d, filters, includeHiddenDirectories, includeHiddenFiles, collectedFiles, collectedDirectories, ref localFileCount);
				fileCount += localFileCount;

				//Only add directories that contain something
				if (localFileCount > 0)
					node.AppendChild(dirNode);
			}
		}

		///<summary>
		///	Processes the document and attempts to read a provided directory and 
		///	generate WiX directory/component/file tags.
		///</summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public XmlNode PreprocessDocument(XmlDocument document, XmlNode parentNode, XmlNode node, XmlAttributeCollection attributes) {
			#region Check params
			string directory				= (attributes[ATTR_DIRECTORY]			!= null ? attributes[ATTR_DIRECTORY].Value			: string.Empty);
			string directoryRef				= (attributes[ATTR_DIRECTORY_REF]		!= null ? attributes[ATTR_DIRECTORY_REF].Value		: string.Empty);
			string componentGroup			= (attributes[ATTR_COMPONENT_GROUP]		!= null ? attributes[ATTR_COMPONENT_GROUP].Value	: string.Empty);
			bool includeHiddenDirectories	= bool.Parse((attributes[ATTR_INCLUDE_HIDDEN_DIRECTORIES]	!= null ? attributes[ATTR_INCLUDE_HIDDEN_DIRECTORIES].Value.ToLower()	: "false"));
			bool includeHiddenFiles			= bool.Parse((attributes[ATTR_INCLUDE_HIDDEN_FILES]			!= null ? attributes[ATTR_INCLUDE_HIDDEN_FILES].Value.ToLower()			: "false"));

			if (string.IsNullOrEmpty(directory))
				throw new ArgumentNullException("Missing directory attribute");
			if (!Directory.Exists(directory))
				throw new DirectoryNotFoundException("Missing directory: " + directory);

			DirectoryInfo top = new DirectoryInfo(directory);

			//Read inner XML and find filters
			Filter[] filters = Filter.Parse(node);

			collectedFiles.Clear();
			collectedDirectories.Clear();
			#endregion

			XmlNode topComponentGroupNode = null;
			XmlNode topNode = null;

			if (!includeHiddenDirectories && isHidden(top))
				return null;
			
			if (string.IsNullOrEmpty(directoryRef)) {
				if (parentNode != null && ("Directory".Equals(parentNode.LocalName, StringComparison.OrdinalIgnoreCase) || "DirectoryRef".Equals(parentNode.LocalName, StringComparison.OrdinalIgnoreCase))) {
					topNode = parentNode;
				} else {
					topNode = createDefaultWiXNode(document, "Directory");
					appendDefaultWiXAttribute(topNode, "Id", top.Name);
					appendDefaultWiXAttribute(topNode, "Name", top.Name);
				}
			} else {
				topNode = createDefaultWiXNode(document, "DirectoryRef");
				appendDefaultWiXAttribute(topNode, "Id", directoryRef);
			}

			if (!string.IsNullOrEmpty(componentGroup))
				appendDefaultWiXAttribute((topComponentGroupNode = createDefaultWiXNode(document, "ComponentGroup")), "Id", componentGroup);

			int fileCount = 0;
			populateForDirectory(topComponentGroupNode, topNode, top, filters, includeHiddenDirectories, includeHiddenFiles, collectedFiles, collectedDirectories, ref fileCount);

			//Find the top-level node where we can insert a <ComponentGroup />
			XmlNode topLevelNode = findTopLevelDefaultWiXNode(node);
			if (topLevelNode == null)
				throw new NullReferenceException("Unable to locate top-level WiX node to insert <ComponentGroup />");

			//Add component group no-matter-what
			if (topComponentGroupNode != null)
				topLevelNode.AppendChild(topComponentGroupNode);

			if (fileCount <= 0)
				return null;
			return topNode;
		}
	}
}
