using System;
using System.IO;
using System.Text;
using System.Reflection;
using System.Diagnostics;
using System.Collections.Generic;
using Microsoft.Tools.WindowsInstallerXml;

namespace OSSBuild.WiX {
	public class WixPreprocessorExtension : PreprocessorExtension {
		#region Variables
		private static readonly Dictionary<string, Info> classes;
		private static readonly string[] prefixes;
		#endregion

		#region Initialization
		static WixPreprocessorExtension() {
			//Debugger.Launch();
			Type IWiXExtensionType = typeof(IWiXExtension);
			List<string> names = new List<string>(1);
			classes = new Dictionary<string, Info>(3);

			foreach (Type t in typeof(WixPreprocessorExtension).Assembly.GetTypes()) {
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

					Info info = new Info() {
						ExtensionType = t, 
						Functions = validMethods.ToArray(), 
						Instance = instance, 
						Name = name
					};

					classes.Add(t.Name.ToLower(), info);
					names.Add(name);
				}
			}

			prefixes = names.ToArray();
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
		private class Info {
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
		#endregion

		public override string EvaluateFunction(string prefix, string function, string[] args) {
			#region Check params
			if (classes == null || classes.Count <= 0)
				return null;
			string prefixAsLowerCase = prefix.ToLower();
			if (!classes.ContainsKey(prefixAsLowerCase))
				return null;
			Info info = classes[prefixAsLowerCase];
			if (info == null)
				return null;
			if (!info.ContainsFunction(prefix, function))
				return null;
			#endregion

			return info.InvokeFunction(function, args);
		}
	}
}
