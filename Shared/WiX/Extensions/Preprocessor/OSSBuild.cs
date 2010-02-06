using System;
using System.IO;
using System.Text;
using System.Security.Cryptography;

namespace OSSBuild.WiX {
	internal class OSSBuild : IWiXPreprocessorExtension {
		///<summary>
		///	Provides the typical project namespace URI.
		///</summary>
		public string NamespaceURI(string[] args) {
			return Namespace.OSSBuild;
		}

		///<summary>
		///	Removes whitespace from the beginning and end of the string.
		///</summary>
		public string Trim(string[] args) {
			return args[0].Trim();
		}

		///<summary>
		///	Generates a new GUID
		///</summary>
		public string CreateGUID(string[] args) {
			return Guid.NewGuid().ToString("B");
		}

		///<summary>
		///	Provides a reproducible GUID for a given string using an MD5 hash.
		///</summary>
		public string StringToGUID(string[] args) {
			// step 1, calculate MD5 hash from input
			using (MD5 md5 = MD5.Create()) {
				byte[] inputBytes = System.Text.Encoding.ASCII.GetBytes(args[0]);
				byte[] hash = md5.ComputeHash(inputBytes);

				return new Guid(hash).ToString("B");
			}
		}

		///<summary>
		///	Generates a valid WiX (Windows Installer) ID by only allowing 
		///	characters A-Z, a-z, digits (0-9), underscores (_), and periods (.).
		///	All other characters are replaced with an underscore.
		///</summary>
		public string CreateValidID(string[] args) {
			try {
				char[] name;
				
				using (MD5 md5 = MD5.Create()) {
					byte[] inputBytes = System.Text.Encoding.ASCII.GetBytes(args[0].Trim());
					byte[] hash = md5.ComputeHash(inputBytes);

					name = Convert.ToBase64String(hash).ToCharArray();
				}

				StringBuilder sb = new StringBuilder("_", name.Length);
				for (int i = 0; i < name.Length; ++i) {
					if (char.IsLetterOrDigit(name[i]) || name[i] == '_' || name[i] == '.')
						sb.Append(name[i]);
					else
						sb.Append('_');
				}

				return sb.ToString();
			} catch {
				return args[0];
			}
		}

		///<summary>
		///	Goes through all passed in arguments and returns the first one that is 
		///	either an existing file or directory.
		///</summary>
		public string ExistsCoalesce(string[] args) {
			foreach (string a in args) {
				if (File.Exists(a))
					return a;
				if (Directory.Exists(a))
					return a;
			}

			StringBuilder sb = new StringBuilder("Unable to locate a file/directory in the following locations: " + Environment.NewLine, 256);
			foreach (string a in args) {
				sb.Append("    ");
				sb.AppendLine(a);
			}

			throw new Exception(sb.ToString());
		}
	}
}
