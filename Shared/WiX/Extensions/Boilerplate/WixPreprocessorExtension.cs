using System;
using System.IO;
using System.Xml;
using System.Text;
using System.Reflection;
using System.Diagnostics;
using System.Collections.Generic;
using Microsoft.Tools.WindowsInstallerXml;

namespace OSSBuild.WiX {
	public class WixPreprocessorExtension : PreprocessorExtension {
		#region Variables
		private static readonly string[] prefixes;
		private static readonly string[] nodeNames;
		private static readonly Dictionary<string, FunctionInfo> classes;
		private static readonly Dictionary<string, NodeInfo> documentClasses;
		#endregion

		#region Initialization
		static WixPreprocessorExtension() {
			//Debugger.Launch();
			Type IWiXExtensionType = typeof(IWiXExtension);
			Type IWiXDocumentExtensionType = typeof(IWiXDocumentExtension);

			classes = new Dictionary<string, FunctionInfo>(3);
			documentClasses = new Dictionary<string, NodeInfo>(3);
			List<string> names = new List<string>(1);
			List<string> documentNames = new List<string>(1);

			foreach (Type t in typeof(WixPreprocessorExtension).Assembly.GetTypes()) {
				#region Load document extensions
				if (IWiXDocumentExtensionType.IsAssignableFrom(t)) {
					//Create an instance of this object and hold onto it
					IWiXDocumentExtension instance = null;
					try { instance = (IWiXDocumentExtension)t.Assembly.CreateInstance(t.FullName, true); } catch { }
					if (instance != null) {
						string name = camelCaseName(t.Name);

						NodeInfo info = new NodeInfo() {
							Instance = instance,
							ExtensionType = t,
							Name = name
						};

						documentClasses.Add(t.Name.ToLower(), info);
						documentNames.Add(name);
					}
				}
				#endregion

				#region Load function extensions
				if (IWiXExtensionType.IsAssignableFrom(t)) {
					#region Look for public methods
					MethodInfo[] methods = t.GetMethods();
					if (methods == null || methods.Length <= 0)
						continue;
					#endregion

					List<MethodInfo> validMethods = new List<MethodInfo>(3);
					foreach (MethodInfo mi in methods) {
						#region Double check it's public and NOT static
						if (!mi.IsPublic || mi.IsStatic)
							continue;
						#endregion

						#region Check return type
						if (mi.ReturnType == null || !mi.ReturnType.Equals(typeof(string)))
							continue;
						#endregion

						#region Check parameters
						ParameterInfo[] parameters = mi.GetParameters();
						if (parameters == null || parameters.Length <= 0)
							continue;

						//Must have 1 param
						if (parameters.Length != 1)
							continue;

						if (/* must have (string[] args) param */!parameters[0].ParameterType.Equals(typeof(string[])))
							continue;
						#endregion

						validMethods.Add(mi);
					}

					//If we couldn't find any methods that fit the bill, then don't add this as a 
					//valid prefix and enable access to it.
					if (validMethods.Count <= 0)
						continue;

					//Create an instance of this object and hold onto it
					object instance = null;
					try { instance = t.Assembly.CreateInstance(t.FullName, true); } catch { }
					if (instance == null)
						continue;

					string name = camelCaseName(t.Name);

					FunctionInfo info = new FunctionInfo() {
						ExtensionType = t, 
						Functions = validMethods.ToArray(), 
						Instance = instance, 
						Name = name
					};

					classes.Add(t.Name.ToLower(), info);
					names.Add(name);
				}
				#endregion
			}

			prefixes = names.ToArray();
			nodeNames = documentNames.ToArray();
		}
		#endregion

		#region Properties
		public override string[] Prefixes {
			get { return prefixes; }
		}
		#endregion

		#region Helper Methods
		private static string camelCaseName(string name) {
			if (string.IsNullOrEmpty(name))
				return string.Empty;
			if (name.Length <= 1)
				return name.ToLower();

			char[] chars = name.ToCharArray();
			StringBuilder sb = new StringBuilder(chars.Length);

			bool stillUpper = true;
			for (int i = 0; i < chars.Length; ++i) {
				if (char.IsUpper(chars[i])) {
					if (stillUpper)
						sb.Append(char.ToLower(chars[i]));
					else
						sb.Append(chars[i]);
				} else {
					if (stillUpper)
						stillUpper = false;
					sb.Append(chars[i]);
				}
			}

			return sb.ToString();
		}
		#endregion

		#region Helper Classes
		private class FunctionInfo {
			private object[] invokeParams = new object[1];

			public Type ExtensionType {
				get;
				set;
			}

			public object Instance {
				get;
				set;
			}

			public string Name { 
				get; 
				set; 
			}

			public MethodInfo[] Functions {
				get;
				set;
			}

			public bool ContainsFunction(string prefix, string functionName) {
				if (string.IsNullOrEmpty(functionName))
					return false;

				if (!Name.Equals(prefix, StringComparison.CurrentCultureIgnoreCase))
					return false;

				foreach(MethodInfo mi in Functions)
					if (mi.Name.Equals(functionName, StringComparison.CurrentCultureIgnoreCase))
						return true;

				return false;
			}

			public string InvokeFunction(string functionName, string[] args) {
				try {
					foreach (MethodInfo mi in Functions) {
						if (!mi.Name.Equals(functionName, StringComparison.CurrentCultureIgnoreCase))
							continue;

						//We found the function - invoke it via reflection
						lock (Instance) {
							invokeParams[0] = args;
							return (string)mi.Invoke(Instance, invokeParams);
						}
					}
				} catch {
				}

				return null;
			}
		}

		private class NodeInfo {
			public Type ExtensionType {
				get;
				set;
			}

			public IWiXDocumentExtension Instance {
				get;
				set;
			}

			public string Name { 
				get; 
				set; 
			}

			public XmlNode InvokePreprocessDocument(XmlDocument document, XmlNode parentNode, XmlNode node, XmlAttributeCollection attributes) {
				try {
					//Invoke the preprocess document method
					lock (Instance) {
						return Instance.PreprocessDocument(document, parentNode, node, attributes);
					}
				} catch {
					throw;
				}
			}
		}
		#endregion

		public override string EvaluateFunction(string prefix, string function, string[] args) {
			#region Check params
			if (classes == null || classes.Count <= 0)
				return null;
			string prefixAsLowerCase = prefix.ToLower();
			if (!classes.ContainsKey(prefixAsLowerCase))
				return null;
			FunctionInfo info = classes[prefixAsLowerCase];
			if (info == null)
				return null;
			if (!info.ContainsFunction(prefix, function))
				return null;
			#endregion

			return info.InvokeFunction(function, args);
		}

		public override void PreprocessDocument(XmlDocument document) {
			#region Check params
			if (documentClasses == null || documentClasses.Count <= 0) {
				base.PreprocessDocument(document);
				return;
			}
			#endregion

			foreach (string nodeName in nodeNames) {
				#region Find node info
				if (nodeName == null || string.IsNullOrEmpty(nodeName))
					continue;
				string nodeNameAsLowerCase = nodeName.ToLower();
				if (!documentClasses.ContainsKey(nodeNameAsLowerCase))
					continue;
				NodeInfo info = documentClasses[nodeNameAsLowerCase];
				if (info == null)
					continue;
				#endregion

				//Don't use a foreach here b/c processing the document will most likely result in 
				//the nodes changing all around. Instead loop until we can't find any more.
				XmlNode node;
				XmlNodeList nodes;
				while ((nodes = document.GetElementsByTagName(nodeName)) != null && nodes.Count > 0) {
					if ((node = nodes[0]) == null)
						continue;

					//Have the extension process the document now.
					XmlNode replacementNode = info.InvokePreprocessDocument(document, node.ParentNode, node, node.Attributes);

					//Insert this node after our node so that it's in the document tree.
					//Then we take out the custom tag node.
					if (replacementNode != null)
						node.ParentNode.InsertAfter(replacementNode, node);

					//Remove this node from the document at the very least.
					//This also ensures us that this loop will eventually exit.
					node.ParentNode.RemoveChild(node);
				}
			}

			base.PreprocessDocument(document);
		}
	}
}
