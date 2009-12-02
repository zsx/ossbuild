using System;
using System.IO;

namespace OSSBuild.WiX {
	internal class OSSBuild : IWiXExtension {
		public string ExistsCoalesce(string[] args) {
			foreach (string a in args) {
				if (File.Exists(a))
					return a;
				if (Directory.Exists(a))
					return a;
			}
			return null;
		}
	}
}
